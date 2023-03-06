/** cf01.cpp


Copyright (c) 2021 Mac Stevens <stevensm@earthlink.net> <www.macstevens.net>

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

Reference: https://opensource.org/licenses/ISC
*/
#include <fstream> 
#include <limits> 
#include <iostream> 
#include <vector> 
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cf01.h"


/* parse "3,66,92,4" => vector */
void parse_uint64_csv(const std::string& s, std::vector<cf01_uint64> *v){
assert(NULL != v);
size_t i=0, j=0, k=0;
std::string uint_str;
while((i != std::string::npos) && (i < s.length())) {
    j=s.find(",",i);
    uint_str = s.substr(i,j);
    cf01_uint64 ui = strtoull(uint_str.c_str(), NULL, 10);
    v->push_back(ui);
    i = (std::string::npos == j) ? std::string::npos : (j + 1);
    }
}


cf01_aasrt_result::cf01_aasrt_result():
    m_done(false),
    m_pass(false),
    m_file_name(),
    m_func_name(),
    m_line_num(0),
    m_call_depth(0),
    m_err_msg()
{
memset(m_aasrt_p_count, 0, sizeof(m_aasrt_p_count));
memset(m_curr_t_count, 0, sizeof(m_curr_t_count));

#ifndef NDEBUG
assert(!m_done);
assert(!m_pass);
assert(m_file_name.empty());
assert(m_func_name.empty());
assert(0==m_line_num);
assert(0==m_call_depth);
for(int i=0; i<CF01_AASRT_CALL_DEPTH_COUNT; ++i)
    {
    assert(0==m_aasrt_p_count[i]);
    assert(0==m_curr_t_count[i]);
    }
assert(m_err_msg.empty());
#endif
};


cf01_aasrt_result::~cf01_aasrt_result() {}




cf01_auto_assert_wksp cf01_auto_assert_wksp::m_instance;


cf01_auto_assert_wksp::cf01_auto_assert_wksp():
    m_call_depth(0),
    m_aasrt_result_last_passed(),
    m_aasrt_result_first_failed()
{
memset(m_curr_p_count, 0, sizeof(m_curr_p_count));
memset(m_curr_t_count, 0, sizeof(m_curr_t_count));
memset(m_depth_call_idx_range_crit, 0, sizeof(m_depth_call_idx_range_crit));
memset(m_idx_crit_idx, 0, sizeof(m_idx_crit_idx));
memset(m_xdbg_ctrl_t_count, 0, sizeof(m_xdbg_ctrl_t_count));
memset(m_xdbg_ctrl_dbg_lvl, 0, sizeof(m_xdbg_ctrl_dbg_lvl));
clear_utility_err_buf();
init_from_journal_file();
}

cf01_auto_assert_wksp::~cf01_auto_assert_wksp() {}


void cf01_auto_assert_wksp::init_default(){
cf01_call_idx_range_crit idx_range_crit;
memset(m_depth_call_idx_range_crit, 0, sizeof(m_depth_call_idx_range_crit));

idx_range_crit.m_call_idx_range_end = 32;
idx_range_crit.m_call_idx_div = 4; 
idx_range_crit.m_call_idx_mod = 0;
add_aasrt_call_idx_range_crit(0, &idx_range_crit, NULL, NULL);

idx_range_crit.m_call_idx_range_end = 256;
idx_range_crit.m_call_idx_div = 16; 
idx_range_crit.m_call_idx_mod = 0;
add_aasrt_call_idx_range_crit(0, &idx_range_crit, NULL, NULL);

idx_range_crit.m_call_idx_range_end = 4096;
idx_range_crit.m_call_idx_div = 32; 
idx_range_crit.m_call_idx_mod = 0;
add_aasrt_call_idx_range_crit(0, &idx_range_crit, NULL, NULL);

idx_range_crit.m_call_idx_range_end = 0;
idx_range_crit.m_call_idx_div = 64; 
idx_range_crit.m_call_idx_mod = 0;
add_aasrt_call_idx_range_crit(0, &idx_range_crit, NULL, NULL);

complete_table();
assert(0 == verify_data(NULL));
}

