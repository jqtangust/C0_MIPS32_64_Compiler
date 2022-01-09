#pragma once
#include "head.h"
#include "Lex.h"

extern string syn_keywordID[keywordNum];//关键字ID
extern map<string, string> String_List;

void Syntax_Analysis_main();

///语法分析
void syn_add();//+/-
void syn_multiply();//*//
void syn_relation();//关系
void syn_letter();//字母
void syn_digit();//数字
void syn_notZeroDigit();//不以零开头数字
void syn_character();//字符
void syn_string();//字符串
void Syntax_Analysis();//程序
void syn_const_dec();//常量声明
void syn_const_def();//常量定义
void syn_noSignInt();//无符号整型
void syn_int();//整型
void syn_Fractional();//小数
void syn_realNumber();//实数
void syn_identifier();//标识符（自定义符号）
void syn_headFile();//头部声明（相当于函数声明）
void syn_var_dec();//变量声明
void syn_var_def();//变量定义
void syn_enum();//可枚举常量
void syn_typeidentifier();//类型标识符
void syn_functionwithreturn();//有返回值函数的定义
void syn_functionwithoutreturn();//无返回值函数的定义
void syn_mixsentence();//混合语句
void syn_parameter();//参数
void syn_parameter_table();//参数表
void syn_main();//主函数
void syn_expression();//表达式
void syn_term();//项
void syn_factor();//因子
void syn_sentence();//语句
void syn_assign_sen();//赋值语句
void syn_condition_sen();//条件语句
void syn_condition();//条件本身
void syn_loop_sen();//循环语句
void syn_situation_sen();//情况语句
void syn_situation_table();//情况表
void syn_situation_sen_son();//情况子语句
void syn_default();//default
void syn_functionwithreturn_recall();//调用有返回值的函数
void syn_functionwithoutreturn_recall();//调用无返回值的函数
void syn_value_parameter_table();//值参数表
void syn_sentence_colomn();//语句列
void syn_scanf();//读
void syn_printf();//写
void syn_return();//返回