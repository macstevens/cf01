/** cf.c

*/

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cf.h"

/* STATIC FUNCTIONS */

/* append formatted string   
prerequisite:  msg[0] = 0, or msg is null-terminated string
*/
static void cf00_msg_append_f(char *msg, const size_t max_msg_len,
    const char *fmtstr, ...)
{
    if (NULL != msg && NULL != fmtstr)
    {
        const size_t existing_msg_len = strlen(msg);
        if (max_msg_len > existing_msg_len + 2)
        {
            size_t remaining_len = (max_msg_len - existing_msg_len) - 1;
            va_list arg_ptr;
            char *m = msg + existing_msg_len;
            if (existing_msg_len > 0)
            {
                *m = '\n';
                ++m;
                --remaining_len;
            }
            va_start(arg_ptr, fmtstr);
            vsnprintf(m, remaining_len, fmtstr, arg_ptr);
            va_end(arg_ptr);
        }
    }
}

/* grow m_alloc_block_chain */
static void cf00_sa_allocate_block(cf00_string_allocator *a)
{
    assert(NULL != a);
    cf00_alloc_block_a *b = 
        (cf00_alloc_block_a *)malloc(sizeof(cf00_alloc_block_a));
    b->m_next_alloc_block = a->m_alloc_block_chain;
    a->m_alloc_block_chain = b;    
}

static char *cf00_allocate_char_buf(cf00_string_allocator *a, 
    const uint32 capacity)
{
    char *char_buf = NULL;
    if (NULL == a) {
        char_buf = (char *)malloc(capacity);
    }
    else {
        char **free_chain = NULL;
        switch (capacity) {
        case  16 : free_chain =  &(a->m_free_chain_char_buf_16); break;
        case  32 : free_chain =  &(a->m_free_chain_char_buf_32); break;
        case  64 : free_chain =  &(a->m_free_chain_char_buf_64); break;
        case 128 : free_chain = &(a->m_free_chain_char_buf_128); break;
        case 256 : free_chain = &(a->m_free_chain_char_buf_256); break;
        case 512 : free_chain = &(a->m_free_chain_char_buf_512); break;
        default : free_chain = NULL; break;
        }

        if (NULL == free_chain) {            
            char_buf = (char *)malloc(capacity);
        }
        else {
            if (NULL == *free_chain) {
                /* grow free chain */
                assert(capacity <= CF00_ALLOC_BLOCK_SIZE_A);
                cf00_sa_allocate_block(a);
                struct cf00_alloc_block_a *b = a->m_alloc_block_chain;
                char *m = (char *)(&((b->m_raw_memory)[0]));
                char *m_end = m + CF00_ALLOC_BLOCK_SIZE_A + 1 - capacity;
                while (m < m_end) {
                    *((char **)m) = *free_chain;
                    *free_chain = m;
                    m += capacity;
                }
            }
            /* remove from free chain */
            assert(NULL != *free_chain);
            char_buf = *free_chain;
            *free_chain = *(char**)(*free_chain);            
        }
    }
    return char_buf;
}

static void cf00_free_char_buf(cf00_string_allocator *a, char *buf, 
    const uint32 capacity)
{
    assert(NULL != buf);
    if (NULL == a) {
        free(buf);
    }
    else {
        char **free_chain = NULL;
        switch (capacity) {
        case  16 : free_chain =  &(a->m_free_chain_char_buf_16); break;
        case  32 : free_chain =  &(a->m_free_chain_char_buf_32); break;
        case  64 : free_chain =  &(a->m_free_chain_char_buf_64); break;
        case 128 : free_chain = &(a->m_free_chain_char_buf_128); break;
        case 256 : free_chain = &(a->m_free_chain_char_buf_256); break;
        case 512 : free_chain = &(a->m_free_chain_char_buf_512); break;
        default : free_chain = NULL; break;
        }
        if (NULL == free_chain) {            
            free(buf);
        }
        else {
            /* add buf to free chain */
            *((char **)buf) = *free_chain;
            *free_chain = buf;          
        }
    }
}