void cf01_auto_assert_wksp::init_from_journal_file(){
static const char *jrnl_file_name = "cf01_jrnl.txt";
enum parse_mode_type{
    PARSE_MODE_SEEKING_TOKEN_START,
    PARSE_MODE_READING_TAG,
    PARSE_MODE_READING_START_TAG,
    PARSE_MODE_READING_END_TAG,
    PARSE_MODE_READING_CONTENT_TOKEN,
    PARSE_MODE_READING_ESCAPE_CHAR,
    PARSE_MODE_DONE
};
parse_mode_type parse_mode = PARSE_MODE_SEEKING_TOKEN_START;
std::vector<std::string> tokens;
std::string token;
std::ifstream is;
is.open(jrnl_file_name);
while (is.good()) {
    bool token_done = false;
    char ch;
    is.get(ch);
    switch(parse_mode){
    case PARSE_MODE_SEEKING_TOKEN_START:
        if(!isspace(ch)){
            token.append(1, ch);
            parse_mode = ('<' == ch) ? PARSE_MODE_READING_TAG:
                PARSE_MODE_READING_CONTENT_TOKEN;
            }
        break;
    case PARSE_MODE_READING_TAG:
        if('<' == ch){ /* re-start reading tag */
            tokens.push_back(token);
            token.erase();
            token.append(1, ch);
            }
        else if(!isspace(ch)){
            token.append(1, ch);
            parse_mode = ('/' == ch) ? PARSE_MODE_READING_END_TAG:
                PARSE_MODE_READING_START_TAG;
            }
        break;
    case PARSE_MODE_READING_START_TAG:
        if('<' == ch){ /* re-start reading tag */
            tokens.push_back(token);
            token.erase();
            token.append(1, ch);
            parse_mode=PARSE_MODE_READING_TAG;
            }
        else{
            token.append(1, ch);
            if('>' == ch){
                tokens.push_back(token);
                token.erase();
                parse_mode=PARSE_MODE_SEEKING_TOKEN_START;
                }
            }
        break;
    case PARSE_MODE_READING_END_TAG:
        if('<' == ch){ /* re-start reading tag */
            tokens.push_back(token);
            token.erase();
            token.append(1, ch);
            parse_mode=PARSE_MODE_READING_TAG;
            }
        else{
            token.append(1, ch);
            if('>' == ch){
                tokens.push_back(token);
                token.erase();
                parse_mode=PARSE_MODE_SEEKING_TOKEN_START;
                }
            }
        break;
    case PARSE_MODE_READING_CONTENT_TOKEN:
        if('\\' == ch){
            parse_mode = PARSE_MODE_READING_ESCAPE_CHAR;
            }
        else if('<' == ch){
            tokens.push_back(token);
            token.erase();
            token.append(1, ch);
            parse_mode=PARSE_MODE_READING_TAG;
            }
        else{
            token.append(1, ch);
            }
        break;
    case PARSE_MODE_READING_ESCAPE_CHAR:
        token.append(1, ch);
        parse_mode = PARSE_MODE_READING_CONTENT_TOKEN;
        break;
    case PARSE_MODE_DONE:
        break;
        }
    }
is.close();

enum tag_type{
    TT_AASRT_RESULT,
    TT_DONE,
    TT_PASS,
    TT_FILE_NAME,
    TT_FUNC_NAME,
    TT_LINE_NUM,
    TT_CALL_DEPTH,
    TT_AASRT_P_COUNT,
    TT_CURR_T_COUNT,
    TT_ERR_MSG,
    TT_COUNT
};

static const char *tag_open_str[] =
{
    "<aasrt_result>",   
    "<done>",
    "<pass>",
    "<file_name>",
    "<func_name>",
    "<line_num>",
    "<call_depth>",
    "<aasrt_p_count>",
    "<curr_t_count>",
    "<err_msg>"
};

static const char *tag_close_str[] =
{
    "</aasrt_result>",   
    "</done>",
    "</pass>",
    "</file_name>",
    "</func_name>",
    "</line_num>",
    "</call_depth>",
    "</aasrt_p_count>",
    "</curr_t_count>",
    "</err_msg>"
};

std::vector<size_t> tg_op;
std::vector<size_t> tg_cl;
cf01_aasrt_result *aasrt_result = NULL;
cf01_aasrt_result *aasrt_result_pass = NULL; /* last passing result*/
cf01_aasrt_result *aasrt_result_fail = NULL; /* last failing result*/
static const size_t idx_not_found = std::numeric_limits<size_t>::max();
size_t i=0, j=0, k=0, aar_begin_idx=idx_not_found, aar_end_idx=idx_not_found;
tg_op.resize(TT_COUNT, idx_not_found);
tg_cl.resize(TT_COUNT, idx_not_found);
/*
token parse mode
   find "<aasrt_result>" => create new cf01_aasrt_result  
       seek "</aasrt_result>"
       parse all tokens in between

   find "<done>" =>

*/
for(i = 0; i < tokens.size(); ++i){
    const std::string& tok = tokens.at(i);
    bool match_found = false;
    for(j=0;(j < TT_COUNT) && !match_found; ++j){
        if(tok == tag_open_str[j]){
            tg_op.at(j) = i; match_found=true; }
        if(tok == tag_close_str[j]){
            tg_cl.at(j) = i; match_found=true; }
        }

    if ((tg_op.at(TT_AASRT_RESULT) != idx_not_found) &&
        (tg_cl.at(TT_AASRT_RESULT) != idx_not_found)) {
        aasrt_result = new cf01_aasrt_result();

        //std::cout << "[" << i << "]" << tokens.at(i) << "\n";
        //std::cout << "tg_op.at(TT_AASRT_RESULT)" << ((int)tg_op.at(TT_AASRT_RESULT)) << "\n";
        //std::cout << "tg_cl.at(TT_AASRT_RESULT)" << ((int)tg_cl.at(TT_AASRT_RESULT)) << "\n";
        //std::cout << "tg_op.at(TT_CALL_DEPTH)" << ((int)tg_op.at(TT_CALL_DEPTH)) << "\n";
        //std::cout << "tg_cl.at(TT_CALL_DEPTH)" << ((int)tg_cl.at(TT_CALL_DEPTH)) << "\n";

        if ((tg_op.at(TT_DONE) != idx_not_found) &&
            (tg_cl.at(TT_DONE) != idx_not_found) &&
            ((tg_op.at(TT_DONE)+1) < tg_cl.at(TT_DONE))){
            const std::string& val_tok = tokens.at(tg_op.at(TT_DONE)+1);
            aasrt_result->m_done = (val_tok=="true") ? true : false;
            }

        if ((tg_op.at(TT_PASS) != idx_not_found) &&
            (tg_cl.at(TT_PASS) != idx_not_found) &&
            ((tg_op.at(TT_PASS)+1) < tg_cl.at(TT_PASS))){
            const std::string& val_tok = tokens.at(tg_op.at(TT_PASS)+1);
            aasrt_result->m_pass = (val_tok=="true") ? true : false;
            }

        if ((tg_op.at(TT_FILE_NAME) != idx_not_found) &&
            (tg_cl.at(TT_FILE_NAME) != idx_not_found) &&
            ((tg_op.at(TT_FILE_NAME)+1) < tg_cl.at(TT_FILE_NAME))){
            for(j=tg_op.at(TT_FILE_NAME)+1; j<tg_cl.at(TT_FILE_NAME);++j){
                const std::string& val_tok = tokens.at(j);
                (aasrt_result->m_file_name)+=val_tok;
                }
            }

        if ((tg_op.at(TT_FUNC_NAME) != idx_not_found) &&
            (tg_cl.at(TT_FUNC_NAME) != idx_not_found) &&
            ((tg_op.at(TT_FUNC_NAME)+1) < tg_cl.at(TT_FUNC_NAME))){
            for(j=tg_op.at(TT_FUNC_NAME)+1; j<tg_cl.at(TT_FUNC_NAME);++j){
                const std::string& val_tok = tokens.at(j);
                (aasrt_result->m_func_name)+=val_tok;
                }
            }

        if ((tg_op.at(TT_LINE_NUM) != idx_not_found) &&
            (tg_cl.at(TT_LINE_NUM) != idx_not_found) &&
            ((tg_op.at(TT_LINE_NUM)+1) < tg_cl.at(TT_LINE_NUM))){
            const std::string& val_tok = tokens.at(tg_op.at(TT_LINE_NUM)+1);
            aasrt_result->m_line_num = atoi(val_tok.c_str());
            }

        if ((tg_op.at(TT_CALL_DEPTH) != idx_not_found) &&
            (tg_cl.at(TT_CALL_DEPTH) != idx_not_found) &&
            ((tg_op.at(TT_CALL_DEPTH)+1) < tg_cl.at(TT_CALL_DEPTH))){
            const std::string& val_tok = tokens.at(tg_op.at(TT_CALL_DEPTH)+1);
            aasrt_result->m_call_depth =
                static_cast<cf01_uint8>(atoi(val_tok.c_str()));
            }

        if ((tg_op.at(TT_AASRT_P_COUNT) != idx_not_found) &&
            (tg_cl.at(TT_AASRT_P_COUNT) != idx_not_found) &&
            ((tg_op.at(TT_AASRT_P_COUNT)+1) < tg_cl.at(TT_AASRT_P_COUNT))){
            const std::string& val_tok=tokens.at(tg_op.at(TT_AASRT_P_COUNT)+1);
            std::vector<cf01_uint64> p_count_vec;
            parse_uint64_csv(val_tok, &p_count_vec);
            for(j=0; ((j < p_count_vec.size()) &&
                (j < CF01_AASRT_CALL_DEPTH_COUNT)); ++j){
                (aasrt_result->m_aasrt_p_count)[j]=p_count_vec.at(j);
                }
            }

        if ((tg_op.at(TT_CURR_T_COUNT) != idx_not_found) &&
            (tg_cl.at(TT_CURR_T_COUNT) != idx_not_found) &&
            ((tg_op.at(TT_CURR_T_COUNT)+1) < tg_cl.at(TT_CURR_T_COUNT))){
            const std::string& val_tok=tokens.at(tg_op.at(TT_CURR_T_COUNT)+1);
            std::vector<cf01_uint64> t_count_vec;
            parse_uint64_csv(val_tok, &t_count_vec);
            for(j=0; ((j < t_count_vec.size()) &&
                (j < CF01_AASRT_CALL_DEPTH_COUNT)); ++j){
                (aasrt_result->m_curr_t_count)[j]=t_count_vec.at(j);
                }
            }

        if ((tg_op.at(TT_ERR_MSG) != idx_not_found) &&
            (tg_cl.at(TT_ERR_MSG) != idx_not_found) &&
            ((tg_op.at(TT_ERR_MSG)+1) < tg_cl.at(TT_ERR_MSG))){
            for(j=tg_op.at(TT_ERR_MSG)+1; j<tg_cl.at(TT_ERR_MSG);++j){
                const std::string& val_tok = tokens.at(j);
                (aasrt_result->m_err_msg)+=val_tok;
                }
            }

        if(aasrt_result->m_done){
            if(aasrt_result->m_pass){
                if(aasrt_result_pass != NULL){
                    delete aasrt_result_pass;
                    }
                aasrt_result_pass = aasrt_result;
                }
            else{
                if(aasrt_result_fail != NULL){
                    delete aasrt_result_fail;
                    }
                aasrt_result_fail = aasrt_result;
                }
            }
        else{
            delete aasrt_result;
            }
        aasrt_result = NULL;

        tg_op.clear();
        tg_cl.clear();
        tg_op.resize(TT_COUNT, idx_not_found);
        tg_cl.resize(TT_COUNT, idx_not_found);
        }

    }


if(NULL == aasrt_result_fail){
    init_default();
    }
else{
    std::cout << "aasrt_result_fail:\n";
    std::cout << " done="<<((aasrt_result_fail->m_done)?"true":"false")<<"\n";
    std::cout << " pass="<<((aasrt_result_fail->m_pass)?"true":"false")<<"\n";
    std::cout << " file_name="<<(aasrt_result_fail->m_file_name)<<"\n";
    std::cout << " m_func_name="<<(aasrt_result_fail->m_func_name)<<"\n";
    std::cout << " line_num="<<(aasrt_result_fail->m_line_num)<<"\n";
    std::cout << " call_depth="<<((int)(aasrt_result_fail->m_call_depth))<<"\n";
    std::cout << " aasrt_p_count=";
    for(j=0; j < CF01_AASRT_CALL_DEPTH_COUNT; ++j){
        if(j>0){ std::cout<<","; }
        std::cout << ((aasrt_result_fail->m_aasrt_p_count)[j]);
        }
    std::cout << "\n";
    std::cout << " curr_t_count=";
    for(j=0; j < CF01_AASRT_CALL_DEPTH_COUNT; ++j){
        if(j>0){ std::cout<<","; }
        std::cout << ((aasrt_result_fail->m_curr_t_count)[j]);
        }
    std::cout << "\n";
    std::cout << " err_msg="<<(aasrt_result_fail->m_err_msg)<<"\n";

    cf01_call_idx_range_crit idx_range_crit;
    for(size_t call_depth = 0; call_depth < CF01_AASRT_CALL_DEPTH_COUNT;
        ++call_depth){
        cf01_uint64 p_count = (aasrt_result_fail->m_aasrt_p_count)[call_depth];
        if(p_count > 0){
            const cf01_uint64 intense_search_count =  (p_count > 128) ?
                (p_count / 8) : (p_count >= 16)? 16 : p_count;
            const cf01_uint64 count_2 = p_count - intense_search_count;
            const cf01_uint64 medium_search_count =  (count_2 > 128) ?
                (count_2 / 4) : (count_2 >= 32)? 32 : count_2;
            const cf01_uint64 range_end_intense_search = p_count+1;
                assert(range_end_intense_search >= intense_search_count);
            const cf01_uint64 range_end_medium_search = 
                range_end_intense_search - intense_search_count;
                assert(range_end_medium_search >= medium_search_count);
            const cf01_uint64 range_end_sparse_search =
                range_end_medium_search - medium_search_count;

            /* sparse search at beginning */
            if((range_end_sparse_search > 0) &&
                (range_end_sparse_search < range_end_medium_search)){
                idx_range_crit.m_call_idx_range_end = range_end_sparse_search;
                idx_range_crit.m_call_idx_div = 32; 
                idx_range_crit.m_call_idx_mod = 0;
                add_aasrt_call_idx_range_crit(call_depth, &idx_range_crit,
                    NULL, NULL);
                }

            /* medium search, nearing failure point */
            if((range_end_medium_search > 0 ) &&
                (range_end_medium_search < range_end_intense_search)){
                idx_range_crit.m_call_idx_range_end = range_end_medium_search;
                idx_range_crit.m_call_idx_div = 8; 
                idx_range_crit.m_call_idx_mod = 0;
                add_aasrt_call_idx_range_crit(call_depth, &idx_range_crit,
                    NULL, NULL);
                }

            /* intense search, directly preceding known failure point */
            idx_range_crit.m_call_idx_range_end = range_end_intense_search;
            idx_range_crit.m_call_idx_div = 1; 
            idx_range_crit.m_call_idx_mod = 0;
            add_aasrt_call_idx_range_crit(call_depth, &idx_range_crit,
                NULL, NULL);

            /* after known failure, sparse search */
            idx_range_crit.m_call_idx_range_end = 0;
            idx_range_crit.m_call_idx_div = 64; 
            idx_range_crit.m_call_idx_mod = 0;
            add_aasrt_call_idx_range_crit(call_depth, &idx_range_crit,
                NULL, NULL);
            }
        else{
            /* default */
            idx_range_crit.m_call_idx_range_end = 32;
            idx_range_crit.m_call_idx_div = 4; 
            idx_range_crit.m_call_idx_mod = 0;
            add_aasrt_call_idx_range_crit(call_depth, &idx_range_crit,
                NULL, NULL);
            
            idx_range_crit.m_call_idx_range_end = 256;
            idx_range_crit.m_call_idx_div = 16; 
            idx_range_crit.m_call_idx_mod = 0;
            add_aasrt_call_idx_range_crit(call_depth, &idx_range_crit,
                NULL, NULL);
            
            idx_range_crit.m_call_idx_range_end = 4096;
            idx_range_crit.m_call_idx_div = 32; 
            idx_range_crit.m_call_idx_mod = 0;
            add_aasrt_call_idx_range_crit(call_depth, &idx_range_crit,
                NULL, NULL);
            
            idx_range_crit.m_call_idx_range_end = 0;
            idx_range_crit.m_call_idx_div = 64; 
            idx_range_crit.m_call_idx_mod = 0;
            add_aasrt_call_idx_range_crit(call_depth, &idx_range_crit,
                NULL, NULL);
            }
        }
    complete_table();
    }

if(NULL != aasrt_result_pass){
    delete aasrt_result_pass; 
    }
if(NULL != aasrt_result_fail){
    delete aasrt_result_fail; 
    }

#ifndef NDEBUG
    {
    std::string err_msg;
    int err_cnt = verify_data(&err_msg);
    if(err_cnt > 0){
        std::cout << "err_cnt=" << err_cnt << "  err_msg=" << err_msg << "\n";
        }
    }
#endif
}

