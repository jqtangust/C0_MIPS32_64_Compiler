#pragma once
#include "head.h"


#define op_const "const"
#define op_int "int"
#define op_char "char"
#define op_inta "inta"
#define op_chara "chara"
#define op_end "end"
#define op_add "+"
#define op_sub "-"
#define op_mul "*"
#define op_div "/"
#define op_func "func"
#define op_printf "printf"
#define op_scanf "scanf"
#define op_ret "ret"
#define op_lab "lab"
#define op_para "para"
#define op_callpara "callpara"
#define op_call "call"
#define op_jmp "jmp"
#define op_ass "="
#define op_arryass "=[]"
#define op_assarry "[]="
#define op_bt ">"
#define op_st "<"
#define op_eql "=="
#define op_neq "!="
#define op_bet ">="
#define op_set "<="
#define op_exits "exit"
#define op_void "void"
#define op_value "value"
#define op_space ""
#define op_zero "0"
#define MAX_NUM 1000010

typedef struct Quadruple
{
	string op;
	string src1;
	string src2;
	string res;
	int IsRepeat=0;
	int Which_Block;
	bool isentrance;
	bool isexit;
	bool islableorfunc;
};
extern int IR_Num;//四元式数量
extern Quadruple IR_List[MAX_NUM];

void IR_Output(string op, string src1, string src2, string res);
string genvar();
string genlable();
string genstring();