static cf00_string **cf00_allocate_string_ptr_array(cf00_string_allocator *a, 
    const uint32 capacity)
{
    const uint32 capacity_bytes = capacity * sizeof(cf00_string *);
    cf00_string **string_ptr_array = NULL;
    if (NULL == a) {
        string_ptr_array = (cf00_string **)malloc(capacity_bytes);
    }
    else {
        cf00_string ***free_chain = NULL;
        switch (capacity) {
        case  4 : free_chain =  &(a->m_free_chain_string_ptr_array_4); break;
        case  8 : free_chain =  &(a->m_free_chain_string_ptr_array_8); break;
        case 16 : free_chain = &(a->m_free_chain_string_ptr_array_16); break;
        case 32 : free_chain = &(a->m_free_chain_string_ptr_array_32); break;
        default : free_chain = NULL; break;
        }

        if (NULL == free_chain) {            
            string_ptr_array = (cf00_string **)malloc(capacity_bytes);
        }
        else {
            if (NULL == *free_chain) {
                /* grow free chain */
                assert(capacity_bytes <= CF00_ALLOC_BLOCK_SIZE_A);
                cf00_sa_allocate_block(a);
                struct cf00_alloc_block_a *b = a->m_alloc_block_chain;
                cf00_string **m = (cf00_string **)(&((b->m_raw_memory)[0]));
                cf00_string **m_end = (cf00_string **)(((char *)m) +
                    (CF00_ALLOC_BLOCK_SIZE_A + 1 - capacity_bytes));
                while (m < m_end) {
                    *((cf00_string ***)m) = *free_chain;
                    *free_chain = m;
                    m = (cf00_string **)(((char *)m) + capacity_bytes);
                }
            }
            /* remove from free chain */
            assert(NULL != *free_chain);
            string_ptr_array = *free_chain;
            *free_chain = *(cf00_string***)(*free_chain);
        }
    }
    return string_ptr_array;
}

/**
precondition: all strings in array have been freed or ownership
has been passed to a new array, etc.
*/
static void cf00_free_string_ptr_array(cf00_string_allocator *a,
    cf00_string **str_ptr_array, const uint32 capacity)
{
    assert(NULL != str_ptr_array);
    if (NULL == a) {
        free(str_ptr_array);
    }
    else {
        cf00_string ***free_chain = NULL;
        switch (capacity) {
        case  4 : free_chain =  &(a->m_free_chain_string_ptr_array_4); break;
        case  8 : free_chain =  &(a->m_free_chain_string_ptr_array_8); break;
        case 16 : free_chain = &(a->m_free_chain_string_ptr_array_16); break;
        case 32 : free_chain = &(a->m_free_chain_string_ptr_array_32); break;
        default : free_chain = NULL; break;
        }
        if (NULL == free_chain) {            
            free(str_ptr_array);
        }
        else {
            /* add str_ptr_array to free chain */
            *((cf00_string ***)str_ptr_array) = *free_chain;
            *free_chain = str_ptr_array;          
        }
    }
}





/* EXTERN FUNCTIONS */




/* cf00_string */

/* initialize raw memory */
extern void cf00_str_init(cf00_string *s)
{
    assert(NULL != s);
    memset (s, 0, sizeof(cf00_string));
}

/* 
clear string
also, prepare for freeing raw memory of struct cf00_string itself */
extern void cf00_str_clear(cf00_string *s)
{
    if (NULL != s)
    {
        if (NULL != s->m_char_buf) {
            cf00_free_char_buf(s->m_allocator, s->m_char_buf, s->m_capacity);
        }      
        s->m_char_buf = NULL;
        s->m_length = 0;
        s->m_capacity = 0;
    }
}

int cf00_str_compare(const cf00_string *x, const cf00_string *y)
{
    int result = 0;
    if (NULL == x) {
        if (NULL == y) {
            result = 0;
        }
        else {
            result = -1;
        }
    }
    else {
        if (NULL == y) {
            result = 1;
        }
        else {
            const uint32 min_len = (x->m_length < y->m_length) ? 
                x->m_length : y->m_length;
            result = strncmp(x->m_char_buf, y->m_char_buf, min_len);
            if (0 == result && x->m_length != y->m_length) {
                result = (x->m_length < y->m_length) ? -1 : 1;          
            } 
        }
    }
    return result;
}