/* add index range criteria 
*result==true => criteria successfully added */
void cf01_auto_assert_wksp::add_aasrt_call_idx_range_crit(
    const cf01_uint8& depth,
    const cf01_call_idx_range_crit* idx_range_crit,
    bool *result,
    std::string *err_str )
{
assert( depth < CF01_AASRT_CALL_DEPTH_COUNT);
assert(NULL != idx_range_crit);

cf01_uint8 idx_crit_idx = 0;
bool found = false;
bool success = false;
cf01_call_idx_range_crit* c=NULL;
while(( !found ) && (idx_crit_idx < CF01_AASRT_CALL_IDX_RANGE_COUNT ) )
    {
    /* if existing div==0, then the it is considered uninitialized, so it is
    the correct point to insert new data. */
    c = &( m_depth_call_idx_range_crit[depth][idx_crit_idx] );
    if( c-> m_call_idx_div == 0 ) {
	    found = true;
        success = true;
	    }
    else if( (idx_range_crit->m_call_idx_range_end > 0) &&
        (idx_range_crit->m_call_idx_range_end < c->m_call_idx_range_end) ){
    	found = true;
        success = false;
        if(NULL != err_str){
            err_str->append("criteria overwritten");
            }
	    }
    else{
	    ++idx_crit_idx;	}
    }
if(found){
    assert(NULL != c);

    /* copy */
    c->m_call_idx_range_end = idx_range_crit->m_call_idx_range_end;
    c->m_call_idx_div = idx_range_crit->m_call_idx_div;
    c->m_call_idx_mod = idx_range_crit->m_call_idx_mod;
    }
else {
    success = false;
    if(NULL != err_str){
        err_str->append("failed to add criteria");
        }
    }

if(NULL != result){
    *result = success;
    }
}

