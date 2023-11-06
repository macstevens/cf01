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
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if(( __cplusplus > 199711L ) || ( defined(_MSVC_LANG) && (_MSVC_LANG > 199711L ) )) 
#include <tuple>
#endif

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


void cf01_hc_chk_cmd_info::init(const char *file_name, 
    const int line_num, const char *function,
    const char *hash_cmd_str, const cf01_uint64& check_index ){
m_initialized =  true;
m_check_index = check_index;
m_line_num = line_num;
strncpy( m_file_name, file_name, sizeof(m_file_name) );
strncpy( m_func_name, function, sizeof(m_func_name) );
strncpy( m_hash_cmd_str, hash_cmd_str, sizeof(m_hash_cmd_str) );
}


void cf01_hc_chk_rcrd::ostream_output(std::ostream& os,
    const std::string& line_prefix) const{
os << line_prefix << "<hc_chk_rcrd>\n";
os << line_prefix << "  <rcrd_type>" << static_cast<int>(m_rcrd_type)
    << "</rcrd_type>\n";
os << line_prefix << "  <mismatch_prev_run>"
    << ( m_mismatch_prev_run ? "true" : "false" )
    << "</mismatch_prev_run>\n";
os << line_prefix << "  <time_stamp>" << ( m_time_stamp )
    << "</time_stamp>\n";
os << line_prefix << "  <time_stamp_ctime>" << cf01_ctime(m_time_stamp)
    << "</time_stamp_ctime>\n";
os << line_prefix << "  <check_index>" << m_check_index
    << "</check_index>\n";
os << line_prefix << "  <hash>" << cf01_uint64_to_hex_str( m_hash )
    << "</hash>\n";
if( NULL != m_cmd_info ){
    os << line_prefix << "  <cmd_info>\n";
    os << "          <initialized>"
        << ( m_cmd_info->m_initialized ? "true" : "false" )
        << "</initialized>\n";
    os << line_prefix << "    <check_index>" << ( m_cmd_info->m_check_index )
        << "</check_index>\n";
    os << line_prefix << "    <line_num>" << ( m_cmd_info->m_line_num )
        << "</line_num>\n";
    os << line_prefix << "    <file_name>" << ( m_cmd_info->m_file_name )
        << "</file_name>\n";
    os << line_prefix << "    <func_name>" << ( m_cmd_info->m_func_name )
        << "</func_name>\n";
    os << line_prefix << "    <hash_cmd>" << ( m_cmd_info->m_hash_cmd_str )
        << "</hash_cmd>\n";
    os << line_prefix << "  </cmd_info>\n";
    }
os << line_prefix << "</hc_chk_rcrd>\n";
}


cf01_auto_assert_wksp cf01_auto_assert_wksp::m_instance;

const char * const cf01_auto_assert_wksp::m_jrnl_file_name = "cf01_jrnl.txt";

cf01_auto_assert_wksp::cf01_auto_assert_wksp():
    m_wksp_enabled(false),
    m_jrnl_write_mode(CF01_JRNL_WRITE_MODE_DEFAULT),
    m_jrnl_write_started(false),
    m_call_depth(0),
    m_call_depth_unlimited(0),
    m_aasrt_result_last_passed(),
    m_aasrt_result_first_failed(),
    m_start_time(0),
    m_publish_deadline(0),
    m_hc_chk_rcrd_free_chain(NULL)
{
memset(m_curr_p_count, 0, sizeof(m_curr_p_count));
memset(m_curr_t_count, 0, sizeof(m_curr_t_count));
memset(m_depth_call_idx_range_crit, 0, sizeof(m_depth_call_idx_range_crit));
memset(m_idx_crit_idx, 0, sizeof(m_idx_crit_idx));
memset(m_xdbg_ctrl_t_count, 0, sizeof(m_xdbg_ctrl_t_count));
memset(m_xdbg_ctrl_dbg_lvl, 0, sizeof(m_xdbg_ctrl_dbg_lvl));
clear_utility_err_buf();
memset(m_hc_chk_depth_rcrds, 0, sizeof(m_hc_chk_depth_rcrds));
memset(m_hc_chk_rcrd_pool, 0, sizeof(m_hc_chk_rcrd_pool));
init_hc_chk();
init_from_journal_file();
}

cf01_auto_assert_wksp::~cf01_auto_assert_wksp() {
publish_hc_chk_rcrds();
if( m_jrnl_write_started ){
    write_journal_end();
    }
}

void cf01_auto_assert_wksp::enable_wksp(const bool& wksp_enabled){
m_wksp_enabled = wksp_enabled;
}

void cf01_auto_assert_wksp::set_jrnl_write_mode(
    const cf01_jrnl_write_mode& m, const char *file_name,
    const int line_num, const char *function){
assert( static_cast<int>(m) >= 0 );
assert( m < CF01_JRNL_WRITE_MODE_COUNT );
m_jrnl_write_mode = m;
}

void cf01_auto_assert_wksp::init_aa_default(){
cf01_call_idx_range_crit idx_range_crit = {};
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

complete_depth_call_idx_range_crit_table();
assert(0 == verify_data(NULL));
}

void cf01_auto_assert_wksp::init_from_journal_file(){
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
is.open(m_jrnl_file_name);
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
    TT_AASRT_P_COUNT,
    TT_AASRT_RESULT,
    TT_CALL_DEPTH,
    TT_CHECK_INDEX,
    TT_CURR_T_COUNT,
    TT_DONE,
    TT_ERR_MSG,
    TT_FILE_NAME,
    TT_FUNC_NAME,
    TT_HASH,
    TT_HASH_CMD,
    TT_HC_CHK_RCRD,
    TT_JRNL_START,
    TT_JRNL_END,
    TT_LINE_NUM,
    TT_MISMATCH_PREV_RUN,
    TT_PASS,
    TT_TIME_STAMP,
    TT_TIME_STAMP_CTIME,  

    TT_COUNT
};

static const char *tag_open_str[] =
{
    "<aasrt_p_count>",
    "<aasrt_result>",
    "<call_depth>",
    "<check_index>",
    "<curr_t_count>",
    "<done>",
    "<err_msg>",
    "<file_name>",
    "<func_name>",
    "<hash>",
    "<hash_cmd>",
    "<hc_chk_rcrd>",
    "<jrnl_start>",
    "<jrnl_end>",
    "<line_num>",
    "<mismatch_prev_run>",
    "<pass>",
    "<time_stamp>",
    "<time_stamp_ctime>"
};