void cf00_str_resize(cf00_string *s, const uint32 new_sz)
{
    if (NULL != s) {
        cf00_str_reserve(s, new_sz+1);
        s->m_length = new_sz;
    }
}

void cf00_str_reserve(cf00_string *s, const uint32 new_cap)
{
    if (NULL != s && s->m_capacity < new_cap)
    {
        // allocate new buffer
        uint32 new_capacity;
        if (new_cap <= 64) {
            if (new_cap <= 16) {
                new_capacity = 16;
            }
            else if (new_cap <= 32) {
                new_capacity = 32;
            }
            else {
                new_capacity = 64;
            }
        }
        else {
            if (new_cap <= 256) {
                if (new_cap <= 128) {
                    new_capacity = 128;
                }
                else{
                    new_capacity = 256;
                }
            }
            else{
                if (new_cap <= 512) {
                    new_capacity = 512;
                }
                else{
                    new_capacity = new_cap;
                }
            }
        }
        assert(new_capacity >= new_cap);
        char *new_buf = cf00_allocate_char_buf(s->m_allocator, new_capacity);

        /* copy buffer contents */
        if (NULL != s->m_char_buf) {        
            memcpy(new_buf, s->m_char_buf, s->m_length);
        }
        if (new_capacity > s->m_length) {
            /* add terminating null character, if there is room */
            new_buf[s->m_length] = 0;
        }

        /* free old buffer and update string data */
        if (NULL != s->m_char_buf) {
            cf00_free_char_buf(s->m_allocator, s->m_char_buf, s->m_capacity);
        }
        s->m_char_buf = new_buf;
        s->m_capacity = new_capacity;
    }
}

void cf00_str_assign(cf00_string *dest, const cf00_string *src)
{
    if (NULL != dest && NULL != src && dest != src) {
        cf00_str_reserve(dest, src->m_length + 1);
        memcpy(dest->m_char_buf, src->m_char_buf, src->m_length);
        (dest->m_char_buf)[src->m_length] = 0;
        dest->m_length = src->m_length;
    }
}

void cf00_str_assign_from_char_ptr(cf00_string *dest, const char *src)
{
    if (NULL != dest) {
        if (NULL == src) {
            cf00_str_clear(dest);
        }
        else if (dest->m_char_buf != src) {
            const uint32 len = strlen(src);
            cf00_str_reserve(dest, len + 1);
            memcpy(dest->m_char_buf, src, len);
            (dest->m_char_buf)[len] = 0;
            dest->m_length = len;
        }
    }
}

void cf00_str_append(cf00_string *s, const cf00_string *addition)
{
    if (NULL != s && NULL != addition) {
        const uint32 add_len = addition->m_length;
        const uint32 len = s->m_length + add_len;
        cf00_str_reserve(s, len + 1);
        memcpy((s->m_char_buf)+(s->m_length), addition->m_char_buf, add_len);
        (s->m_char_buf)[len] = 0;
        s->m_length = len;
    }
}

void cf00_str_append_char_buf(cf00_string *s, const char *addition)
{
    if (NULL != s && NULL != addition) {
        const uint32 add_len = strlen(addition);
        const uint32 len = s->m_length + add_len;
        cf00_str_reserve(s, len + 1);
        memcpy((s->m_char_buf)+(s->m_length), addition, add_len);
        (s->m_char_buf)[len] = 0;
        s->m_length = len;
    }
}