/* complete them_depth_call_idx_range_crit table by
copying data from higher rows to adjacent lower
uninitialized rows */ 
void cf01_auto_assert_wksp::complete_table()
{
cf01_uint8 d;
for( d = 1; d < CF01_AASRT_CALL_DEPTH_COUNT; ++d )
    {
    cf01_call_idx_range_crit* c = &( m_depth_call_idx_range_crit[d][0] );

    if( ( c->m_call_idx_range_end == 0 ) &&
        ( c->m_call_idx_div == 0 ) &&
        ( c->m_call_idx_mod == 0 ) ){
        /* copy row from above */
        cf01_uint8 idx_crit_idx;
        for( idx_crit_idx = 0; 
            idx_crit_idx < CF01_AASRT_CALL_IDX_RANGE_COUNT; 
            ++idx_crit_idx ) {
            cf01_call_idx_range_crit* c_master =
                &( m_depth_call_idx_range_crit[d-1][idx_crit_idx] );
            c = &( m_depth_call_idx_range_crit[d][idx_crit_idx] );
            
            /* copy */
            c->m_call_idx_range_end = c_master->m_call_idx_range_end;
            c->m_call_idx_div = c_master->m_call_idx_div;
            c->m_call_idx_mod = c_master->m_call_idx_mod;
            }
	    }
    }
}


