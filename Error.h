#pragma once

void Grammar_Error(int err_ID, int err_line, int err_coloumn);

extern bool Error_Flag;

///ERROR
#define err_const_redefination          1
#define err_var_redefination            2
#define err_function_redefination       3
#define err_lacksemicolon               4
#define err_var_undefination            5
#define err_function_undefination       6
#define err_const_assign                7
#define err_addorsub                    8
#define err_mulordiv                    9
#define err_relation                    10
#define err_const                       11
#define err_intorchar                   12
#define err_ass                         13
#define err_number                      14
#define err_int                         15
#define err_identifier                  16
#define err_leftparenthesis             17
#define err_rightparenthesis            18
#define err_leftmiddle                  19
#define err_rightmiddle                 20
#define err_leftbraces                  21
#define err_rightbraces                 22
#define err_void                        23
#define err_main                        24
#define err_printf                      25
#define err_scanf                       26
#define err_colon                       27
#define err_case                        28
#define err_switch                      29
#define err_while                       30
#define err_if                          31
#define err_do                          32
#define err_for                         33
#define err_assign                      34
#define err_semicolon                   35
#define err_return                      36
#define err_letter                      37
#define err_digit                       38
#define err_notzero                     39
#define err_singlemark                  40
#define err_doublemark                  41
#define err_character                   42
#define err_string                      43
#define err_enumerabel                  44