uint64 cf00_str_verify_data(const cf00_string *s, char *err_msg,
    const size_t max_err_msg_len)
{
    uint64 error_count = 0;

    if (NULL != s) {
        if (NULL != s->m_allocator) {
            /* verify s was allocated by m_allocator */
        }

        if (NULL == s->m_char_buf)
        {
            if (s->m_length > 0) {
                ++error_count;
                cf00_msg_append_f(err_msg, max_err_msg_len, 
                    "m_char_buf NULL, but m_length = %i", (int)(s->m_length));
            }
            if (s->m_capacity > 0) {
                ++error_count;
                cf00_msg_append_f(err_msg, max_err_msg_len, 
                   "m_char_buf NULL, but m_capacity = %i",(int)(s->m_capacity));
            }
        }
        else
        {
            if (s->m_length < s->m_capacity && 
                0x0 != (s->m_char_buf)[s->m_length]) {
                ++error_count;
                cf00_msg_append_f(err_msg, max_err_msg_len, 
                   "string not null-terminated");
            }
        }

        if (s->m_length > s->m_capacity) {
            ++error_count;
            cf00_msg_append_f(err_msg, max_err_msg_len, 
               "string m_length(%i) > m_capacity(%i)",(int)(s->m_length),
               (int)(s->m_capacity));
        }

        if (0 == s->m_capacity && NULL != s->m_char_buf) {
            ++error_count;
            cf00_msg_append_f(err_msg, max_err_msg_len, 
                "m_capacity = %i, but m_char_buf non-NULL",
                (int)(s->m_capacity));
        }
    }

    return error_count;
}



/* cf00_str_vec */

/* initialize raw memory */
void cf00_str_vec_init(cf00_str_vec *sv)
{
    assert(NULL != sv);
    memset(sv, 0, sizeof(cf00_str_vec));
}

/* 
clear string vector
also, prepare for freeing raw memory of struct cf00_str_vec itself */
void cf00_str_vec_clear(cf00_str_vec *sv)
{
    if (NULL != sv)
    {
        if (NULL != sv->m_str_array) {
            /* free individual strings */
            cf00_string **s = sv->m_str_array;
            cf00_string ** const s_end = s + (sv->m_size);
            for (; s_end != s; ++s)
            {
                cf00_free_string(*s);
            }

            /* free array */
            cf00_free_string_ptr_array(sv->m_allocator, sv->m_str_array,
                sv->m_capacity);
        }
        sv->m_str_array = NULL;
        sv->m_size = 0;
        sv->m_capacity = 0;
    }
}

int cf00_str_vec_compare(const cf00_str_vec *x, const cf00_str_vec *y)
{
    int result = 0;
    if (NULL == x) {
        if (NULL == y) {
            result = 0;
        }
        else {
            result = -1;
        }
    }
    else {
        if (NULL == y) {
            result = 1;
        }
        else {             
            const uint32 min_size = (x->m_size < y->m_size) ? 
                x->m_size : y->m_size;
            cf00_string **xs = x->m_str_array;
            cf00_string **xs_end = xs + min_size;
            cf00_string **ys = y->m_str_array;
            while (0 == result && xs < xs_end) {
                result = cf00_str_compare(*xs, *ys);
                ++xs;
                ++ys;
            }
            if (0 == result && x->m_size != y->m_size) {
                result = (x->m_size < y->m_size) ? -1 : 1;          
            } 
        }
    }
    return result;
}

void cf00_str_vec_resize(cf00_str_vec *sv, const uint32 new_sz)
{
    if (NULL != sv) {
        if (new_sz < sv->m_size)
        {
            /* free extra strings */
            cf00_string **s = sv->m_str_array + new_sz;
            cf00_string ** const s_end = sv->m_str_array + sv->m_size;
            for (; s_end != s; ++s)
            {
                cf00_free_string(*s);
                *s = NULL;
            }
        }
        cf00_str_vec_reserve(sv, new_sz+1);
        sv->m_size = new_sz;
    }
}

