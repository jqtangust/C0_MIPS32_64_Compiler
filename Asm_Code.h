#pragma once

#include "IR.h"
//asm
#define mips_nop   "nop"

#define mips_move  "move"
#define mips_mul   "mul"
#define mips_div   "div"
#define mips_add   "add"
#define mips_sub   "sub"
#define mips_addi  "addi"
#define mips_subi  "subi"

#define mips_li    "li"
#define mips_la    "la"
#define mips_lw    "lw"
#define mips_sw    "sw"

#define mips_jal   "jal"
#define mips_jr    "jr"
#define mips_j     "j"

#define mips_beq   "beq"
#define mips_bne   "bne"
#define mips_blt   "blt"
#define mips_bgt   "bgt"
#define mips_ble   "ble"
#define mips_bge   "bge"

#define mips_addu  "addu"
#define mips_addiu "addiu"
#define mips_subu  "subu"
#define mips_subiu "subiu"
#define mips_and   "and"
#define mips_or    "or"
#define mips_xor   "xor"
#define mips_nor   "nor"

#define mips_slt   "slt"
#define mips_slta  "slta"
#define mips_sll   "sll"

#define mips_shamt "shamt"
#define mips_sra   "sra"
#define mips_sllv  "sllv"
#define mips_srlv  "srlv"
#define mips_srav  "srav"
#define mips_ori   "ori"
#define mips_xori  "xori"
#define mips_lui   "lui"
#define mips_slti  "slti"
#define mips_sltiu "sltiu"
#define mips_space " "

#define mips_syscall "syscall"
//
///**
//zero    0
//at      用作汇编器的暂时变量
//v0,v1   子函数调用返回结果
//a0-a3   子函数调用的参数
//t0-t7   暂时变量，子函数使用时不需要保存于恢复
//t8-t9   暂时变量，子函数使用时不需要保存与恢复
//s0-s7   子函数寄存器变量，在返回之前子函数必须保存和恢复使用过的变量，从而
//		调用函数知道这些寄存器的值没有变化
//k1,k2   通常被中断或异常处理程序使用作为保存一些系统参数
//gp      全局指针。一些系统维护这个指针来更方便的存取static和extern变量
//sp      堆栈指针
//s8/fp   第九个寄存器变量/框架指针
//ra      子函数的返回地址
//**/
//#define reg_sp "$sp"
//#define reg_fp "$fp"
//#define reg_v0 "$v0"
//#define reg_v1 "$v1"
//#define reg_ra "$ra"
//#define reg_a0 "$a0"
//#define reg_a1 "$a1"
//#define reg_a2 "$a2"
//#define reg_a3 "$a3"
//#define reg_t0 "$t0"
//#define reg_t1 "$t1"
//#define reg_t2 "$t2"
//#define reg_t3 "$t3"
//#define reg_t4 "$t4"
//#define reg_t5 "$t5"
//#define reg_t6 "$t6"
//#define reg_t7 "$t7"
//#define reg_t8 "$t8"
//#define reg_t9 "$t9"
//#define reg_s0 "$s0"
//#define reg_s1 "$s1"
//#define reg_s2 "$s2"
//#define reg_s3 "$s3"
//#define reg_s4 "$s4"
//#define reg_s5 "$s5"
//#define reg_s6 "$s6"
//#define reg_s7 "$s7"
//#define reg_k0 "$k0"
//#define reg_k1 "$k1"

#define reg_zero    "$0"
#define reg_at      "$1"
#define reg_v0      "$2"
#define reg_v1      "$3"

#define reg_a0      "$4"
#define reg_a1      "$5"
#define reg_a2      "$6"
#define reg_a3      "$7"

#define reg_t0      "$8"
#define reg_t1      "$9"
#define reg_t2      "$10"
#define reg_t3      "$11"
#define reg_t4      "$12"
#define reg_t5      "$13"
#define reg_t6      "$14"
#define reg_t7      "$15"

#define reg_s0      "$16"
#define reg_s1      "$17"
#define reg_s2      "$18"
#define reg_s3      "$19"
#define reg_s4      "$20"
#define reg_s5      "$21"
#define reg_s6      "$22"
#define reg_s7      "$23"

#define reg_t8      "$24"
#define reg_t9      "$25"

#define reg_k0      "$26"
#define reg_k1      "$27"

#define reg_gp      "$gp"
#define reg_sp      "$sp"
#define reg_fp      "$fp"

#define reg_ra      "$ra"

#define prfint   "1"
#define prfchar  "11"
#define prfstr   "4"
#define scfint   "5"
#define scfchar  "12"

#define main_end  "10"

#define intsize 4
#define charsize 4

void Asm_Code_main();

extern ofstream ASM;
extern int paranum;
extern int valparanum;
extern string res, src1, src2;
extern map<string, string> reg2var;
extern int offset;

typedef struct
{
	int offset;
	string type;
}Var_Address;

extern map<string, Var_Address> Global_Var;
extern map<string, Var_Address> Local_Var;
extern map<string, Var_Address> Local_Var_Res;



//extern asmsentence asmsent[MAX_NUM];

string to_string_asm(int n);
string shamt(int a, string b);
void savelocvar();
void recoverlocvar();
void outlocvar();

bool isnumber(string str);
bool isstring(string str);
bool isreg(string str);


void ASM_init();
void ASM_Output(string op, string var1, string var2, string var3);
void mips_int();
void mips_char();
void mips_addfun();
void mips_subfun();
void mips_mulfun();
void mips_divfun();
void mips_ass();
void mips_para();
void mips_func();
void mips_call();
void mips_return();
void mips_callpara();
void mips_lab();
void mips_jmp();
void mips_bltfun();
void mips_blefun();
void mips_bgtfun();
void mips_bgefun();
void mips_beqfun();
void mips_bnefun();
void mips_exits();
void mips_scf();
void mips_prf();
void mips_assarry();
void mips_intarry();
void mips_chararry();