/* increment p_count.  Call this when a standard assertion is reached */
void cf01_auto_assert_wksp::incr_p_count()
{
cf01_uint64 p_cnt;
cf01_uint8 idx_crit_idx;

/* update m_curr_p_count[] */
p_cnt = m_curr_p_count[m_call_depth];
if( p_cnt < std::numeric_limits<cf01_uint64>::max() )
    {
    ++p_cnt;
    }
m_curr_p_count[m_call_depth] = p_cnt;

/* update m_idx_crit_idx[] */
idx_crit_idx = m_idx_crit_idx[m_call_depth];
if( idx_crit_idx < ( CF01_AASRT_CALL_IDX_RANGE_COUNT - 1 ) )
    {
    cf01_call_idx_range_crit const *idx_range_criteria =
         &( m_depth_call_idx_range_crit[m_call_depth][idx_crit_idx] );

    if( 0 == idx_range_criteria->m_call_idx_range_end ) 
        {
        /* range_end==0 => unlimited range for idx_range_criteria. no update */
        }
    else if(p_cnt < idx_range_criteria->m_call_idx_range_end)
        {
        /* p_cnt is within range for idx_range_criteria. no update */
        }
    else
        {
        /* p_cnt is past the range for idx_range_criteria. do update.
        first, check that p_cnt is only one past the range */
        assert( p_cnt == idx_range_criteria->m_call_idx_range_end );
        ++idx_crit_idx;
        m_idx_crit_idx[m_call_depth] = idx_crit_idx; 
        }
    }
} 