void cf00_str_vec_reserve(cf00_str_vec *sv, const uint32 new_cap)
{
    if (NULL != sv && sv->m_capacity <= new_cap)
    {
        // allocate new buffer
        uint32 new_capacity;
        if (new_cap <= 8) {
            if (new_cap <= 4) {
                new_capacity = 4;
            }
            else {
                new_capacity = 8;
            }
        }
        else {
            if (new_cap <= 16) {
                new_capacity = 16;
            }
            else if (new_cap <= 32) {
                new_capacity = 32;
            }
            else{
                new_capacity = new_cap;
            }
        }
        assert(new_capacity >= new_cap);
        cf00_string **new_str_array = cf00_allocate_string_ptr_array(
            sv->m_allocator, new_capacity);

        /* copy str_array contents */
        if (NULL != sv->m_str_array && sv->m_size > 0) {
            memcpy(new_str_array, sv->m_str_array, 
                (sv->m_size) * sizeof(cf00_string *));
        }

        /* free old buffer and update string vector data */
        if (NULL != sv->m_str_array) {
            cf00_free_string_ptr_array(sv->m_allocator, sv->m_str_array,
                sv->m_capacity);
        }
        sv->m_str_array = new_str_array;
        sv->m_capacity = new_capacity;
    }
}

/* adds string pointer s to vector, does not copy */
void cf00_str_vec_push_back(cf00_str_vec *sv, cf00_string *s)
{
    assert(NULL != sv);
    assert(NULL != s);
    assert(sv->m_allocator == s->m_allocator);
    cf00_str_vec_reserve(sv, sv->m_size + 1);
    (sv->m_str_array)[sv->m_size] = s;
    ++(sv->m_size);    
}

/* copies string contents */
void cf00_str_vec_push_back_copy(cf00_str_vec *sv, const cf00_string *s)
{
    assert(NULL != sv);
    assert(NULL != s);
    cf00_string *s_copy = cf00_allocate_string(sv->m_allocator);
    cf00_str_assign(s_copy, s);
    cf00_str_vec_push_back(sv, s_copy);
}

void cf00_str_vec_push_back_format(cf00_str_vec *sv, const char *fmtstr, ...)
{
    /* not implemented */
}

uint64 cf00_str_vec_verify_data(const cf00_str_vec *sv, char *err_msg,
    const size_t max_err_msg_len)
{
    uint64 error_count = 0;

    if (NULL != sv)
    {
        size_t i;

        /* for each string, call cf00_str_verify_data(); */
        for (i = 0; i < sv->m_capacity; ++i) {
            cf00_string *s = (sv->m_str_array)[i];

            if (i < sv->m_size) {
                if (NULL != s) {
                    error_count += cf00_str_verify_data(s, err_msg,
                        max_err_msg_len);            
                    if (s->m_allocator != sv->m_allocator) {
                        cf00_msg_append_f(err_msg, max_err_msg_len, 
                            "m_allocator mismatch");
                        ++error_count;
                    }
                }
            }
            else {
                if (NULL != s) {
                    cf00_msg_append_f(err_msg, max_err_msg_len, 
                        "non-NULL string past array end i=%i", (int)i);
                }
            }
        }

        /*    compare capacity, size */

        if (sv->m_size > sv->m_capacity) {
            ++error_count;
            cf00_msg_append_f(err_msg, max_err_msg_len, 
               "str_vec m_size(%i) > m_capacity(%i)",(int)(sv->m_size),
               (int)(sv->m_capacity));
        }

        if (0 == sv->m_capacity && NULL != sv->m_str_array) {
            ++error_count;
            cf00_msg_append_f(err_msg, max_err_msg_len, 
                "m_capacity = %i, but m_str_array non-NULL",
                (int)(sv->m_str_array));
        }

    }

    return error_count;
}



/* cf00_string_allocator */

extern void cf00_str_alloc_init(cf00_string_allocator *a)
{
    memset(a, 0, sizeof(cf00_string_allocator));
}

/* prepare for freeing raw memory of struct cf00_string_allocator itself

precondition: all strings and str_vecs cleared
*/
extern void cf00_str_alloc_clear(cf00_string_allocator *a)
{
    assert(NULL != a);

    /* free all blocks */
    cf00_alloc_block_a *b = a->m_alloc_block_chain;
    while (NULL != b) {
        cf00_alloc_block_a *del_b = b;
        b = b->m_next_alloc_block;
        free(del_b);
    }

    /* reset all pointers */
    memset(a, 0, sizeof(cf00_string_allocator));
}

