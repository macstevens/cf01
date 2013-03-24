/** cf.c

*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cf.h"

/* STATIC FUNCTIONS */

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
                char *m_end = m + CF00_ALLOC_BLOCK_SIZE_A;
                while (m < m_end) {
                    *((char **)m) = *free_chain;
                    /*memcpy(m, *free_chain, sizeof(*free_chain));*/
                    *free_chain = m;
                    m += capacity;
                }
            }
            /* remove from free chain */
            assert(NULL != *free_chain);
            char_buf = *free_chain;
            *free_chain = *(char**)(*free_chain);
            /*memcpy(*free_chain, (*(char**)(*free_chain)), sizeof(*free_chain));*/
        }
    }
    return char_buf;
}

static void cf00_free_char_buf(cf00_string_allocator *a, char *buf, 
    const uint32 capacity)
{
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
            memcpy(buf, *free_chain, sizeof(*free_chain));
            *free_chain = buf;          
        }
    }
}





/* EXTERN FUNCTIONS */




/* cf_00_string */

/* initialize raw memory */
extern void cf_00_str_init(cf00_string *s)
{
    assert(NULL != s);
    memset (s, 0, sizeof(cf00_string));
}

/* 
clear string
also, prepare for freeing raw memory of struct cf00_string_allocator itself */
extern void cf_00_str_clear(cf00_string *s)
{
    if (NULL != s)
    {
        cf00_free_char_buf(s->m_allocator, s->m_char_buf, s->m_capacity);
        s->m_char_buf = 0;
        s->m_length = 0;
        s->m_capacity = 0;
    }
}


void cf_00_str_resize(cf00_string *s, const uint32 new_sz)
{
    if (NULL != s) {
        cf_00_str_reserve(s, new_sz+1);
        s->m_length = new_sz;
    }
}

void cf_00_str_reserve(cf00_string *s, const uint32 new_cap)
{
    if (NULL != s && s->m_capacity <= new_cap)
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
        memcpy(new_buf, s->m_char_buf, s->m_length);
        if (new_capacity > s->m_length) {
            /* add terminating null character, if there is room */
            new_buf[s->m_length] = 0;
        }
        
        /* free old buffer and update string data */
        cf00_free_char_buf(s->m_allocator, s->m_char_buf, s->m_capacity);
        s->m_char_buf = new_buf;
        s->m_capacity = new_capacity;
    }
}

void cf_00_str_assign(cf00_string *dest, const cf00_string *src)
{

/* not implemented */

}

void cf_00_str_assign_from_char_ptr(cf00_string *dest, const char *src)
{

/* not implemented */

}

void cf_00_str_append(cf00_string *s, const cf00_string *addition)
{

/* not implemented */

}

void cf_00_str_append_char_buf(cf00_string *s, const char *addition)
{

/* not implemented */

}




/* cf00_string_allocator */

extern void cf00_init_string_allocator(cf00_string_allocator *a)
{
    memset(a, 0, sizeof(cf00_string_allocator));
}

/* prepare for freeing raw memory of struct cf00_string_allocator itself */
extern void cf00_clear_string_allocator(cf00_string_allocator *a)
{
    assert(NULL != a);

    /* normally, would need to clear all individual strings, but strings
    have no internal allocated memory other than that which is in the blocks */


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
        cf_00_str_init(str);
    }
    else {
        if (NULL == a->m_free_chain_string) {
            /* grow free chain */
            assert(sizeof(cf00_string) <= CF00_ALLOC_BLOCK_SIZE_A);
            cf00_sa_allocate_block(a);
            struct cf00_alloc_block_a *b = a->m_alloc_block_chain;
            cf00_string *m = (cf00_string *)(&((b->m_raw_memory)[0]));
            cf00_string *m_end = m + CF00_ALLOC_BLOCK_SIZE_A;
            while (m < m_end) {
                cf00_string *temp_ptr;
                cf_00_str_init(m);
                m->m_allocator = a;
                m->m_char_buf = (char *)(a->m_free_chain_string);
                a->m_free_chain_string = m;
                m += sizeof(cf00_string);
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

/* not implemented */

    return sv;
}

extern void cf00_free_string(cf00_string *s)
{
    if (NULL != s) {
        cf_00_str_clear(s);
        cf00_string_allocator *a = s->m_allocator;
        if (NULL != a) {
            /* put on free chain */

/* not implemented */

        }
        else {
            free(s);
        }
    }
}

extern void cf00_free_str_vec(cf00_str_vec *sv)
{

}

/* dump debug information to stdout */
void cf00_str_alloc_debug_dump(cf00_string_allocator *a)
{
    if (NULL == a) {
        printf("NULL STRING ALLOCATOR\n");
    }
    else {
        int block_count = 0;
        cf00_alloc_block_a *b = a->m_alloc_block_chain;
        while (NULL != b) {
            b = b->m_next_alloc_block;
            ++block_count;
        }
        printf("  BLOCK COUNT:%i\n", block_count);
    }
}