/* true => should run standard assertion.  This does not apply to
extra debug assertions or code. */
bool cf01_auto_assert_wksp::should_run_aasrt() const
{
/* get criteria */
assert( m_call_depth < CF01_AASRT_CALL_DEPTH_COUNT);
const cf01_uint8 idx_crit_idx = m_idx_crit_idx[m_call_depth];
assert( idx_crit_idx < CF01_AASRT_CALL_IDX_RANGE_COUNT);
const cf01_call_idx_range_crit *idx_range_criteria =
    &( m_depth_call_idx_range_crit[m_call_depth][idx_crit_idx] );

/* Get p_count.  Should be within range of idx_range_criteria */
const cf01_uint8 p_cnt = m_curr_p_count[m_call_depth]; 
assert((idx_range_criteria->m_call_idx_range_end) == 0 || 
       ( p_cnt < idx_range_criteria->m_call_idx_range_end ) );

/* if p_count % div == mod, then assertion should be run */
bool result = ( ( idx_range_criteria->m_call_idx_div > 0) && 
    ( p_cnt % (idx_range_criteria->m_call_idx_div) ==
      ( idx_range_criteria->m_call_idx_mod ) ) ) ? true : false;
return result;
}

/* true-> extra debug code and assertions should be run */
bool cf01_auto_assert_wksp::should_run_xdbg(const cf01_uint8& dbg_lvl) const
{
assert(m_call_depth < CF01_AASRT_CALL_DEPTH_COUNT); 

bool result = false;
const cf01_uint64 t_count = m_curr_t_count[m_call_depth];
 
/* Extra debug code and assertions with debug level at or below
m_xdbg_ctrl_dbg_lvl[d] will be executed when transition count is at or 
above m_xdbg_ctrl_t_count[d] */
if( ( t_count >= m_xdbg_ctrl_t_count[m_call_depth] ) &&
    ( dbg_lvl <= m_xdbg_ctrl_dbg_lvl[m_call_depth] ) )
    {
    if( m_aasrt_result_first_failed.m_done )
        {
        /* if an assertion has already failed,
        then don't run any more extra debug code */
        assert( !m_aasrt_result_first_failed.m_pass );
        assert(!result);
        }
    else
        {
        /* if all auto-assertions so far have passed,
        then run extra debug code */
        result = true;
        }
    }
   
return result;
}