extern cf00_string *cf00_allocate_string(cf00_string_allocator *a)
{
    cf00_string *str = NULL;

    if (NULL == a) {
        str = (cf00_string *)malloc(sizeof(cf00_string));
        cf00_str_init(str);
    }
    else {
        if (NULL == a->m_free_chain_string) {
            /* grow free chain */
            assert(sizeof(cf00_string) <= CF00_ALLOC_BLOCK_SIZE_A);
            cf00_sa_allocate_block(a);
            struct cf00_alloc_block_a *b = a->m_alloc_block_chain;
            cf00_string *m = (cf00_string *)(&((b->m_raw_memory)[0]));
            cf00_string *m_end = (cf00_string *)(((char *)m) + 
                (CF00_ALLOC_BLOCK_SIZE_A + 1 - sizeof(cf00_string)));
            while (m < m_end) {
                cf00_string *temp_ptr;
                cf00_str_init(m);
                m->m_allocator = a;
                m->m_char_buf = (char *)(a->m_free_chain_string);
                a->m_free_chain_string = m;
                m = (cf00_string *)(((char *)m) + sizeof(cf00_string));
            }
        }

        /* remove from free chain */
        assert(NULL != a->m_free_chain_string);        
        str = a->m_free_chain_string;
        a->m_free_chain_string = (cf00_string *)(str->m_char_buf);
        str->m_char_buf = NULL;
    }

    return str;
}

extern cf00_str_vec *cf00_allocate_str_vec(cf00_string_allocator *a)
{
    cf00_str_vec *sv = NULL;

    if (NULL == a) {
        sv = (cf00_str_vec *)malloc(sizeof(cf00_str_vec));
        cf00_str_vec_init(sv);
    }
    else {
        if (NULL == a->m_free_chain_str_vec) {
            /* grow free chain */
            assert(sizeof(cf00_string) <= CF00_ALLOC_BLOCK_SIZE_A);
            cf00_sa_allocate_block(a);
            struct cf00_alloc_block_a *b = a->m_alloc_block_chain;
            cf00_str_vec *m = (cf00_str_vec *)(&((b->m_raw_memory)[0]));
            cf00_str_vec *m_end = (cf00_str_vec *)(((char *)m) + 
                (CF00_ALLOC_BLOCK_SIZE_A + 1 - sizeof(cf00_str_vec)));
            while (m < m_end) {
                cf00_str_vec *temp_ptr;
                cf00_str_vec_init(m);
                m->m_allocator = a;
                m->m_str_array = (cf00_string **)(a->m_free_chain_str_vec);
                a->m_free_chain_str_vec = m;
                m = (cf00_str_vec *)(((char *)m) + sizeof(cf00_str_vec));
            }
        }

        /* remove from free chain */
        assert(NULL != a->m_free_chain_str_vec);        
        sv = a->m_free_chain_str_vec;
        a->m_free_chain_str_vec = (cf00_str_vec *)(sv->m_str_array);
        sv->m_str_array = NULL;
    }

    return sv;
}

extern void cf00_free_string(cf00_string *s)
{
    if (NULL != s) {
        cf00_str_clear(s);
        cf00_string_allocator *a = s->m_allocator;
        if (NULL != a) {
            /* put on free chain */
            s->m_char_buf = (char *)(a->m_free_chain_string);
            a->m_free_chain_string = s;
        }
        else {
            free(s);
        }
    }
}

extern void cf00_free_str_vec(cf00_str_vec *sv)
{
    if (NULL != sv) {
        cf00_str_vec_clear(sv);
        cf00_string_allocator *a = sv->m_allocator;
        if (NULL != a) {
            /* put on free chain */
            sv->m_str_array = (cf00_string **)(a->m_free_chain_str_vec);
            a->m_free_chain_str_vec = sv;
        }
        else {
            free(sv);
        }
    }
}

