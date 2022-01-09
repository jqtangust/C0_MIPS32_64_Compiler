#pragma once
#include "head.h"

//单词结构体
typedef struct word
{
	int lex_symbol_ID;
	string lex_symbol;
	int lex_indexOfFile;
	int lex_indexOfLine;
};

//使用vector管理符号表，可以方便的实现入栈和出栈操作
extern vector<word> Lex_Table;

enum LEX_NUM
{
	lex_error_id,    //0//error
	lex_int_id,      //1//int
	lex_char_id,     //2//char
	lex_float_id,    //3//float
	lex_double_id,   //4//double
	lex_bool_id,     //5//bool
	lex_void_id,     //6//void
	lex_const_id,    //7//const
	lex_if_id,       //8//if
	lex_else_id,     //9//else
	lex_for_id,      //10//for
	lex_while_id,    //11//while
	lex_do_id,       //12//do
	lex_switch_id,   //13//switch
	lex_case_id,     //14//case
	lex_default_id,  //15//default
	lex_printf_id,   //16//print
	lex_scanf_id,    //17//scanf
	lex_return_id,   //18//return
	lex_main_id,     //19//main
	lex_indentifer_id,//20//identifer,标识符
	lex_add_id,      //21//+
	lex_subtract_id, //22//-
	lex_multiply_id, //23//*
	lex_divide_id,   //24//,///
	lex_percent_id,  //25//%
	lex_less_id,     //26//<
	lex_greater_id,  //27//>
	lex_lessequal_id,//28//<,//
	lex_greaterequal_id,//29//>,//
	lex_equalequal_id,//30//,//,//
	lex_notequal_id, //31//,//!,//
	lex_equal_id,    //32//,//
	lex_comma_id,    //33//,
	lex_semicolon_id,//34//;
	lex_colon_id,    //35//:
	lex_dot_id,      //36//.
	lex_singlemark_id,//37//'
	lex_doublemark_id,//38//"
	lex_leftparenthesis_id,//39//(
	lex_rightparenthesis_id,//40//)
	lex_leftmiddle_id,//41//[
	lex_rightmiddle_id,//42//]
	lex_leftbraces_id,//43//{
	lex_rightbraces_id,//44//}
	lex_NotZeroHeadNum_id,//45//不以0开头数字
	lex_string_id,    //46//字符串
	lex_character_id, //47//字符
	lex_letter_id,    //48//_
	lex_ZeroToNine,   //49//0~9
	lex_Zero_id,      //50//0
	lex_OneToNine_id, //51//1~9
	lex_ZeroHeadNum_id,//52//0开头数字
	lex_struct_id,    //53//结构体
};