void cf01_auto_assert_wksp::report_aasrt_result(
    const bool result,
    const char *file_name,
    const int line_num,
    const char *function,
    const char *condition_str){
bool should_save_to_file = false;
cf01_uint16 i;
cf01_aasrt_result *aasrt_result = NULL;
if(result){
    aasrt_result = &m_aasrt_result_last_passed; }
else if( !m_aasrt_result_first_failed.m_done ) {
    aasrt_result = &m_aasrt_result_first_failed;
    should_save_to_file = true; }

if(NULL != aasrt_result){
    aasrt_result->m_done = true;
    aasrt_result->m_pass = result;
    (aasrt_result->m_file_name).assign(file_name);
    (aasrt_result->m_func_name).assign(function);
    aasrt_result->m_line_num = line_num;
    aasrt_result->m_call_depth = m_call_depth;
    for(i = 0; i < CF01_AASRT_CALL_DEPTH_COUNT; ++i){
        aasrt_result->m_aasrt_p_count[i] = m_curr_p_count[i]; 
        aasrt_result->m_curr_t_count[i] = m_curr_t_count[i];
        }

    /* error message = condition string + utility error message */
    (aasrt_result->m_err_msg).assign(condition_str);
    (aasrt_result->m_err_msg)+="\n";
    (aasrt_result->m_err_msg).insert( (aasrt_result->m_err_msg).end(),
        &(m_utility_err_buf[0]), &(m_utility_err_buf[m_utility_err_buf_pos]));

    if( should_save_to_file ){
        /* clear utility buffer */
        clear_utility_err_buf();
        }
    }

if((NULL != aasrt_result) && should_save_to_file){
    save_aasrt_result_to_file(aasrt_result);
    }
}


/* debug dump */
std::ostream& cf01_auto_assert_wksp::ostream_output(std::ostream& os) const
{
size_t call_depth;
size_t i;
size_t end_idx;

os << "<cf01_auto_assert_wksp>\n";
os << "  <this>" << std::hex << reinterpret_cast<cf01_uint64>(this)
     << std::dec << "</this>\n";
os << "  <call_depth>" << (int)m_call_depth << "</call_depth>\n";

os << "  <curr_p_count>";
end_idx = CF01_AASRT_CALL_DEPTH_COUNT;
do{ --end_idx; }
    while( (end_idx > 0) && (m_curr_p_count[end_idx] == 0) );
for(i = 0; i <= end_idx; ++i){
    cf01_uint64 p_count = m_curr_p_count[i];
    if(i>0){os << ",";}
    os << p_count;
    }
os << "</aasrt_p_count>\n";

os << "  <curr_t_count>";
end_idx = CF01_AASRT_CALL_DEPTH_COUNT;
do{ --end_idx; }
    while( (end_idx > 0) && (m_curr_t_count[end_idx] == 0) );
for(i = 0; i <= end_idx; ++i){
    cf01_uint64 t_count = m_curr_t_count[i];
    if(i>0){os << ",";}
    os << t_count;
    }
os << "</curr_t_count>\n";


os << "  <depth_call_idx_range_crit>\n";
for(call_depth = 0; call_depth<CF01_AASRT_CALL_DEPTH_COUNT; ++call_depth){
    bool should_output_at_depth = false;
    if(call_depth == 0){ 
        should_output_at_depth = true; }
    else{ /* compare to previous depth*/
        for( i = 0; (i < CF01_AASRT_CALL_IDX_RANGE_COUNT) &&
            !should_output_at_depth; ++i){
            const cf01_call_idx_range_crit *c_prev = 
                &(m_depth_call_idx_range_crit[call_depth-1][i]);
            const cf01_call_idx_range_crit *c = 
                &(m_depth_call_idx_range_crit[call_depth][i]);
            if(memcmp(c_prev,c,sizeof(cf01_call_idx_range_crit))!=0){
                should_output_at_depth = true;}/* different from prev depth */
            }
        }
    if(should_output_at_depth){
        for( i = 0; (i < CF01_AASRT_CALL_IDX_RANGE_COUNT); ++i){
            const cf01_call_idx_range_crit *c = 
                &(m_depth_call_idx_range_crit[call_depth][i]);
            if( (c->m_call_idx_range_end != 0) ||
                (c->m_call_idx_div != 0) ||
                (c->m_call_idx_mod != 0)){
                os << "    <crit>depth=" << static_cast<int>(call_depth)
                   << ",range_end=" << (c->m_call_idx_range_end)
                   << ",div=" << (c->m_call_idx_div)
                   << ",mod=" << (c->m_call_idx_mod)
                   << "</crit>\n";
                }
            }
        }
    }

end_idx = CF01_AASRT_CALL_DEPTH_COUNT;
do{ --end_idx; }
    while( (end_idx > 0) && (m_curr_t_count[end_idx] == 0) );
for(i = 0; i <= end_idx; ++i){
    cf01_uint64 t_count = m_curr_t_count[i];
    if(i>0){os << ",";}
    os << t_count;
    }
os << "</depth_call_idx_range_crit>\n";


os << "</cf01_auto_assert_wksp>\n";
return os;
}