/* dump debug information to stdout */
void cf00_str_alloc_debug_dump(cf00_string_allocator *a)
{
    typedef struct {char *m_c; int m_i;} char_ptr_int;
    const char_ptr_int char_buf_free_chn_sz[] = {
        {  a->m_free_chain_char_buf_16,  16 },
        {  a->m_free_chain_char_buf_32,  32 },
        {  a->m_free_chain_char_buf_64,  64 },
        { a->m_free_chain_char_buf_128, 128 },
        { a->m_free_chain_char_buf_256, 256 },
        { a->m_free_chain_char_buf_512, 512 }
    };
    const size_t char_buf_free_chn_count =
        sizeof(char_buf_free_chn_sz)/sizeof(char_buf_free_chn_sz[0]);

    typedef struct {cf00_string **m_s; int m_i;} str_ptr_int;
    const str_ptr_int str_array_free_chn_sz[] = {
        {  a->m_free_chain_string_ptr_array_4,  4 },
        {  a->m_free_chain_string_ptr_array_8,  8 },
        { a->m_free_chain_string_ptr_array_16, 16 },
        { a->m_free_chain_string_ptr_array_32, 32 }
    };
    const size_t str_array_free_chn_count =
        sizeof(str_array_free_chn_sz)/sizeof(str_array_free_chn_sz[0]);

    if (NULL == a) {
        printf("NULL STRING ALLOCATOR\n");
    }
    else {
        size_t i;
        printf("STRING ALLOCATOR\n");
        int count = 0;
        cf00_alloc_block_a *b = a->m_alloc_block_chain;
        while (NULL != b) {
            b = b->m_next_alloc_block;
            ++count;
        }
        printf("  BLOCK COUNT:%i\n", count);

        for (i = 0; i < char_buf_free_chn_count; ++i) {
            const char_ptr_int data = char_buf_free_chn_sz[i];
            char *free_chain = data.m_c;
            int char_buf_sz = data.m_i;
            count = 0;
            while (NULL != free_chain) {
                free_chain = *((char**)free_chain);
                ++count;
            }
            printf("  CHAR BUF[%i] FREE CHAIN SIZE:%i\n", char_buf_sz, count);
        }

        count = 0;
        cf00_string *sfc = a->m_free_chain_string;
        while (NULL != sfc) {
            sfc = (cf00_string *)(sfc->m_char_buf);
            ++count;
        }
        printf("  STRING FREE CHAIN SIZE:%i\n", count);

        for (i = 0; i < str_array_free_chn_count; ++i) {
            const str_ptr_int data = str_array_free_chn_sz[i];
            cf00_string **free_chain = data.m_s;
            int str_array_sz = data.m_i;
            count = 0;
            while (NULL != free_chain) {
                free_chain = *((cf00_string***)free_chain);
                ++count;
            }
            printf("  STR ARRAY[%i] FREE CHAIN SIZE:%i\n", str_array_sz, count);
        }

        count = 0;
        cf00_str_vec *svfc = a->m_free_chain_str_vec;
        while (NULL != svfc) {
            svfc = (cf00_str_vec *)(svfc->m_str_array);
            ++count;
        }
        printf("  STR_VEC FREE CHAIN SIZE:%i\n", count);
    }
}