static const char *tag_close_str[] =
{
    "</aasrt_p_count>",
    "</aasrt_result>",
    "</call_depth>",
    "</check_index>",
    "</curr_t_count>",
    "</done>",
    "</err_msg>",
    "</file_name>",
    "</func_name>",
    "</hash>",
    "</hash_cmd>",
    "</hc_chk_rcrd>",
    "</jrnl_start>",
    "</jrnl_end>",
    "</line_num>",
    "</mismatch_prev_run>",
    "</pass>",
    "</time_stamp>",
    "</time_stamp_ctime>"
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

    if ((tg_op.at(TT_JRNL_START) != idx_not_found) &&
        (tg_cl.at(TT_JRNL_START) != idx_not_found)) {
        /* clear */
        clear_hc_chk_depth_rcrds();
        tg_op.clear();
        tg_cl.clear();
        tg_op.resize(TT_COUNT, idx_not_found);
        tg_cl.resize(TT_COUNT, idx_not_found);
        }

    if ((tg_op.at(TT_JRNL_END) != idx_not_found) &&
        (tg_cl.at(TT_JRNL_END) != idx_not_found)) {
        tg_op.clear();
        tg_cl.clear();
        tg_op.resize(TT_COUNT, idx_not_found);
        tg_cl.resize(TT_COUNT, idx_not_found);
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
                static_cast<cf01_depth_t>(strtoull(val_tok.c_str(), NULL, 10));
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

    if ((tg_op.at(TT_HC_CHK_RCRD) != idx_not_found) &&
        (tg_cl.at(TT_HC_CHK_RCRD) != idx_not_found)) {

        cf01_hc_chk_rcrd *rcrd = alloc_hc_chk_rcrd();
        if( NULL != rcrd ){
            assert( CF01_HC_CHK_RCRD_TYPE_NONE == rcrd->m_rcrd_type );
            assert( false == rcrd->m_mismatch_prev_run );
            assert( 0 == rcrd->m_time_stamp );
            assert( 0 == rcrd->m_check_index );
            assert( 0 == rcrd->m_hash );
            assert( NULL == rcrd->m_cmd_info );
            assert( NULL == rcrd->m_next );

            cf01_depth_t call_depth = 0;
            if ((tg_op.at(TT_CALL_DEPTH) != idx_not_found) &&
                (tg_cl.at(TT_CALL_DEPTH) != idx_not_found) &&
                ((tg_op.at(TT_CALL_DEPTH)+1) < tg_cl.at(TT_CALL_DEPTH))){
                const std::string& val_tok = 
                    tokens.at(tg_op.at(TT_CALL_DEPTH)+1);
                call_depth = static_cast<cf01_depth_t>(
                    strtoull(val_tok.c_str(), NULL, 10));
                if( call_depth >= CF01_AASRT_CALL_DEPTH_COUNT ){
                    call_depth = CF01_AASRT_CALL_DEPTH_COUNT - 1;
                    }
                }

            /* m_rcrd_type always = CF01_HC_CHK_RCRD_TYPE_DONE_PREV_RUN */
            rcrd->m_rcrd_type = CF01_HC_CHK_RCRD_TYPE_DONE_PREV_RUN;

            if ((tg_op.at(TT_MISMATCH_PREV_RUN) != idx_not_found) &&
                (tg_cl.at(TT_MISMATCH_PREV_RUN) != idx_not_found) &&
                ((tg_op.at(TT_MISMATCH_PREV_RUN)+1) < 
                   tg_cl.at(TT_MISMATCH_PREV_RUN))){
                const std::string& val_tok =
                    tokens.at(tg_op.at(TT_MISMATCH_PREV_RUN)+1);
                rcrd->m_mismatch_prev_run = (val_tok=="true") ? true : false;
                }

            if ((tg_op.at(TT_TIME_STAMP) != idx_not_found) &&
                (tg_cl.at(TT_TIME_STAMP) != idx_not_found) &&
                ((tg_op.at(TT_TIME_STAMP)+1) < tg_cl.at(TT_TIME_STAMP))){
                const std::string& val_tok = 
                    tokens.at(tg_op.at(TT_TIME_STAMP)+1);
                rcrd->m_time_stamp = static_cast<time_t>(
                    strtoull(val_tok.c_str(), NULL, 10));
                }

            if ((tg_op.at(TT_CHECK_INDEX) != idx_not_found) &&
                (tg_cl.at(TT_CHECK_INDEX) != idx_not_found) &&
                ((tg_op.at(TT_CHECK_INDEX)+1) < tg_cl.at(TT_CHECK_INDEX))){
                const std::string& val_tok = 
                    tokens.at(tg_op.at(TT_CHECK_INDEX)+1);
                rcrd->m_check_index = static_cast<cf01_uint64>(
                    strtoull(val_tok.c_str(), NULL, 10));
                }

            if ((tg_op.at(TT_HASH) != idx_not_found) &&
                (tg_cl.at(TT_HASH) != idx_not_found) &&
                ((tg_op.at(TT_HASH)+1) < tg_cl.at(TT_HASH))){
                const std::string& val_tok = 
                    tokens.at(tg_op.at(TT_HASH)+1);
                rcrd->m_hash = cf01_hex_str_to_uint64( val_tok );
                }

            assert( call_depth < CF01_AASRT_CALL_DEPTH_COUNT );
            cf01_hc_chk_depth_rcrd *hc_chk_depth_rcrd =                
                &( m_hc_chk_depth_rcrds[call_depth] );
            rcrd->m_next = hc_chk_depth_rcrd->m_guide_rcrds;
            hc_chk_depth_rcrd->m_guide_rcrds = rcrd;
            }

        tg_op.clear();
        tg_cl.clear();
        tg_op.resize(TT_COUNT, idx_not_found);
        tg_cl.resize(TT_COUNT, idx_not_found);
        }
    }

/* reverse m_hc_chk_depth_rcrds[...].m_guide_rcrds */
cf01_depth_t cd = 0;
for( cd = 0; cd < CF01_AASRT_CALL_DEPTH_COUNT; ++cd ){
    cf01_hc_chk_depth_rcrd *hc_chk_depth_rcrd = &( m_hc_chk_depth_rcrds[cd] );
    assert( NULL == hc_chk_depth_rcrd->m_unpub_rcrds );
    reverse_hc_chk_rcrd_list( &( hc_chk_depth_rcrd->m_guide_rcrds ) );
    if( !is_hc_chk_rcrd_list_fwd_sequence(
        hc_chk_depth_rcrd->m_guide_rcrds ) ){
        /* journal corrupt */

        /* sort guide_rcrds */
        std::cout << __FILE__ << "[" << __LINE__ 
            << "] TODO: sort guide_rcrds  depth=" << cd << "\n";
        }

    /* find failed hash check in previous run.  Insert up to 31 guide records before it */
    cf01_hc_chk_rcrd *p = NULL;
    cf01_hc_chk_rcrd *n = hc_chk_depth_rcrd->m_guide_rcrds;
    while( ( NULL != n ) && ( false == n->m_mismatch_prev_run ) ){
        p = n;
        n = n->m_next;
        }

    if( ( NULL != n ) && ( n->m_mismatch_prev_run ) ){ 
        if( NULL == p ){
            /* failed on the first check */
            }
        else{
            static const cf01_uint64 max_additional_check_count_a = 31;
            cf01_uint64 additional_check_count = max_additional_check_count_a;
            if( p->m_check_index >= n->m_check_index ){
                /* error */
                }
            else{
                const cf01_uint64 max_additional_check_count_b = 
                    ( n->m_check_index - p->m_check_index ) - 1;
                if( additional_check_count > max_additional_check_count_b ){
                    additional_check_count = max_additional_check_count_b;
                    }
                }
            double check_index_step_d = ( n->m_check_index - p->m_check_index ) /
                static_cast<double>( additional_check_count + 1 );
            if( check_index_step_d < 1.0 ){ check_index_step_d = 1.0; }
            double additional_check_index_d = static_cast<double>(p->m_check_index) +
                check_index_step_d;
            cf01_hc_chk_rcrd *arp = p;
            cf01_uint64 a = 0;
            for( a = 0; ( a < additional_check_count ) &&
                ( additional_check_index_d < static_cast<double>(n->m_check_index) ); 
                ++a, additional_check_index_d += check_index_step_d ) {
                cf01_uint64 additional_check_index =
                    static_cast<cf01_uint64>(floor(additional_check_index_d));

                cf01_hc_chk_rcrd *ar = alloc_hc_chk_rcrd();
                if( NULL != ar ){
                    arp->m_next = ar;
                    ar->m_rcrd_type = CF01_HC_CHK_RCRD_TYPE_GUIDE;
                    assert( false == ar->m_mismatch_prev_run );
                    assert( 0 == ar->m_time_stamp );
                    ar->m_check_index = additional_check_index;
                    assert( 0 == ar->m_hash );
                    assert( NULL == ar->m_cmd_info );
                    assert( NULL == ar->m_next );
                    arp = ar;
                    }
                }
            arp->m_next = n;
            }
        }
    }


if(NULL == aasrt_result_fail){
    init_aa_default();
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

    cf01_call_idx_range_crit idx_range_crit = {};
    for(cf01_depth_t call_depth = 0; call_depth < CF01_AASRT_CALL_DEPTH_COUNT;
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
    complete_depth_call_idx_range_crit_table();
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

void cf01_auto_assert_wksp::init_hc_chk(){
assert( NULL == m_hc_chk_rcrd_free_chain );
size_t i;

for( i = 0; i < CF01_AASRT_CALL_DEPTH_COUNT; ++i ){
    const cf01_hc_chk_depth_rcrd *r = &( m_hc_chk_depth_rcrds[i] );
    assert( 0 == r->m_check_index );
    assert( NULL == r->m_unpub_rcrds );
    assert( NULL == r->m_guide_rcrds );
    assert( 0 == r->m_next_check_deadline );
    assert( 0 == r->m_next_check_index );
    assert( false == (r->m_first_fail_cmd_info).m_initialized );
    assert( false == (r->m_misc_cmd_info).m_initialized );
    }

for( i = 0; i < CF01_AA_HC_CHK_RCRD_POOL_SZ; ++i ){
    cf01_hc_chk_rcrd *rcrd = &(m_hc_chk_rcrd_pool[i]);
    assert( CF01_HC_CHK_RCRD_TYPE_NONE == rcrd->m_rcrd_type );
    assert( false == rcrd->m_mismatch_prev_run );
    assert( 0 == rcrd->m_time_stamp );
    assert( 0 == rcrd->m_check_index );
    assert( 0 == rcrd->m_hash );;
    assert( 0 == rcrd->m_cmd_info );
    assert( NULL == rcrd->m_next );
    rcrd->m_next = m_hc_chk_rcrd_free_chain;
    m_hc_chk_rcrd_free_chain = rcrd;
    }
}

/* add index range criteria 
*result==true => criteria successfully added */
void cf01_auto_assert_wksp::add_aasrt_call_idx_range_crit(
    const cf01_depth_t& depth,
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
void cf01_auto_assert_wksp::complete_depth_call_idx_range_crit_table()
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

void cf01_auto_assert_wksp::add_hc_chk_rcrd( const cf01_depth_t& depth, 
    const cf01_hc_chk_rcrd* hc_chk_rcrd, bool *result,
    std::string *err_str ){
assert( NULL != hc_chk_rcrd );
bool success = false;
cf01_hc_chk_rcrd *r = alloc_hc_chk_rcrd();
if( NULL == r ){
    if(NULL != err_str){
        err_str->append("failed to add hc_chk_rcrd -- memory pool exhausted");
        }
    }
else if( NULL == hc_chk_rcrd ){
    if(NULL != err_str){
        err_str->append("hc_chk_rcrd NULL");
        }
    }
else{
    const cf01_depth_t d = ( depth < CF01_AASRT_CALL_DEPTH_COUNT )  ?
        depth : ( CF01_AASRT_CALL_DEPTH_COUNT - 1 );
    cf01_hc_chk_depth_rcrd *hc_chk_depth_rcrd = &( m_hc_chk_depth_rcrds[d] );
    r->init( *hc_chk_rcrd );
    r->m_next = hc_chk_depth_rcrd->m_guide_rcrds;
    hc_chk_depth_rcrd->m_guide_rcrds = r;
    success = true;
    }

if(NULL != result){
    *result = success;
    }
}

/* increment p_count.  Call this when a standard assertion is reached */
void cf01_auto_assert_wksp::incr_p_count()
{
if(m_wksp_enabled){
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
            /* range_end==0 => unlimited range for idx_range_criteria. 
            no update */
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
} 


/* true => should run standard assertion.  This does not apply to
extra debug assertions or code. */
bool cf01_auto_assert_wksp::should_run_aasrt() const
{
bool result = false;
if( m_wksp_enabled ){
    /* get criteria */
    assert( m_call_depth < CF01_AASRT_CALL_DEPTH_COUNT);
    const cf01_uint8 idx_crit_idx = m_idx_crit_idx[m_call_depth];
    assert( idx_crit_idx < CF01_AASRT_CALL_IDX_RANGE_COUNT);
    const cf01_call_idx_range_crit *idx_range_criteria =
        &( m_depth_call_idx_range_crit[m_call_depth][idx_crit_idx] );
    
    /* Get p_count.  Should be within range of idx_range_criteria */
    const cf01_uint64 p_cnt = m_curr_p_count[m_call_depth]; 
    assert((idx_range_criteria->m_call_idx_range_end) == 0 || 
           ( p_cnt < idx_range_criteria->m_call_idx_range_end ) );
    
    /* if p_count % div == mod, then assertion should be run */
    result = ( ( idx_range_criteria->m_call_idx_div > 0) && 
        ( p_cnt % (idx_range_criteria->m_call_idx_div) ==
          ( idx_range_criteria->m_call_idx_mod ) ) ) ? true : false;
    }
return result;
}

/* true-> extra debug code and assertions should be run */
bool cf01_auto_assert_wksp::should_run_xdbg(const cf01_uint8& dbg_lvl) const
{
assert(m_call_depth < CF01_AASRT_CALL_DEPTH_COUNT); 

bool result = false;
if( m_wksp_enabled ){
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
    if(m_utility_err_buf_pos > 0){
        (aasrt_result->m_err_msg)+="\n";
        (aasrt_result->m_err_msg).insert( (aasrt_result->m_err_msg).end(),
            &(m_utility_err_buf[0]),
            &(m_utility_err_buf[m_utility_err_buf_pos]) );
        }

    if( should_save_to_file ){
        /* clear utility buffer */
        clear_utility_err_buf();
        }
    }

if((NULL != aasrt_result) && should_save_to_file){
    if( CF01_JRNL_WRITE_MODE_OFF != m_jrnl_write_mode ){
        if( !m_jrnl_write_started ){ write_journal_start(); }
        assert(m_jrnl_write_started);
        save_aasrt_result_to_file(aasrt_result);
        }
    }
}

void cf01_auto_assert_wksp::incr_hc_chk_index(){
assert( m_call_depth < CF01_AASRT_CALL_DEPTH_COUNT );
cf01_hc_chk_depth_rcrd *hc_chk_depth_rcrd =
    &( m_hc_chk_depth_rcrds[m_call_depth] );
++(hc_chk_depth_rcrd->m_check_index);
}

void cf01_auto_assert_wksp::hash_consistency_check( const cf01_uint64 hash,
    const char *file_name, const int line_num, const char *function,
    const char *hash_cmd_str){
if( m_wksp_enabled ){
    assert( m_call_depth < CF01_AASRT_CALL_DEPTH_COUNT );
    cf01_hc_chk_depth_rcrd *hc_chk_depth_rcrd =
        &( m_hc_chk_depth_rcrds[m_call_depth] );
    const cf01_hc_chk_rcrd *guide_rcrd = hc_chk_depth_rcrd->m_guide_rcrds;
    bool retire_guide_rcrd = false;
    cf01_hc_chk_rcrd *curr_run_rcrd = alloc_hc_chk_rcrd();
    const time_t now = time(NULL);
    
        //{
        //// ../cf01.cpp[1096]  hash_consistency_check  XXXX  depth=3  check_index=161  hash=1370c86748aa1d02  guide_rcrd->m_check_index=207
        //std::cout << __FILE__ << "[" << __LINE__ << "]"
        //    << "  hash_consistency_check  type=" 
        //    << ((NULL == guide_rcrd) ? 9999 : static_cast<int>( guide_rcrd->m_rcrd_type )) 
        //    << " ";
        //if( ( NULL == guide_rcrd ) ||
        //    ( guide_rcrd->m_rcrd_type != CF01_HC_CHK_RCRD_TYPE_DONE_PREV_RUN ) ){
        //    std::cout << "----  ";
        //    }
        //else if( guide_rcrd->m_check_index != hc_chk_depth_rcrd->m_check_index ){
        //    std::cout << "XXXX  ";
        //    }
        //else if( guide_rcrd->m_hash == hash ){
        //    std::cout << "PASS  ";
        //    }
        //else{
        //    std::cout << "FAIL  ";
        //    }
        //
        //std::cout << "depth=" << m_call_depth 
        //    << "  check_index=" << (hc_chk_depth_rcrd->m_check_index)
        //    << "  hash=" << cf01_uint64_to_hex_str(hash);
        //if( NULL != guide_rcrd ){
        //    std::cout << "  guide_rcrd->m_check_index=" << guide_rcrd->m_check_index;
        //    if( guide_rcrd->m_check_index == hc_chk_depth_rcrd->m_check_index ){
        //        std::cout << "  prev_hash=" << cf01_uint64_to_hex_str(guide_rcrd->m_hash);
        //        }
        //    }
        //std::cout << "\n";
        //}
    
    
    if( NULL != curr_run_rcrd ){
        /* initialize current run record */
        curr_run_rcrd->m_rcrd_type = CF01_HC_CHK_RCRD_TYPE_DONE_THIS_RUN;
        curr_run_rcrd->m_time_stamp = now;
        curr_run_rcrd->m_check_index = hc_chk_depth_rcrd->m_check_index;
        curr_run_rcrd->m_hash = hash;
    
        /* check hash mismatch */
        if( NULL == guide_rcrd ){
            /* no previous record */
            curr_run_rcrd->m_mismatch_prev_run = false;
            }
        else if( guide_rcrd->m_check_index != hc_chk_depth_rcrd->m_check_index ){
            /* previous record exists, but check index has not reached that record */
            assert( guide_rcrd->m_check_index > hc_chk_depth_rcrd->m_check_index );
            curr_run_rcrd->m_mismatch_prev_run = false;
            }
        else{
            /* check index matches */
            if( CF01_HC_CHK_RCRD_TYPE_DONE_PREV_RUN != guide_rcrd->m_rcrd_type ){
                /* no previous hash value to check because this is a guide */
                assert( 0 == guide_rcrd->m_hash );
                assert( CF01_HC_CHK_RCRD_TYPE_GUIDE == guide_rcrd->m_rcrd_type );
                curr_run_rcrd->m_mismatch_prev_run = false;
                }
            else if( guide_rcrd->m_hash == hash ){
                /* hash matches previous run */
                curr_run_rcrd->m_mismatch_prev_run = false;
                }
            else{
                /* hash does not match previous run */
                curr_run_rcrd->m_mismatch_prev_run = true;    
                }
            retire_guide_rcrd = true;
            }
    
        /* save file, line, function name, hash_cmd_str */
        if( ( curr_run_rcrd->m_mismatch_prev_run ) &&
            !( ( hc_chk_depth_rcrd->m_first_fail_cmd_info ).m_initialized ) ){
            ( hc_chk_depth_rcrd->m_first_fail_cmd_info ).init(file_name, line_num,
                function, hash_cmd_str, curr_run_rcrd->m_check_index );
            curr_run_rcrd->m_cmd_info =
                &( hc_chk_depth_rcrd->m_first_fail_cmd_info );
            }
        else if( !( ( hc_chk_depth_rcrd->m_misc_cmd_info ).m_initialized ) ){
            ( hc_chk_depth_rcrd->m_misc_cmd_info ).init(file_name, line_num,
                function, hash_cmd_str, curr_run_rcrd->m_check_index );
            curr_run_rcrd->m_cmd_info =
                &( hc_chk_depth_rcrd->m_misc_cmd_info );
            }
        else{
            assert( NULL == curr_run_rcrd->m_cmd_info );
            }
    
        /* add current record to unpublished list */
        curr_run_rcrd->m_next = hc_chk_depth_rcrd->m_unpub_rcrds;
        hc_chk_depth_rcrd->m_unpub_rcrds = curr_run_rcrd;
        }
    
    if( retire_guide_rcrd ){
        /* retire previous run record */
        assert( NULL != guide_rcrd );
        hc_chk_depth_rcrd->m_guide_rcrds = guide_rcrd->m_next;
        free_hc_chk_rcrd(const_cast<cf01_hc_chk_rcrd *>( guide_rcrd ));
        }
    
    /* deadline for running next check */
    hc_chk_depth_rcrd->m_next_check_deadline = calc_next_check_deadline(
        hc_chk_depth_rcrd->m_check_index, now );
    
    /* run next check when index reaches this value */
    cf01_uint64 next_check_index = calc_next_check_index(
        hc_chk_depth_rcrd->m_check_index );
    if( ( NULL != hc_chk_depth_rcrd->m_guide_rcrds ) &&
        ( hc_chk_depth_rcrd->m_guide_rcrds->m_check_index < next_check_index ) ){
        next_check_index = hc_chk_depth_rcrd->m_guide_rcrds->m_check_index;
        }
    hc_chk_depth_rcrd->m_next_check_index = next_check_index;
    
    /* publish records to journal */
    if( now >= m_publish_deadline ){
        publish_hc_chk_rcrds();
        }
    }
}

cf01_uint64 cf01_auto_assert_wksp::default_hash( const cf01_uint64& start,
    const cf01_uint8 *bytes, const size_t& byte_count ){
/* https://www.random.org/cgi-bin/randbyte?nbytes=2048&format=h */
static const cf01_uint64 ran_uint64[]={ 
    0x260a87ebae3a5c63ULL,  0xfb32c8f208137419ULL,  
    0xb30c6c94cf069c64ULL,  0xe483b73d1d12acffULL,  
    0x991be7638405711aULL,  0x04d49e433ab40751ULL,  
    0x71e6f07a9b954f94ULL,  0xad22229ad12c80a0ULL,  
    0xd9bcc62e2dea4f62ULL,  0xc4d7405c562a4142ULL,  
    0x96ef6484be07dc41ULL,  0x0165d8cf2c260459ULL,  
    0xe52671ac0a167355ULL,  0x9e44aed68eb39f7eULL,  
    0x72157626bc0c7415ULL,  0x066548a852c9c0f2ULL,  
    0x1c460b633ba6093aULL,  0xc062696609c0c34aULL,  
    0xdd5af4ba5b7d260fULL,  0x3111c9af3e7eaed9ULL,  
    0x319c4ad2e61a2b61ULL,  0x8140ada40f91c38bULL,  
    0x4773400fecfca84eULL,  0xf964121730c0d267ULL,  
    0x27e7d56bf03810ceULL,  0xff36436cd2b62807ULL,  
    0x633d1f47181fe2e0ULL,  0x97bcd2229a679c80ULL,  
    0xbe8911bbd05cee8aULL,  0x12bc430af7f9b108ULL,  
    0x9f1f4025ef331d3eULL,  0x23dc7333b5d81070ULL,  
    0xc3a6fba9aa80be09ULL,  0xba4f236e0713f4d0ULL,  
    0x096d2cf407a0e14cULL,  0x1d06a57b07d120aeULL,  
    0xa6d9c632b5e5f8abULL,  0x8e5bce9d94017362ULL,  
    0x576bb35541deb59cULL,  0xa044f32e4302eb43ULL,  
    0x43d9cc9c3cd7ac79ULL,  0x720bcb01dd4d466eULL,  
    0xa444a291f2f09e82ULL,  0x345dbd16fdd8574fULL,  
    0x3c91da8907966c06ULL,  0x7386824430d898caULL,  
    0xe49e23992190ac70ULL,  0x8ab3b3b28eadbe45ULL,  
    0xef0ee14221f17f5aULL,  0xb87ab88720ebcd06ULL,  
    0x6facc5bbb9d0f600ULL,  0xea12c3ea30e2e403ULL,  
    0x62f7887bde9e44a7ULL,  0xdcd200d995f0ce4eULL,  
    0x44d683feeeaca898ULL,  0x05ed1fa91a267ebdULL,  
    0x734f83b0cebe2cc4ULL,  0x7448e7a283ff3c82ULL,  
    0x6971ac579c66db8eULL,  0xb42808f83d5ef7e8ULL,  
    0x6c0e3ae43084b58dULL,  0x3ef3f66c2e616aa8ULL,  
    0x6be99aa0c8b44b5dULL,  0xa40d517c5c11af47ULL,  
    0xbd7959a05b699258ULL,  0xdb61ae7cae8abbf4ULL,  
    0x3b6619e55ba194cfULL,  0x6b973066f564cadfULL,  
    0x6a1267d6d558de01ULL,  0xdb47e4a833608b9fULL,  
    0xae72366ff3bac667ULL,  0x7f13f5b9fb979797ULL,  
    0x2b3cfdbe144c7a4eULL,  0xb6254c1f3ea1a1baULL,  
    0x4fbbb5afd1eb822fULL,  0x45ed01306dc10604ULL,  
    0xbb74a482cf6356ccULL,  0xc48762490a64e756ULL,  
    0x87765e28230bff47ULL,  0xe35f3d2b7bbc328cULL,  
    0xe6092f33136e9126ULL,  0xf6b845295adbaa0bULL,  
    0xa18cd71f3632b1bcULL,  0x186baac782ff4e5bULL,  
    0x1836b4dc1997325eULL,  0x7054096fc7ed6aedULL,  
    0x232fceecb5b6c116ULL,  0x6cccaec8e43331a9ULL,  
    0x776b42c2ea5b4165ULL,  0x223ec84d27c9293fULL,  
    0xe092b0ba320a00deULL,  0xeb33595587942b02ULL,  
    0x2f335e789a9474e4ULL,  0xa0a532a5e47c341fULL,  
    0xf24498a02726fcbeULL,  0xf6c6ebe6f298f9a0ULL,  
    0x8d16536f682176d9ULL,  0x6200f80b553c27f5ULL,  
    0xc645f3b032c8ba23ULL,  0x35b027aa5458abfcULL,  
    0xbdaa1e4513a44725ULL,  0x80ea06643fe4fc0bULL,  
    0xa58db2d31be7cdd7ULL,  0x0454855a8b08444dULL,  
    0x5c8a14b9dd5dcc06ULL,  0xe8de0304ead9cae6ULL,  
    0xbeb1e3ab4b4c7275ULL,  0x347fad83111dabeeULL,  
    0xe4cfe9d64dcfea06ULL,  0x4f68bcdbc6e5f09cULL,  
    0x10a6b243ba306f62ULL,  0x7f5576849b45766bULL,  
    0x458ff748d78e989aULL,  0xebb2f188f7a51144ULL,  
    0x56364a5169abb24eULL,  0x53586a0e45394f39ULL,  
    0x63eb0b0ac236d3b5ULL,  0x93eb2cd428978462ULL,  
    0xde617335233621bfULL,  0xd582d1c3f1690365ULL,  
    0x2946ca622976a5afULL,  0x413c87615b8856dbULL,  
    0x1eee8fcdaa7a429aULL,  0x18c4d89437afa87cULL,  
    0xcfdd4d9cce5c2e6cULL,  0x435da981eb3aa5c2ULL,  
    0x4b31f40d4a3fdccfULL,  0x51230a2ac16f6e3dULL,  
    0x539747d61078db49ULL,  0x0dfffa0e53d796b9ULL,
    0xfff910579cb64588ULL,  0x924368674b8b9199ULL,
    0xd4aad9c0909c0299ULL,  0x6095381332025253ULL,
    0xe691af4550074350ULL,  0x9781499432ebe74dULL,
    0x8f110ccacb61be06ULL,  0xf6483d3c0f09856fULL,
    0x4b718df7fd2da67bULL,  0x86665a171ed42620ULL,
    0xc2dbd28e8c68f832ULL,  0xecd3006ff16b3dd0ULL,
    0x09c1e113012865c6ULL,  0xbbbe80e6457d41dfULL,
    0x13d93736cc023cacULL,  0x3cb2f0d6f1d5d07dULL,
    0x9a1362d6d2330f9cULL,  0xc2adf89ea4513ce3ULL,
    0x02b9d5b24451edd9ULL,  0xad54d4de3475f439ULL,
    0xc1895b700c9fa5ddULL,  0x6ae52f2cc0d7c741ULL,
    0x5e257cfbcae899f1ULL,  0x34d142b0e89a21d7ULL,
    0x7c70a5d87aca0eb9ULL,  0xcaef402b24cb7257ULL,
    0xa121135775acdc5eULL,  0x0e1ec8ab03626475ULL,
    0xed02d5a1795d6a2bULL,  0x4985d82140b94485ULL,
    0x25ce51354556aa3bULL,  0x462f903e26a46242ULL,
    0x00ec7371741aa931ULL,  0x328436db28298289ULL,
    0x5e4b31880e20621dULL,  0xb48a383e5c567f01ULL,
    0x977a920e6706829fULL,  0x635651cf2ac8f86fULL,
    0xd355847069d80eceULL,  0x38f0913114708f85ULL,
    0x420b946258cf1d95ULL,  0x38b7ae08486aa935ULL,
    0xa5357702d63a6147ULL,  0x2758e48b82e7f0b1ULL,
    0x9fe3cca786265cd2ULL,  0xe197fc9286f0c66dULL,
    0x5357c97f26dee3d6ULL,  0x48885dfe2e4d7a96ULL,
    0x4ceed1e14de1a9aaULL,  0x3e7132a0c1b7adfeULL,
    0x546f827fe76737c6ULL,  0xe42dcc2ee3961bcbULL,
    0x42a0275101198229ULL,  0x883eb4732d6db285ULL,
    0x657ecc9c92738a02ULL,  0xfcd3434dff750e97ULL,
    0x03f97de8384ea0b1ULL,  0x3e5e144afbc3827eULL,
    0x49aa88e96b7127b2ULL,  0xae23693c9565efd3ULL,
    0xb7e4aaaed4709c7fULL,  0xaaec92c9edec2444ULL,
    0x8e461a9fb2010d73ULL,  0x782fd146341fd664ULL,
    0x6538eb123784930fULL,  0x7888466779228777ULL,
    0xc4a822dd43212f24ULL,  0xd7ecf251c78c3a19ULL,
    0xf5b1d6d9117f1114ULL,  0xca9603f4032878bfULL,
    0x02b7ab276f0ea1eaULL,  0x04c5d0c7e4ab196bULL,
    0x61234c0f79c96f5aULL,  0x96bc48b690ae21d0ULL,
    0x6ccf6c34eb0ec961ULL,  0x054e072312a3fdc9ULL,
    0xf0299d886f6537fcULL,  0x4381e7ca284201c9ULL,
    0x1d707df00a59c3d8ULL,  0x08616a1281ebe1a7ULL,
    0xe7d64bbfc244472cULL,  0x0ced2542238d4776ULL,
    0xf5e2a9cca486cb25ULL,  0xb79b18a569fdd8f6ULL,
    0xd55ff9a887c2702dULL,  0x18a78d3888577eceULL,
    0x02e1a51970af8db4ULL,  0x41a5c11cfe31ca5eULL,
    0xb82d9310991db708ULL,  0x5a7917ad20179650ULL,
    0x93f08dc9dc61761eULL,  0x54e07169ef3795d2ULL,
    0xc504c5ad0a0f17b6ULL,  0xce7530c52577a545ULL,
    0xf9d0662a0bf76f7dULL,  0x80985ddcb5311f2cULL,
    0xc494467e2fbc1ba0ULL,  0x147778d97d88b322ULL,
    0x40120f06df94a7e0ULL,  0x1f537a9d80b7334dULL,
    0x60325a2688dbb174ULL,  0xc393e9306c08f7f7ULL,
    0x0b567cb3f919c0dcULL,  0x4de7cd7b6b427251ULL,
    0xba3b3bc4ef3d7ea2ULL,  0xcb97107d64888368ULL,
    0x36e8519b7fc16fc1ULL,  0xbbe310f86e3792b3ULL,
    0x1e76a64de1fd47d5ULL,  0x379c41e096078e83ULL,
    0xe3e849de58fb0c03ULL,  0x193b6ba7696bbdedULL,
    0xdd8b94affcc8c77eULL,  0xa61aacecf1edcba1ULL,
    0x0c357859fe9dca60ULL,  0x9d90fa40cafca0d1ULL,
    0xafac9e4886c2ada1ULL,  0xf0e66228678bf646ULL,
    0x7c7aec4ecebde7f9ULL,  0x94469c05b7bc1009ULL,
    0x912c7c425f388cbfULL,  0x8733f62407f1a992ULL,
    0xd3770948f55eb4bcULL,  0xddde9a714567b9c8ULL,
    0x14c1fb5e6a5a09a7ULL,  0x9597edd764f7ccc6ULL 
    };
static const size_t ran_count = sizeof( ran_uint64 ) / sizeof( ran_uint64[0] );

assert( NULL != bytes );
assert( 256 == ran_count );
cf01_uint64 h = start;
const cf01_uint8 * const b_end = &(bytes[byte_count]);
for( const cf01_uint8 *b = &(bytes[0]); b != b_end; ++b ){
    const cf01_uint8& bi = *b;
    const cf01_uint64& r = ran_uint64[bi];
    h ^= r;

    /* https://en.wikipedia.org/wiki/Jenkins_hash_function#one_at_a_time */
    h += (h << 10);
    h ^= (h >> 6);
    }
h += (h << 3);
h ^= (h >> 11);
h += (h << 15);
return h;
}

/* debug dump */
std::ostream& cf01_auto_assert_wksp::ostream_output(std::ostream& os) const
{
cf01_depth_t call_depth;
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


#if 0
    /* for each call depth d, the index i for m_depth_call_idx_range_crit[d][i]
    specifying the current control criteria that should be used */
    cf01_uint8 m_idx_crit_idx[CF01_AASRT_CALL_DEPTH_COUNT]; 



    /* B: extra debug assertions and supporting code 

    These are assertions and supporting code that are only executed if
    the debug level is high enough according to the following if statement:

    if( m_curr_t_count[m_call_depth] >= m_xdbg_ctrl_t_count[m_call_depth]
        && dbg_lvl <= m_xdbg_ctrl_dbg_lvl[m_call_depth] )
        {
        should execute extra debug assertion or code.
        } 
    */

    /* controls when extra debug code and assertions will be executed.
    This is the minimum total transition count at each call depth.
    If total transition count is below this value, then no extra debug
    code or assertions are run.  If total transition count is at or above
    this value, then extra debug code with debug level at or below
     m_xdbg_ctrl_dbg_lvl[d] is run. */
    cf01_uint64 m_xdbg_ctrl_t_count[CF01_AASRT_CALL_DEPTH_COUNT];

    /* controls when extra debug code and assertions will be executed.
    Extra debug code and assertions with debug level at or below
    m_xdbg_ctrl_dbg_lvl[d] will be executed when transition count is at or 
    above m_xdbg_ctrl_t_count[d] */
    cf01_uint8 m_xdbg_ctrl_dbg_lvl[CF01_AASRT_CALL_DEPTH_COUNT]; 

    /* info for last assertion that was run and passed */
    cf01_aasrt_result m_aasrt_result_last_passed;

    /* info for first assertion that failed */
    cf01_aasrt_result m_aasrt_result_first_failed;

    /* utility buffer */
    #define CF01_AA_UTILITY_ERR_BUF_CAPACITY (2047)
    char m_utility_err_buf[CF01_AA_UTILITY_ERR_BUF_CAPACITY];
    size_t m_utility_err_buf_pos;
#endif

/* hash consistency check */
os << "<start_time>" << m_start_time << "</start_time>\n";
os << "<start_time_ctime>" << cf01_ctime(m_start_time)
    << "</start_time_ctime>\n";

os << "<publish_deadline>" << m_publish_deadline << "</publish_deadline>\n";
os << "<publish_deadline_ctime>" << cf01_ctime(m_publish_deadline)
    << "</publish_deadline_ctime>\n";

os << "<hc_chk_depth_rcrds>\n";

for(call_depth = 0; call_depth<CF01_AASRT_CALL_DEPTH_COUNT; ++call_depth){
    const cf01_hc_chk_depth_rcrd *r =
        &(m_hc_chk_depth_rcrds[call_depth]);

    os << "  <hc_chk_depth_rcrd>\n";
    os << "    <call_depth>" << call_depth << "</call_depth>\n";
    os << "    <check_index>" << (r->m_check_index) << "</check_index>\n";

    if( NULL == r->m_unpub_rcrds ){
        os << "    <unpub_rcrds>NULL</unpub_rcrds>\n";
        }
    else{
        os << "    <unpub_rcrds>\n";
        const cf01_hc_chk_rcrd *rr = r->m_unpub_rcrds;
        i = 0;
        while( ( NULL != rr ) && ( i < CF01_AA_HC_CHK_RCRD_POOL_SZ ) ){
            rr->ostream_output( os, "      " );
            rr = rr->m_next;
            ++i;
            }
        os << "    </unpub_rcrds>\n";
        }

    if( NULL == r->m_guide_rcrds ){
        os << "    <guide_rcrds>NULL</guide_rcrds>\n";
        }
    else{
        os << "    <guide_rcrds>\n";
        const cf01_hc_chk_rcrd *rr = r->m_guide_rcrds;
        i = 0;
        while( ( NULL != rr ) && ( i < CF01_AA_HC_CHK_RCRD_POOL_SZ ) ){
            rr->ostream_output( os, "      " );
            rr = rr->m_next;
            ++i;
            }
        os << "    </guide_rcrds>\n";
        }

#if 0

    /* deadline for running next check */
    time_t m_next_check_deadline;

    /* run next check when index reaches this value */
    cf01_uint64 m_next_check_index;

    /* record for first hash consistency check failure */
    cf01_hc_chk_cmd_info m_first_fail_cmd_info;


    /* record for debugging infinite loops */
    cf01_hc_chk_cmd_info m_misc_cmd_info;

#endif

    os << "  </hc_chk_depth_rcrd>\n";

    }
os << "</hc_chk_depth_rcrds>\n";

#if 0
    cf01_hc_chk_rcrd m_hc_chk_rcrd_pool[CF01_AA_HC_CHK_RCRD_POOL_SZ];
    cf01_hc_chk_rcrd *m_hc_chk_rcrd_free_chain;
#endif

os << "</cf01_auto_assert_wksp>\n";
return os;
}


int cf01_auto_assert_wksp::verify_data( std::string *err_str ) const{
int err_cnt = 0;
size_t call_depth, i, j;
const cf01_call_idx_range_crit *c, *c_prev;

const time_t now = time(NULL);

/* m_call_depth */

/* m_curr_p_count */

/* m_curr_t_count */
for( call_depth = 2; call_depth < CF01_AASRT_CALL_DEPTH_COUNT; ++call_depth){
    if((m_curr_p_count[call_depth-1]==0) &&
       (m_curr_p_count[call_depth]>0)){
       ++err_cnt;
       if( NULL != err_str ){
           err_str->append("m_curr_p_count error\n");
           }
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
            if( NULL != err_str ){
                err_str->append("mod>=div\n");
                }
            }
        if(NULL != c_prev){
            if( (c_prev->m_call_idx_range_end == 0) && 
                (c->m_call_idx_range_end > 0 ) ){
                ++err_cnt;
                if( NULL != err_str ){
                    err_str->append("prev_range_end==0,range_end>0\n");
                    }
                }
            if( (c_prev->m_call_idx_range_end > 0) && 
                (c->m_call_idx_range_end > 0 ) && 
                (c_prev->m_call_idx_range_end >= c->m_call_idx_range_end ) ){
                ++err_cnt;
                if( NULL != err_str ){
                    err_str->append("prev_range_end>=range_end\n");
                    }
                }
            }
        }
    }


/* time_t m_start_time */
/* time_t m_publish_deadline */
if( m_publish_deadline < m_start_time ){
    ++err_cnt;
    if( NULL != err_str ){
        err_str->append("publish_deadline < start_time\n");
        }
    }

/* cf01_hc_chk_depth_rcrd m_hc_chk_depth_rcrds[CF01_AASRT_CALL_DEPTH_COUNT] */
cf01_depth_t calld = 0;
for( calld = 0; calld < CF01_AASRT_CALL_DEPTH_COUNT; ++calld ){
    const cf01_hc_chk_depth_rcrd *hc_chk_depth_rcrd = 
        &(m_hc_chk_depth_rcrds[calld]);

    /* list of unpublished records of checks done this run */
    const cf01_hc_chk_rcrd *prev_rcrd = NULL;
    const cf01_hc_chk_rcrd *unpub_r = hc_chk_depth_rcrd->m_unpub_rcrds;
    j = 0;
    while(( NULL != unpub_r ) && ( j < CF01_AA_HC_CHK_RCRD_POOL_SZ )){
        if( CF01_HC_CHK_RCRD_TYPE_DONE_THIS_RUN != unpub_r->m_rcrd_type ){
            ++err_cnt;
            if( NULL != err_str ){
                err_str->append("CF01_HC_CHK_RCRD_TYPE_DONE_THIS_RUN"
                    " != unpub_r->m_rcrd_type\n");
                }
            }

        /* m_mismatch_prev_run */

        /* time stamp */
        if( now < (unpub_r->m_time_stamp) ){
            ++err_cnt;
            if( NULL != err_str ){
                err_str->append("now < (unpub_r->m_time_stamp)\n");
                }
            }
        if( ((unpub_r->m_time_stamp) + 
             static_cast<time_t>( 365 * 24 * 60 * 60 ) ) < now ){
            ++err_cnt;
            if( NULL != err_str ){
                err_str->append("(unpub_r->m_time_stamp) too long ago\n");
                }
            }

        /* records should be in reverse sequence */
        if( NULL != prev_rcrd ){
            if( prev_rcrd->m_check_index <= unpub_r->m_check_index ){
                ++err_cnt;
                if( NULL != err_str ){
                    char err_buf[256];
                    sprintf( err_buf, "calld=%lli "
                        "prev_rcrd->m_check_index=%llu <="
                        " unpub_r->m_check_index=%llu\n", calld, 
                        prev_rcrd->m_check_index, unpub_r->m_check_index );
                    err_str->append( err_buf );
                    }
                }

            if( prev_rcrd->m_time_stamp < unpub_r->m_time_stamp ){
                ++err_cnt;
                if( NULL != err_str ){
                    char err_buf[256];
                    sprintf( err_buf, "calld=%lli " 
                        " prev_rcrd->m_time_stamp=%llu <"
                        " unpub_r->m_time_stamp=%llu\n", calld,
                        prev_rcrd->m_time_stamp, unpub_r->m_time_stamp );
                    err_str->append( err_buf );
                    }
                }            
            }

        /* m_hash */

        /* m_cmd_info */
        if( NULL != unpub_r->m_cmd_info ){
            if( ( unpub_r->m_cmd_info != &( hc_chk_depth_rcrd->m_first_fail_cmd_info ) ) &&
                ( unpub_r->m_cmd_info != &( hc_chk_depth_rcrd->m_misc_cmd_info ) ) ){
                ++err_cnt;
                if( NULL != err_str ){
                    err_str->append("unpub_r->m_cmd_info !="
                        " &( hc_chk_depth_rcrd->m_first_fail_cmd_info ) && "
                        "unpub_r->m_cmd_info !="
                        " &( hc_chk_depth_rcrd->m_misc_cmd_info )\n"
                        );
                    }
                }
            if( false == unpub_r->m_mismatch_prev_run ){
                ++err_cnt;
                if( NULL != err_str ){
                    err_str->append("NULL != unpub_r->m_cmd_info &&"
                        " false == unpub_r->m_mismatch_prev_run\n");
                    }
                }
            if( false == unpub_r->m_cmd_info->m_initialized ){
                ++err_cnt;
                if( NULL != err_str ){
                    err_str->append("false == unpub_r->m_cmd_info->m_initialized\n");
                    }
                }
            if( unpub_r->m_cmd_info->m_check_index != unpub_r->m_check_index ){
                ++err_cnt;
                if( NULL != err_str ){
                    err_str->append("unpub_r->m_cmd_info->m_check_index "
                        "!= unpub_r->m_check_index\n");
                    }
                }
            }

        ++j;
        prev_rcrd = unpub_r;
        unpub_r = unpub_r->m_next;
        }
     if( ( NULL != unpub_r ) && ( j >= CF01_AA_HC_CHK_RCRD_POOL_SZ ) ){
        ++err_cnt;
        if( NULL != err_str ){
            err_str->append("hc_chk_depth_rcrd->m_unpub_rcrds list too long\n");
            }
        }

    /* list of records of checks done previously.  This list is consumed
      as checks are done such that the start of the list is relevant for determining
      which check to run next. */
    prev_rcrd = NULL;
    const cf01_hc_chk_rcrd *guide_r = hc_chk_depth_rcrd->m_guide_rcrds;
    j = 0;
    while(( NULL != guide_r ) && ( j < CF01_AA_HC_CHK_RCRD_POOL_SZ )){
        if( ( CF01_HC_CHK_RCRD_TYPE_GUIDE != guide_r->m_rcrd_type ) && 
            ( CF01_HC_CHK_RCRD_TYPE_DONE_PREV_RUN != guide_r->m_rcrd_type )){
            ++err_cnt;
            if( NULL != err_str ){
                err_str->append("CF01_HC_CHK_RCRD_TYPE_GUIDE"
                    " != guide_r->m_rcrd_type &&"
                    " CF01_HC_CHK_RCRD_TYPE_DONE_PREV_RUN`"
                    " != guide_r->m_rcrd_type\n");
                }
            }

        /* records should be in forward sequence */
        if( NULL != prev_rcrd ){
            if( ( CF01_HC_CHK_RCRD_TYPE_DONE_PREV_RUN == 
                  guide_r->m_rcrd_type ) &&
                ( prev_rcrd->m_check_index >= guide_r->m_check_index ) ){
                ++err_cnt;
                if( NULL != err_str ){
                    char err_buf[256];
                    sprintf( err_buf, "calld=%lli "
                        "prev_rcrd->m_check_index=%llu >="
                        " guide_r->m_check_index=%llu\n", calld, 
                        prev_rcrd->m_check_index, guide_r->m_check_index );
                    err_str->append( err_buf );
                    }
                }

            if( ( CF01_HC_CHK_RCRD_TYPE_DONE_PREV_RUN == 
                  guide_r->m_rcrd_type ) &&
                ( prev_rcrd->m_time_stamp > guide_r->m_time_stamp ) ){
                ++err_cnt;
                if( NULL != err_str ){
                    char err_buf[256];
                    sprintf( err_buf, "calld=%lli " 
                        " prev_rcrd->m_time_stamp=%llu >"
                        " (check_index=%llu) >"
                        " guide_r->m_time_stamp=%llu (check_index=%llu)\n",
                        calld,
                        prev_rcrd->m_time_stamp, prev_rcrd->m_check_index,
                        guide_r->m_time_stamp, guide_r->m_check_index );
                    err_str->append( err_buf );
                    }
                }            
            }

        ++j;
        prev_rcrd = guide_r;
        guide_r = guide_r->m_next;
        }
     if( ( NULL != guide_r ) && ( j >= CF01_AA_HC_CHK_RCRD_POOL_SZ ) ){
        ++err_cnt;
        if( NULL != err_str ){
            err_str->append("hc_chk_depth_rcrd->m_guide_rcrds list too long\n");
            }
        }
    }

/* #define CF01_AA_HC_CHK_RCRD_POOL_SZ (2047) */
/* cf01_hc_chk_rcrd m_hc_chk_rcrd_pool[CF01_AA_HC_CHK_RCRD_POOL_SZ] */
/* cf01_hc_chk_rcrd *m_hc_chk_rcrd_free_chain */


return err_cnt;
}


void cf01_auto_assert_wksp::write_journal_start(){
static const std::string start_tag = "jrnl_start";
write_journal_start_end(start_tag);
m_jrnl_write_started = true;
}

void cf01_auto_assert_wksp::write_journal_end(){
static const std::string end_tag = "jrnl_end";
write_journal_start_end(end_tag);
}

void cf01_auto_assert_wksp::write_journal_start_end(
    const std::string& start_end_tag){
std::ofstream os;
os.open( m_jrnl_file_name, std::ofstream::out | std::ofstream::app );
if(!os.is_open()){
    std::cout << "failed to open " << m_jrnl_file_name << std::endl; }
else{
    time_t now = time(NULL);
    std::string now_str(ctime(&now));
    while((now_str.length() > 0 )&& isspace(now_str.at(now_str.length()-1))){
        now_str.resize(now_str.length()-1); }
    os << "\n<" << start_end_tag << ">\n";
    os << "  <time_t>" << static_cast<unsigned long>(now) << "</time_t>\n";
    os << "  <clock_t>" << static_cast<unsigned long>(clock()) << "</clock_t>\n";
    os << "  <ctime>" << now_str << "</ctime>\n";
    os << "</" << start_end_tag << ">\n";
    os.close();
    }
}

void cf01_auto_assert_wksp::save_aasrt_result_to_file(
    const cf01_aasrt_result *r){
cf01_uint16 i, end_idx;
assert(NULL != r);
std::ofstream os;
os.open( m_jrnl_file_name, std::ofstream::out | std::ofstream::app );
if(!os.is_open()){
    std::cout << "failed to open " << m_jrnl_file_name << std::endl; }
else{
    time_t now = time(NULL);
    os << "\n<aasrt_result>\n";
    os << "  <time_t>" << static_cast<unsigned long>(now) << "</time_t>\n";
    os << "  <clock_t>" << static_cast<unsigned long>(clock()) << "</clock_t>\n";
    os << "  <ctime>" << cf01_ctime(now) << "</ctime>\n";
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
    os << "</aasrt_result>\n";
    os.close();
    }
}

cf01_hc_chk_rcrd *cf01_auto_assert_wksp::alloc_hc_chk_rcrd(){
cf01_hc_chk_rcrd *rcrd = m_hc_chk_rcrd_free_chain;
if( NULL != rcrd ){
    m_hc_chk_rcrd_free_chain = rcrd->m_next;
    rcrd->m_next = NULL;

    assert( CF01_HC_CHK_RCRD_TYPE_NONE == rcrd->m_rcrd_type );
    assert( false == rcrd->m_mismatch_prev_run );
    assert( 0 == rcrd->m_time_stamp );
    assert( 0 == rcrd->m_check_index );
    assert( 0 == rcrd->m_hash );
    assert( NULL == rcrd->m_cmd_info );
    assert( NULL == rcrd->m_next );
    }
return rcrd;
}

void cf01_auto_assert_wksp::free_hc_chk_rcrd(cf01_hc_chk_rcrd *rcrd){
assert(NULL != rcrd);
rcrd->reset();
rcrd->m_next = m_hc_chk_rcrd_free_chain;
m_hc_chk_rcrd_free_chain = rcrd;
}

void cf01_auto_assert_wksp::free_hc_chk_rcrd_chain(cf01_hc_chk_rcrd *rcrd){
cf01_hc_chk_rcrd *r = rcrd;
while(NULL != r){
    cf01_hc_chk_rcrd *n = r->m_next;
    r->reset();
    r->m_next = m_hc_chk_rcrd_free_chain;
    m_hc_chk_rcrd_free_chain = r;
    r = n;
    }
}

/*
   p     +---+    +---+    +---+    +---+    +---+    
 NULL    |*s |--->| n |--->|   |--->|   |--->|   |--->NULL
         +---+    +---+    +---+    +---+    +---+    

         +---+    +---+    +---+    +---+    +---+    
 NULL<---| p |    |*s |--->| n |--->|   |--->|   |--->NULL
         +---+    +---+    +---+    +---+    +---+    

         +---+    +---+    +---+    +---+    +---+    
 NULL<---|   |<---| p |    |*s |--->| n |--->|   |--->NULL
         +---+    +---+    +---+    +---+    +---+    

         +---+    +---+    +---+    +---+    +---+    
 NULL<---|   |<---|   |<---| p |    |*s |--->| n |--->NULL
         +---+    +---+    +---+    +---+    +---+    

         +---+    +---+    +---+    +---+    +---+     n
 NULL<---|   |<---|   |<---|   |<---| p |    |*s |--->NULL
         +---+    +---+    +---+    +---+    +---+    

         +---+    +---+    +---+    +---+    +---+     n
 NULL<---|   |<---|   |<---|   |<---| p |<---|*s |    NULL
         +---+    +---+    +---+    +---+    +---+
*/
void cf01_auto_assert_wksp::reverse_hc_chk_rcrd_list( cf01_hc_chk_rcrd **s ){
assert( NULL != s );
if( NULL != *s ){
    cf01_hc_chk_rcrd *p = NULL;
    cf01_hc_chk_rcrd *n = (*s)->m_next;
    while( NULL != n ){
        (*s)->m_next = p;
        p = (*s);
        (*s) = n;
        n = (*s)->m_next;
        }
    (*s)->m_next = p;
    }
}

/* true => record list starting from r has time stamps and
check index values in ascending order */
bool cf01_auto_assert_wksp::is_hc_chk_rcrd_list_fwd_sequence(
    const cf01_hc_chk_rcrd *r ){
bool fwd_seq = true;
const cf01_hc_chk_rcrd *rcrd = r;
size_t rcrd_count = 0;
while( ( rcrd_count <= CF01_AA_HC_CHK_RCRD_POOL_SZ ) && 
    ( NULL != rcrd ) && fwd_seq ){
    const cf01_hc_chk_rcrd *next_rcrd = rcrd->m_next;
    if( NULL != next_rcrd ){
        if( ( rcrd->m_check_index >= next_rcrd->m_check_index ) ||
            ( rcrd->m_time_stamp > next_rcrd->m_time_stamp ) ){
            fwd_seq = false;
            }
        }
    rcrd = next_rcrd;
    ++rcrd_count;
    }
assert( rcrd_count <= CF01_AA_HC_CHK_RCRD_POOL_SZ );
return fwd_seq;
}

/* true => record list starting from r has time stamps and
check index values in descending order */
bool cf01_auto_assert_wksp::is_hc_chk_rcrd_list_rev_sequence(
    const cf01_hc_chk_rcrd *r ){
bool rev_seq = true;
const cf01_hc_chk_rcrd *rcrd = r;
size_t rcrd_count = 0;
while( ( rcrd_count <= CF01_AA_HC_CHK_RCRD_POOL_SZ ) && 
    ( NULL != rcrd ) && rev_seq ){
    const cf01_hc_chk_rcrd *next_rcrd = rcrd->m_next;
    if( NULL != next_rcrd ){
        if( ( rcrd->m_check_index <= next_rcrd->m_check_index ) ||
            ( rcrd->m_time_stamp < next_rcrd->m_time_stamp ) ){
            rev_seq = false;
            }
        }
    rcrd = next_rcrd;
    ++rcrd_count;
    }
assert( rcrd_count <= CF01_AA_HC_CHK_RCRD_POOL_SZ );
return rev_seq;
}

void cf01_auto_assert_wksp::clear_hc_chk_depth_rcrds(){
size_t i;
for (i = 0; i < CF01_AASRT_CALL_DEPTH_COUNT; ++i) {
    cf01_hc_chk_depth_rcrd *dr = &( m_hc_chk_depth_rcrds[i] );
    if( NULL != dr->m_unpub_rcrds ){
        free_hc_chk_rcrd_chain( dr->m_unpub_rcrds );
        }
    if( NULL != dr->m_guide_rcrds ){
        free_hc_chk_rcrd_chain( dr->m_guide_rcrds );
        }
    }
memset(m_hc_chk_depth_rcrds, 0, sizeof(m_hc_chk_depth_rcrds));
for (i = 0; i < CF01_AASRT_CALL_DEPTH_COUNT; ++i) {
    cf01_hc_chk_depth_rcrd *dr = &( m_hc_chk_depth_rcrds[i] );
    assert( NULL == dr->m_unpub_rcrds );
    assert( NULL == dr->m_guide_rcrds );
    }
}

/* TODO: use pseudo-random number to promote variety in code locations checked. */
cf01_uint64 cf01_auto_assert_wksp::calc_next_check_index(
    const cf01_uint64& curr_check_index ){
static const int n_low = 3;
static const int n_high = 16;
static const cf01_uint64 i_low = 2 << n_low;
static const cf01_uint64 i_high = 2 << n_high;
static const cf01_uint64 i_max = std::numeric_limits<cf01_uint64>::max();

cf01_uint64 next_check_index = i_low;
if( curr_check_index >= i_low ){
    int n = 0;
    #if(( __cplusplus > 199711L ) || ( defined(_MSVC_LANG) && (_MSVC_LANG > 199711L ) )) 
    std::ignore =
    #endif
    frexp( static_cast<double>(curr_check_index), &n );
    assert( n > n_low );
    if( n <= n_high ){
        next_check_index = static_cast<cf01_uint64>(2) << n;
        }
    else{
        next_check_index = ( ( curr_check_index / i_high ) + 1 ) * i_high;
        if( 0 == next_check_index ){
            next_check_index = i_max;
            }
        }
    }

assert( ( curr_check_index < next_check_index ) || ( i_max == next_check_index ) );
return next_check_index;
}


time_t cf01_auto_assert_wksp::calc_next_check_deadline(
    const cf01_uint64& curr_check_index, const time_t& now ){
struct i_t{ cf01_uint64 m_i; time_t m_t; };
static const i_t i_t_array[] = {
    {   1,            1 },
    {   4,            2 },
    {   8,            4 },
    {  16,            8 },
    {  32,           16 },
    { 128,           32 },
    { 512,           64 },
    { 2048,         128 },
    };
static const size_t i_t_count = sizeof(i_t_array)/sizeof(i_t_array[0]);

static const time_t t_delta_max = 256;
size_t j = 0;
time_t t_delta = t_delta_max;
bool done = false;
while (!done) {
    if( j >= i_t_count ){
        done = true;
        }
    else{
        const i_t& it = i_t_array[j];
        if( curr_check_index <= it.m_i ){
            t_delta = it.m_t;
            done = true;
            }
        }
    ++j;
    }

assert( t_delta > 0 );
const time_t deadline = now + t_delta;
return deadline;
}

void cf01_auto_assert_wksp::publish_hc_chk_rcrds(){
/* publish records */
cf01_depth_t depth;
for( depth = 0; depth < CF01_AASRT_CALL_DEPTH_COUNT; ++depth ) {
    cf01_hc_chk_depth_rcrd *dr = &( m_hc_chk_depth_rcrds[depth] );
    if( NULL != dr->m_unpub_rcrds ){
        assert(is_hc_chk_rcrd_list_rev_sequence( dr->m_unpub_rcrds ) );
        reverse_hc_chk_rcrd_list( &(dr->m_unpub_rcrds) );
        const cf01_hc_chk_rcrd *r = dr->m_unpub_rcrds;
        assert(is_hc_chk_rcrd_list_fwd_sequence( r ) );
        while( NULL != r ){
            publish_hc_chk_rcrd( depth, r );
            r = r->m_next;
            }
        free_hc_chk_rcrd_chain(dr->m_unpub_rcrds);
        dr->m_unpub_rcrds = NULL;
        }
    ( dr->m_misc_cmd_info ).reset();
    }

/* reset deadline */
const time_t now = time(NULL);
static const time_t publish_deadline_delta = 2;
m_publish_deadline = now + publish_deadline_delta;
}

void cf01_auto_assert_wksp::publish_hc_chk_rcrd(
    const cf01_depth_t& depth, const cf01_hc_chk_rcrd *r){
if( CF01_JRNL_WRITE_MODE_ON == m_jrnl_write_mode ){
    if( !m_jrnl_write_started ){
        write_journal_start();
        }

    cf01_uint16 i=0, end_idx=0;
    assert(NULL != r);
    std::ofstream os;
    os.open( m_jrnl_file_name, std::ofstream::out | std::ofstream::app );
    if(!os.is_open()){
        std::cout << "failed to open " << m_jrnl_file_name << std::endl; }
    else{
        std::string time_stamp_ctime_str(ctime(&  (r->m_time_stamp)  ));
        while((time_stamp_ctime_str.length() > 0 ) &&
            isspace(time_stamp_ctime_str.at(time_stamp_ctime_str.length()-1))){
            time_stamp_ctime_str.resize(time_stamp_ctime_str.length()-1); 
            }
        os << "\n<hc_chk_rcrd>\n";
        os << "  <call_depth>" << static_cast<unsigned int>(depth)
            << "</call_depth>\n";
        /* m_rcrd_type always = CF01_HC_CHK_RCRD_TYPE_DONE_PREV_RUN */
        os << "  <mismatch_prev_run>" << ((r->m_mismatch_prev_run)?"true":"false")
            << "</mismatch_prev_run>\n";
        os << "  <time_stamp>" << static_cast<unsigned long>(r->m_time_stamp)
            << "</time_stamp>\n";
        os << "  <time_stamp_ctime>" << time_stamp_ctime_str
            << "</time_stamp_ctime>\n";
        os << "  <check_index>" << (r->m_check_index) << "</check_index>\n";
        os << "  <hash>" << cf01_uint64_to_hex_str(r->m_hash) << "</hash>\n";
        const cf01_hc_chk_cmd_info * const cmd_info = r->m_cmd_info;
        if( NULL != cmd_info ){
            assert( cmd_info->m_initialized );
            assert( cmd_info->m_check_index == r->m_check_index );
            os << "  <file_name>" << (cmd_info->m_file_name) << "</file_name>\n";
            os << "  <func_name>" << (cmd_info->m_func_name) << "</func_name>\n";
            os << "  <line_num>" << (cmd_info->m_line_num) << "</line_num>\n";
            os << "  <hash_cmd>" << (cmd_info->m_hash_cmd_str) << "</hash_cmd>\n";
            }
        os << "</hc_chk_rcrd>\n";
        os.close();
        }
    }
}

std::string cf01_ctime( const time_t& t ){
std::string t_str(ctime(&t));
while((t_str.length() > 0 )&& isspace(t_str.at(t_str.length()-1))){
    t_str.resize(t_str.length()-1); }
return t_str;
}

std::string cf01_uint64_to_hex_str( const cf01_uint64& u ){
static const std::string::size_type nybble_count = 2 * sizeof(cf01_uint64);
std::string s;
s.reserve( nybble_count );
std::string::size_type i = nybble_count;
while( i > 0 ){
    --i;
    const cf01_uint8 nybble = static_cast<cf01_uint8>( u >> ( 4 * i ) ) &
        static_cast<cf01_uint8>( 0x0f );
    char ch = '0';
    if( nybble <= 9 ){
        ch = '0' + static_cast<char>( nybble );
        }
    else{
        ch = ( 'a' - static_cast<char>( 10 ) ) + static_cast<char>( nybble );
        }
    s.append(1, ch);
    }
return s;
}

cf01_uint64 cf01_hex_str_to_uint64( const std::string& s ){
cf01_uint64 u = 0;
std::string::size_type i = 0;
static const std::string::size_type i_end_max = 2 * sizeof(cf01_uint64);
std::string::size_type i_end = s.length();
if( i_end > i_end_max ){ i_end = i_end_max; }
while( i < i_end ){
    const char& ch = s.at(i);
    cf01_uint8 nybble = 0;
    if( ( ch >= '0' ) && ( ch <= '9' ) ){
        nybble = static_cast<cf01_uint8>( ch - '0' );
        }
    else if( ( ch >= 'a' ) && ( ch <= 'f' ) ){
        nybble = static_cast<cf01_uint8>( ch - ('a' - static_cast<char>(10) ) );
        }
    else if( ( ch >= 'A' ) && ( ch <= 'F' ) ){
        nybble = static_cast<cf01_uint8>( ch - ('A' - static_cast<char>(10) ) );
        }
    u |= nybble;
    ++i;
    if( i < i_end ){
        u <<= 4;
        }
    }
return u;
}