int cf01_auto_assert_wksp::verify_data( std::string *err_str ) const{
int err_cnt = 0;
size_t call_depth, i;
const cf01_call_idx_range_crit *c, *c_prev;

/* m_call_depth */

/* m_curr_p_count */

/* m_curr_t_count */
for( call_depth = 2; call_depth < CF01_AASRT_CALL_DEPTH_COUNT; ++call_depth){
    if((m_curr_p_count[call_depth-1]==0) &&
       (m_curr_p_count[call_depth]>0)){
       ++err_cnt;
       err_str->append("  m_curr_p_count error");
       }
    }

/* m_depth_call_idx_range_crit */
for( call_depth = 0; call_depth < CF01_AASRT_CALL_DEPTH_COUNT; ++call_depth){
    for(i = 0; i < CF01_AASRT_CALL_IDX_RANGE_COUNT; ++i){
        c = &(m_depth_call_idx_range_crit[call_depth][i]);
        c_prev=(i>0) ? &(m_depth_call_idx_range_crit[call_depth][i-1]) : NULL;
        if( (c->m_call_idx_div > 0) &&
            (c->m_call_idx_mod >= c->m_call_idx_div)){
            ++err_cnt;
            err_str->append("  mod>=div");
            }
        if(NULL != c_prev){
            if( (c_prev->m_call_idx_range_end == 0) && 
                (c->m_call_idx_range_end > 0 ) ){
                ++err_cnt;
                err_str->append("  prev_range_end==0,range_end>0");
                }
            if( (c_prev->m_call_idx_range_end > 0) && 
                (c->m_call_idx_range_end > 0 ) && 
                (c_prev->m_call_idx_range_end >= c->m_call_idx_range_end ) ){
                ++err_cnt;
                err_str->append("  prev_range_end>=range_end");
                }
            }
        }
    }

return err_cnt;
}


void cf01_auto_assert_wksp::save_aasrt_result_to_file(
    const cf01_aasrt_result *r){
static const char *jrnl_file_name = "cf01_jrnl.txt";
cf01_uint16 i, end_idx;
assert(NULL != r);
std::ofstream os;
os.open( jrnl_file_name, std::ofstream::out | std::ofstream::app );
if(!os.is_open()){
    std::cout << "failed to open " << jrnl_file_name << std::endl; }
else{
    time_t now = time(NULL);
    std::string now_str(ctime(&now));
    while((now_str.length() > 0 )&& isspace(now_str.at(now_str.length()-1))){
        now_str.resize(now_str.length()-1); }
    os << "\n<aasrt_result>\n";
    os << "  <time_t>" << static_cast<unsigned long>(now) << "</time_t>\n";
    os << "  <clock_t>" << static_cast<unsigned long>(clock()) << "</clock_t>\n";
    os << "  <ctime>" << now_str << "</ctime>\n";
    os << "  <done>" << ((r->m_done)?"true":"false") << "</done>\n";
    os << "  <pass>" << ((r->m_pass)?"true":"false") << "</pass>\n";
    os << "  <file_name>" << (r->m_file_name) << "</file_name>\n";
    os << "  <func_name>" << (r->m_func_name) << "</func_name>\n";
    os << "  <line_num>" << (r->m_line_num) << "</line_num>\n";
    os << "  <call_depth>" << static_cast<unsigned int>(r->m_call_depth) << "</call_depth>\n";

    os << "  <aasrt_p_count>";
    end_idx = CF01_AASRT_CALL_DEPTH_COUNT;
    do{ --end_idx; }
        while( (end_idx > 0) && ((r->m_aasrt_p_count)[end_idx] == 0) );
    for(i = 0; i <= end_idx; ++i){
        cf01_uint64 p_count = (r->m_aasrt_p_count)[i];
        if(i>0){os << ",";}
        os << p_count;
        }
    os << "</aasrt_p_count>\n";

    os << "  <curr_t_count>";
    end_idx = CF01_AASRT_CALL_DEPTH_COUNT;
    do{ --end_idx; }
        while( (end_idx > 0) && ((r->m_curr_t_count)[end_idx] == 0) );
    for(i = 0; i <= end_idx; ++i){
        cf01_uint64 t_count = (r->m_curr_t_count)[i];
        if(i>0){os << ",";}
        os << t_count;
        }
    os << "</curr_t_count>\n";
    
    os << "  <err_msg>" << (r->m_err_msg) << "</err_msg>\n";
    os << "\n</aasrt_result>\n";
    os.close();
    }
}