uint64 cf00_str_alloc_verify_data(const cf00_string_allocator *a,
    char *err_msg, const size_t max_err_msg_len)
{
    uint64 error_count = 0;

    if (NULL != a)
    {
        static const size_t max_block_count = 0x1000000;
        size_t i;
        size_t count = 0;
        cf00_alloc_block_a *b = a->m_alloc_block_chain;
        while (NULL != b && count <= max_block_count) {
            /* verify block */
            b = b->m_next_alloc_block;
            ++count;
        }

#if 0
        for (i = 0; i < char_buf_free_chn_count; ++i) {
            const char_ptr_int data = char_buf_free_chn_sz[i];
            char *free_chain = data.m_c;
            int char_buf_sz = data.m_i;
            count = 0;
            while (NULL != free_chain) {
                free_chain = *((char**)free_chain);
                ++count;
            }
            printf("  CHAR BUF[%i] FREE CHAIN SIZE:%i\n", char_buf_sz, count);
        }

        count = 0;
        cf00_string *sfc = a->m_free_chain_string;
        while (NULL != sfc) {
            sfc = (cf00_string *)(sfc->m_char_buf);
            ++count;
        }
        printf("  STRING FREE CHAIN SIZE:%i\n", count);

        for (i = 0; i < str_array_free_chn_count; ++i) {
            const str_ptr_int data = str_array_free_chn_sz[i];
            cf00_string **free_chain = data.m_s;
            int str_array_sz = data.m_i;
            count = 0;
            while (NULL != free_chain) {
                free_chain = *((cf00_string***)free_chain);
                ++count;
            }
            printf("  STR ARRAY[%i] FREE CHAIN SIZE:%i\n", str_array_sz, count);
        }

        count = 0;
        cf00_str_vec *svfc = a->m_free_chain_str_vec;
        while (NULL != svfc) {
            svfc = (cf00_str_vec *)(svfc->m_str_array);
            ++count;
        }
#endif

    }

    return error_count;
}










/* cf00_xyz & cf00_xyzabc & cf00_xyzabc_allocator: template code */



/* initialize raw memory */
extern void cf00_xyzabc_init(cf00_xyzabc *x)
{
    assert(NULL != x);
    memset (x, 0, sizeof(cf00_xyzabc));

    /* initialize cf00_managed_object_data */
    (x->m_object_data).m_object_type = CF00_OT_XYZABC;
    (x->m_object_data).m_rev_ptr_ctr_type = CF00_RPCT_ARRAY_3;
    assert(NULL == (x->m_object_data).m_allocator);
    assert(NULL == (x->m_object_data).m_rev_ptr_array_3[0]);
    assert(NULL == (x->m_object_data).m_rev_ptr_array_3[1]);
    assert(NULL == (x->m_object_data).m_rev_ptr_array_3[2]);
}

/* 
clear xyzabc
also, prepare for freeing raw memory of struct cf00_xyzabc itself */
extern void cf00_xyzabc_clear(cf00_xyzabc *x)
{
    if (NULL != x)
    {
        /* TODO: iterate reverse pointers in m_object_data.m_rev_ptr_(container)
           remove pointer to this object.  clear rev_ptr container */

        if (NULL != x->m_xyz_array) {
        //    cf00_free_xyz_buf((cf00_xyzabc_allocator *)
        //        ((x->m_object_data).m_allocator), x->m_xyz_array,
        //        x->m_xyz_array_capacity);
        }      
        x->m_xyz_array = NULL;
        x->m_xyz_array_length = 0;
        x->m_xyz_array_capacity = 0;
    }
}

#if 0
int cf00_xyzabc_compare(const cf00_xyzabc *x, const cf00_xyzabc *y);
void cf00_xyzabc_resize_xyz_array(cf00_xyzabc *x, const uint32 new_sz);
void cf00_xyzabc_reserve_xyz_array(cf00_xyzabc *x, const uint32 new_cap);
void cf00_xyzabc_assign(cf00_xyzabc *dest, const cf00_xyzabc *src);
uint64 cf00_xyzabc_verify_data(const cf00_xyzabc *x, cf00_string *err_msg);


static void cf00_sa_allocate_block(cf00_string_allocator *a);
static xyz *cf00_allocate_xyz_buf(cf00_string_allocator *a, 
    const uint32 capacity);
static void cf00_free_xyz_buf(cf00_string_allocator *a, xyz *buf, 
    const uint32 capacity);


void cf00_xyzabc_alloc_init(cf00_xyzabc_allocator *a);
void cf00_xyzabc_alloc_clear(cf00_xyzabc_allocator *a);
cf00_xyzabc *cf00_allocate_xyzabc(cf00_xyzabc_allocator *a);
void cf00_free_xyzabc(struct cf00_xyzabc *s);

void cf00_xyzabc_alloc_debug_dump(cf00_xyzabc_allocator *a);
uint64 cf00_xyzabc_alloc_verify_data(const cf00_xyzabc_allocator *a,
    cf00_string *err_msg);

#endif
