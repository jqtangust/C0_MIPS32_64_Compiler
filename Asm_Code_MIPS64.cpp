#include "head.h"
#include "Asm_Code_MIPS64.h"
#include "Asm_Code.h"
#include "IR.h"
#include "Optim.h"
#include "Syntax.h"


using namespace std;

//#define ASM_64 cout

ofstream ASM_64("./MIPS64_Code.s");

bool istext_64 = false;

string nowfunc_MIPS64;
bool newfunc_MIPS64;

int paranum_64 = 0;
int preparanum_64;
int valparanum_64 = 0;

/////////////////////////////////////////////////////


void ASM_Output_MIPS64(string mips_64_op, string var1, string var2, string var3)//输出汇编语言
{
	if (mips_64_op[mips_64_op.length() - 1] == ':')//label or func
	{
		ASM_64 << endl;
	}
	else if (istext_64)//text
	{
		ASM_64 << "      ";
	}
	else//data
	{
		ASM_64 << "      ";
	}

	if (var1 == mips_64_space && var2 == mips_64_space && var3 == mips_64_space)
	{
		ASM_64 << mips_64_op;
		ASM_64 << endl;
	}
	else if (var2 == mips_64_space && var3 == mips_64_space)
	{
		ASM_64 << mips_64_op << "    " << var1;
		ASM_64 << endl;
	}

	else if (mips_64_op == ".data" || mips_64_op==".text")
	{
		ASM_64 << mips_64_op;
		ASM_64 << endl;
	}
	else if (var1 == ".ascii" || var1 == ".space" || var1 == ".word")
	{
		ASM_64 << mips_64_op << "    " << var1 << "    " << var2 << endl;
	}
	else
	{
		ASM_64 << mips_64_op << "    " << var1 << "," << var2;
		if (var3 != " ")
			ASM_64 << "," << var3 << endl;
		else
			ASM_64 << endl;
	}
}

//< int, , , a >
void mips_64_int()
{
	if (nowfunc_MIPS64 == "")//全局变量
	{
		Global_Var[res].type = "int";
	}
	else
	{
		offset -= intsize;
		Local_Var[res].offset = offset;
		Local_Var[res].type = "int";
		ASM_Output_MIPS64(mips_64_dsubi, reg_sp, reg_sp, to_string_asm(intsize));
	}
}

//< char, , , a >
void mips_64_char()
{
	if (nowfunc_MIPS64 == "")
	{
		Global_Var[res].type = "char";
	}
	else
	{
		offset -= charsize;
		Local_Var[res].offset = offset;
		Local_Var[res].type = "char";
		ASM_Output_MIPS64(mips_64_dsubi, reg_sp, reg_sp, to_string_asm(charsize));
	}
}

/**
int a[10];				< inta, 10, , a >
**/
void mips_64_intarry()
{
	if (nowfunc_MIPS64 == "")
	{
		Global_Var[res].type = intsize;
	}
	else
	{
		Local_Var[res].offset = offset - intsize;
		offset -= atoi(src1.c_str()) * intsize;
		Local_Var[res].type = "int";
		ASM_Output_MIPS64(mips_64_dsubi, reg_sp, reg_sp, to_string_asm(atoi(src1.c_str()) * intsize));//atoi:字符串转整形
	}
}

//< char, 10, , a >
void mips_64_chararry()
{
	if (nowfunc_MIPS64 == "")
	{
		Global_Var[res].type = "char";
	}
	else
	{
		Local_Var[res].offset = offset - charsize;
		offset -= atoi(src1.c_str()) * charsize;
		Local_Var[res].type = "char";
		ASM_Output_MIPS64(mips_64_dsubi, reg_sp, reg_sp, to_string_asm(atoi(src1.c_str()) * charsize));
	}
}

//////////////////////////////////////////////////////

/**
fun(int a,)				< para, int, , a >
fun(char a,)			< para, char, , a >
**/
void mips_64_para()//< para, int, , a >
{
	if (!newfunc_MIPS64)
	{
		newfunc_MIPS64 = true;
		savelocvar();
	}
	paranum_64++;
	Local_Var[res].offset = (paranum_64 + 1) * 4;
	Local_Var[res].type = src1;
}

/**
int a();				< func, int, , a >
char a();				< func, char, , a >
void a();				< func, void, , a >
**/
void mips_64_func()
{
	ASM_Output_MIPS64(res + ":", mips_64_space, mips_64_space, mips_64_space);
	if (!newfunc_MIPS64)
	{
		newfunc_MIPS64 = true;
		savelocvar();
	}
	nowfunc_MIPS64 = res;
}

/**
return ;				< ret, , , >
return a;				< ret, , , a >
**/
void mips_64_return()
{
	if (res != op_space)
	{
		if (isnumber(res))//imm
		{
			ASM_Output_MIPS64(mips_64_li, reg_t1, res, mips_64_space);
		}
		else if (Local_Var.find(res) != Local_Var.end())//local
		{
			ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_64_space);
		}
		else//global
		{
			ASM_Output_MIPS64(mips_64_la, reg_t0, res, mips_64_space);
			ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(0, reg_t0), mips_64_space);
		}
		ASM_Output_MIPS64(mips_64_move, reg_v0, reg_t1, mips_64_space);
	}
	if (res != "main")
	{
		ASM_Output_MIPS64(mips_64_jr, reg_ra, mips_64_space, mips_64_space);
	}
}

/**
	<exit, , a>
**/
void mips_64_exits()
{
	recoverlocvar();//恢复局部变量
	newfunc_MIPS64 = false;
	paranum_64 = 0;
	if (res == "main")//指示程序退出
	{
		ASM_Output_MIPS64(mips_64_li, reg_v0, main_end, mips_64_space);
		ASM_Output_MIPS64(mips_64_syscall, mips_64_space, mips_64_space, mips_64_space);
	}
}

/**
a + b					< +, a, b, >
**/
void mips_64_daddfun()
{
	if (isnumber(src1))//imm
	{
		ASM_Output_MIPS64(mips_64_li, reg_t1, src1, mips_64_space);
	}
	else if (isreg(src1))//reg
	{
		ASM_Output_MIPS64(mips_64_move, reg_t1, src1, mips_64_space);
	}
	else
	{
		if (Local_Var.find(src1) != Local_Var.end())//local
		{
			ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
		}
		else//global
		{
			ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
			ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(0, reg_t0), mips_64_space);
		}
	}

	if (isnumber(src2))//imm
	{
		ASM_Output_MIPS64(mips_64_li, reg_t2, src2, mips_64_space);
	}
	else if (isreg(src2))//reg
	{
		ASM_Output_MIPS64(mips_64_move, reg_t2, src2, mips_64_space);
	}
	else
	{
		if (Local_Var.find(src2) != Local_Var.end())//local
		{
			ASM_Output_MIPS64(mips_64_ld, reg_t2, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
		}
		else//global
		{
			ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
			ASM_Output_MIPS64(mips_64_ld, reg_t2, shamt(0, reg_t0), mips_64_space);
		}
	}

	ASM_Output_MIPS64(mips_64_dadd, reg_t1, reg_t1, reg_t2);

	if (isreg(res))
	{
		ASM_Output_MIPS64(mips_64_move, res, reg_t2, mips_64_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, res, mips_64_space);
		ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(0, reg_t0), mips_64_space);
	}

}

/**
a - b					< -, a, b, >
**/
void mips_64_dsubfun()
{
	if (isnumber(src1))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t1, src1, mips_64_space);
	}
	else if (isreg(src1))
	{
		ASM_Output_MIPS64(mips_64_move, reg_t1, src1, mips_64_space);
	}
	else
	{
		if (Local_Var.find(src1) != Local_Var.end())
		{
			ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
		}
		else
		{
			ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
			ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(0, reg_t0), mips_64_space);
		}
	}

	if (isnumber(src2))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t2, src2, mips_64_space);
	}
	else if (isreg(src2))
	{
		ASM_Output_MIPS64(mips_64_move, reg_t1, src1, mips_64_space);
	}
	else if (isreg(src1))
	{
		ASM_Output_MIPS64(mips_64_move, reg_t1, src1, mips_64_space);
	}
	else
	{
		if (Local_Var.find(src2) != Local_Var.end())
		{
			ASM_Output_MIPS64(mips_64_ld, reg_t2, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
		}
		else
		{
			ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
			ASM_Output_MIPS64(mips_64_ld, reg_t2, shamt(0, reg_t0), mips_64_space);
		}
	}

	ASM_Output_MIPS64(mips_64_dsub, reg_t1, reg_t1, reg_t2);

	if (isreg(res))
	{
		ASM_Output_MIPS64(mips_64_move, res, reg_t2, mips_64_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, res, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(0, reg_t0), mips_64_space);
	}

}

/**
a * b					< *, a, b, >
**/
void mips_64_dmulfun()
{
	if (isnumber(src1))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t1, src1, mips_64_space);
	}
	else if (isreg(src1))
	{
		ASM_Output_MIPS64(mips_64_move, reg_t1, src1, mips_64_space);
	}
	else
	{
		if (Local_Var.find(src1) != Local_Var.end())
		{
			ASM_Output_MIPS64(mips_64_ld, reg_t2, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
		}
		else
		{
			ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
			ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(0, reg_t0), mips_64_space);
		}
	}

	if (isnumber(src2))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t2, src2, mips_64_space);
	}
	else if (isreg(src2))
	{
		ASM_Output_MIPS64(mips_64_move, reg_t2, src2, mips_64_space);
	}
	else
	{
		if (Local_Var.find(src2) != Local_Var.end())
		{
			ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
		}
		else
		{
			ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
			ASM_Output_MIPS64(mips_64_ld, reg_t2, shamt(0, reg_t0), mips_64_space);
		}
	}

	ASM_Output_MIPS64(mips_64_dmul, reg_t1, reg_t1, reg_t2);

	if (isreg(res))
	{
		ASM_Output_MIPS64(mips_64_move, res, reg_t2, mips_64_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, res, mips_64_space);
		ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(0, reg_t0), mips_64_space);
	}
}

/**
a / b					< /, a, b, >
**/
void mips_64_ddivfun()
{
	if (isnumber(src1))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t1, src1, mips_64_space);
	}
	else if (isreg(src1))
	{
		ASM_Output_MIPS64(mips_64_move, reg_t1, src1, mips_64_space);
	}
	else
	{
		if (Local_Var.find(src1) != Local_Var.end())
		{
			ASM_Output_MIPS64(mips_64_ld, reg_t2, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
		}
		else
		{
			ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
			ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(0, reg_t0), mips_64_space);
		}
	}

	if (isnumber(src2))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t2, src2, mips_64_space);
	}
	else if (isreg(src2))
	{
		ASM_Output_MIPS64(mips_64_move, reg_t2, src2, mips_64_space);
	}
	else
	{
		if (Local_Var.find(src2) != Local_Var.end())
		{
			ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
		}
		else
		{
			ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
			ASM_Output_MIPS64(mips_64_ld, reg_t2, shamt(0, reg_t0), mips_64_space);
		}
	}

	ASM_Output_MIPS64(mips_64_ddiv, reg_t1, reg_t1, reg_t2);
	ASM_Output_MIPS64("mfhi", reg_t1, mips_64_space, mips_64_space);

	if (isreg(res))
	{
		ASM_Output_MIPS64(mips_64_move, res, reg_t2, mips_64_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, res, mips_64_space);
		ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(0, reg_t0), mips_64_space);
	}
}

/**
a = b					< =, b, , a >
**/
void mips_64_ass()
{
	if (isnumber(src1))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t1, src1, mips_64_space);
	}
	else if (isreg(src1))
	{
		ASM_Output_MIPS64(mips_64_move, reg_t1, src1, mips_64_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(0, reg_t0), mips_64_space);
	}

	if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, res, mips_64_space);
		ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(0, reg_t0), mips_64_space);
	}
}

/**
a[i] = b;				< []=, b, i, a >
**/
void mips_64_assarry()
{
	if (isnumber(src1))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t1, src1, mips_64_space);
	}
	else if (isreg(src1))
	{
		ASM_Output_MIPS64(mips_64_move, reg_t1, src1, mips_64_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(0, reg_t0), mips_64_space);
	}


	if (isnumber(src2))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t0, to_string(stoi(src2) * 4), mips_64_space);
	}
	else if (isreg(src2))
	{
		ASM_Output_MIPS64(mips_64_move, reg_t2, src2, mips_64_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t2, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t2, shamt(0, reg_t0), mips_64_space);
	}

	if (isreg(res))
	{
		std::cout << reg2var[res] << endl;
		if (Local_Var[reg2var[res]].type == "int")
		{
			ASM_Output_MIPS64(mips_64_dmul, reg_t2, reg_t2, "4");
		}
		//else
		//{
		//}
		ASM_Output_MIPS64(mips_64_daddi, reg_t0, reg_fp, to_string_asm(Local_Var[reg2var[res]].offset));
		ASM_Output_MIPS64(mips_64_dsub, reg_t0, reg_t0, reg_t2);
		ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(0, reg_t0), mips_64_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		if (Local_Var[res].type == "int")
		{
			ASM_Output_MIPS64(mips_64_dmul, reg_t2, reg_t2, "4");

		}
		//else
		//{(res+"("+reg_t0+")")
		//}
		//ASM_Output_MIPS64(mips_64_daddi, reg_t0, reg_fp, mips_64_space);
		//ASM_Output_MIPS64(mips_64_dsub, reg_t0, reg_t0, reg_t2);
		ASM_Output_MIPS64(mips_64_sd, reg_t1, res + shamt(0, reg_t0), mips_64_space);
	}
	else
	{
		if (Local_Var[res].type == "int")
		{
			ASM_Output_MIPS64(mips_64_dmul, reg_t2, reg_t2, "4");
		}
		else
		{
			//ASM_Output_MIPS64(mips_64_la, reg_t0, res, mips_64_space);
			//ASM_Output_MIPS64(mips_64_dsub, reg_t0, reg_t0, reg_t2);
			ASM_Output_MIPS64(mips_64_sd, reg_t1, res + shamt(0, reg_t0), mips_64_space);
		}
	}
}

/**
a = b[i];				< =[], i, b, a >
**/
void mips_64_arrassy() {

	ASM_Output_MIPS64(mips_64_li, reg_t0, to_string(stoi(src1) * 4), mips_64_space);//存偏移量
	ASM_Output_MIPS64(mips_64_ld, reg_t1, src2 + '(' + reg_t0 + ')', mips_64_space);//取出a[]
	ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_64_space);//存入b

}

/**
(int|char) fun();		< call, , , fun >
(void) fun();			< call, , , >
**/
void mips_64_call()
{
	ASM_Output_MIPS64(mips_64_dsubi, reg_sp, reg_sp, "4");// $sp = $sp - 4;
	ASM_Output_MIPS64(mips_64_sd, reg_ra, shamt(0, reg_sp), mips_64_space);// ($sp) = $ra
	ASM_Output_MIPS64(mips_64_dsubi, reg_sp, reg_sp, "4");// $sp = $sp - 4;
	ASM_Output_MIPS64(mips_64_sd, reg_fp, shamt(0, reg_sp), mips_64_space);// ($sp) = $fp
	ASM_Output_MIPS64(mips_64_move, reg_fp, reg_sp, mips_64_space);// $fp = $sp

	ASM_Output_MIPS64(mips_64_jal, src1, mips_64_space, mips_64_space);
	ASM_Output_MIPS64(mips_64_nop, mips_64_space, mips_64_space, mips_64_space);
	ASM_Output_MIPS64(mips_64_move, reg_sp, reg_fp, mips_64_space);// recover sp
	ASM_Output_MIPS64(mips_64_ld, reg_ra, shamt(4, reg_fp), mips_64_space);// recover ra
	ASM_Output_MIPS64(mips_64_ld, reg_fp, shamt(0, reg_fp), mips_64_space);// recover fp
	// store return Value
	if (isreg(res))
	{
		ASM_Output_MIPS64(mips_64_move, res, reg_v0, mips_64_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_sd, reg_v0, shamt(Local_Var[res].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, res, mips_64_space);
		ASM_Output_MIPS64(mips_64_sd, reg_v0, shamt(0, reg_t0), mips_64_space);
	}
}

/**
fun(a);					< callpara, int|char, , a >
**/
void mips_64_callpara()
{
	if (isnumber(res))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t1, res, mips_64_space);
	}
	else if (isreg(res))
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t1, res, mips_64_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, res, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t1, shamt(0, reg_t0), mips_64_space);
	}
	ASM_Output_MIPS64(mips_64_dsubi, reg_sp, reg_sp, "4");
	ASM_Output_MIPS64(mips_64_sd, reg_t1, shamt(0, reg_sp), mips_64_space);
}


//////////////////////////////////////////////////////////////

/**
set label				< lab, , , label >
**/
void mips_64_lab()
{
	ASM_Output_MIPS64(res + ":", mips_64_space, mips_64_space, mips_64_space);
}

/**
jmp	label		 		< j, , , label>
**/
void mips_64_jmp()
{
	ASM_Output_MIPS64(mips_64_j, res, mips_64_space, mips_64_space);
}

//////////////////////////////////////////////////////////////

/**
if(a < b)				< blt, a, b, label >
**/
void mips_64_bltfun()
{

	if (isnumber(src1))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t8, src1, mips_64_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t8, src1, mips_64_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(0, reg_t0), mips_64_space);
	}

	if (isnumber(src2))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t9, src2, mips_64_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t9, src2, mips_64_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(0, reg_t0), mips_64_space);
	}

	ASM_Output_MIPS64(mips_64_blt, reg_t8, reg_t9, res);
}

/**
if(a <= b)				< ble, a, b, label >
**/
void mips_64_blefun()
{
	if (isnumber(src1))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t8, src1, mips_64_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t8, src1, mips_64_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(0, reg_t0), mips_64_space);
	}
	if (isnumber(src2))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t9, src2, mips_64_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t9, src2, mips_64_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(0, reg_t0), mips_64_space);
	}
	ASM_Output_MIPS64(mips_64_ble, reg_t8, reg_t9, res);
}

/**
if(a > b)				< bgt, a, b, label >
**/
void mips_64_bgtfun()
{
	if (isnumber(src1))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t8, src1, mips_64_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t8, src1, mips_64_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(0, reg_t0), mips_64_space);
	}
	if (isnumber(src2))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t9, src2, mips_64_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t9, src2, mips_64_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(0, reg_t0), mips_64_space);
	}
	ASM_Output_MIPS64(mips_64_bgt, reg_t8, reg_t9, res);
}

/**
if(a >= b)				< bge, a, b, label >
**/
void mips_64_bgefun()
{
	if (isnumber(src1))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t8, src1, mips_64_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t8, src1, mips_64_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(0, reg_t0), mips_64_space);
	}
	if (isnumber(src2))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t9, src2, mips_64_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t9, src2, mips_64_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(0, reg_t0), mips_64_space);
	}
	ASM_Output_MIPS64(mips_64_bge, reg_t8, reg_t9, res);
}

/**
if(a == b)				< beq, a, b, label >
**/
void mips_64_bnefun()
{
	if (isnumber(src1))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t8, src1, mips_64_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t8, src1, mips_64_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(0, reg_t0), mips_64_space);
	}
	if (isnumber(src2))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t9, src2, mips_64_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t9, src2, mips_64_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(0, reg_t0), mips_64_space);
	}
	ASM_Output_MIPS64(mips_64_bne, reg_t8, reg_t9, res);
}

/**
if(a != b)				< bne, a, b, label >
**/
void mips_64_beqfun()
{
	if (isnumber(src1))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t8, src1, mips_64_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t8, src1, mips_64_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t8, shamt(0, reg_t0), mips_64_space);
	}
	if (isnumber(src2))
	{
		ASM_Output_MIPS64(mips_64_li, reg_t9, src2, mips_64_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output_MIPS64(mips_64_move, reg_t9, src2, mips_64_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
		ASM_Output_MIPS64(mips_64_ld, reg_t9, shamt(0, reg_t0), mips_64_space);
	}
	ASM_Output_MIPS64(mips_64_beq, reg_t8, reg_t9, res);
}

//////////////////////////////////////////////////////////////

/**
scanf(a);				< scf, , , a >
**/
void mips_64_scf()
{
	if (Local_Var.find(res) != Local_Var.end())
	{
		if (Local_Var[res].type == "int")
		{
			ASM_Output_MIPS64(mips_64_li, reg_v0, scfint, mips_64_space);
		}
		else

		{
			ASM_Output_MIPS64(mips_64_li, reg_v0, scfchar, mips_64_space);
		}
	}
	else//glb
	{
		if (Global_Var[res].type == "int")
		{
			ASM_Output_MIPS64(mips_64_li, reg_v0, scfint, mips_64_space);
		}
		else
		{
			ASM_Output_MIPS64(mips_64_li, reg_v0, scfchar, mips_64_space);
		}
	}
	ASM_Output_MIPS64(mips_64_syscall, mips_64_space, mips_64_space, mips_64_space);//syscall
	if (isreg(res))
	{
		ASM_Output_MIPS64(mips_64_sd, reg_v0, res, mips_64_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output_MIPS64(mips_64_sd, reg_v0, shamt(Local_Var[res].offset, reg_fp), mips_64_space);
	}
	else
	{
		ASM_Output_MIPS64(mips_64_li, reg_v1, res, mips_64_space);
		ASM_Output_MIPS64(mips_64_sd, reg_v0, shamt(0, reg_v1), mips_64_space);
	}
	// 处理读入时的行末回车
	if (Local_Var.find(res) != Local_Var.end())
	{
		if (Local_Var[res].type == "int")
		{
		}
		else
		{
			ASM_Output_MIPS64(mips_64_li, reg_v0, scfchar, mips_64_space);
			ASM_Output_MIPS64(mips_64_syscall, mips_64_space, mips_64_space, mips_64_space);
		}
	}
	else
	{
		if (Global_Var[res].type == "int")
		{
		}
		else
		{
			ASM_Output_MIPS64(mips_64_li, reg_v0, scfchar, mips_64_space);
			ASM_Output_MIPS64(mips_64_syscall, mips_64_space, mips_64_space, mips_64_space);
		}
	}
}

/**
print("a",b);			< prf, a, b,  >
**/
void mips_64_prf()
{
	if (src1 == op_space)//src1==""
	{
	}
	else if (isstring(src1))// src1==string
	{
		ASM_Output_MIPS64(mips_64_la, reg_a0, String_List[src1], mips_64_space);
		ASM_Output_MIPS64(mips_64_li, reg_v0, prfstr, mips_64_space);
		ASM_Output_MIPS64(mips_64_syscall, mips_64_space, mips_64_space, mips_64_space);
	}
	else// src1==expr
	{
		if (isnumber(src1))
		{
			ASM_Output_MIPS64(mips_64_li, reg_a0, res, mips_64_space);
			ASM_Output_MIPS64(mips_64_li, reg_v0, prfint, mips_64_space);
		}
		else if (isreg(src1))
		{
			ASM_Output_MIPS64(mips_64_move, reg_a0, src1, mips_64_space);
		}
		else if (Local_Var.find(src1) != Local_Var.end())
		{
			ASM_Output_MIPS64(mips_64_ld, reg_a0, shamt(Local_Var[src1].offset, reg_fp), mips_64_space);
			if (Local_Var[src1].type == "int")
			{
				ASM_Output_MIPS64(mips_64_li, reg_v0, prfint, mips_64_space);
			}
			else
			{
				ASM_Output_MIPS64(mips_64_li, reg_v0, prfchar, mips_64_space);
			}
		}
		else
		{
			ASM_Output_MIPS64(mips_64_la, reg_t0, src1, mips_64_space);
			ASM_Output_MIPS64(mips_64_ld, reg_a0, shamt(0, reg_t0), mips_64_space);
			if (Global_Var[src1].type == "int")
			{
				ASM_Output_MIPS64(mips_64_li, reg_v0, prfint, mips_64_space);
			}
			else
			{
				ASM_Output_MIPS64(mips_64_li, reg_v0, prfchar, mips_64_space);
			}
		}
		ASM_Output_MIPS64(mips_64_syscall, mips_64_space, mips_64_space, mips_64_space);
	}
	//src2
	if (src2 != op_space)
	{
		if (isnumber(src2))
		{
			ASM_Output_MIPS64(mips_64_li, reg_a0, res, mips_64_space);
			ASM_Output_MIPS64(mips_64_li, reg_v0, prfint, mips_64_space);
		}
		else if (isreg(src2))
		{
			ASM_Output_MIPS64(mips_64_move, reg_a0, src2, mips_64_space);
		}
		else if (Local_Var.find(src2) != Local_Var.end())
		{
			ASM_Output_MIPS64(mips_64_ld, reg_a0, shamt(Local_Var[src2].offset, reg_fp), mips_64_space);
			if (Local_Var[src2].type == "int")
			{
				ASM_Output_MIPS64(mips_64_li, reg_v0, prfint, mips_64_space);
			}
			else
			{
				ASM_Output_MIPS64(mips_64_li, reg_v0, prfchar, mips_64_space);
			}
		}
		else
		{
			ASM_Output_MIPS64(mips_64_la, reg_t0, src2, mips_64_space);
			ASM_Output_MIPS64(mips_64_ld, reg_a0, shamt(0, reg_t0), mips_64_space);
			if (Global_Var[src2].type == "int")
			{
				ASM_Output_MIPS64(mips_64_li, reg_v0, prfint, mips_64_space);
			}
			else
			{
				ASM_Output_MIPS64(mips_64_li, reg_v0, prfchar, mips_64_space);
			}
		}
		ASM_Output_MIPS64(mips_64_syscall, mips_64_space, mips_64_space, mips_64_space);
	}
}

//////////////////////////////////////////////////////////////

void ASM_init_MIPS64()//初始化
{
	offset = 0;
	paranum_64 = 0;
	preparanum_64 = 0;
	valparanum_64 = 0;
	nowfunc_MIPS64 = "";
	Local_Var.clear();
	Global_Var.clear();
	Local_Var_Res.clear();
	newfunc_MIPS64 = false;
}

void Data_Num_MIPS64()//数值型变量
{
	for (int i = 1; i <= IR_Num; i++)
	{
		string op = IR_List[i].op;
		src1 = IR_List[i].src1;
		src2 = IR_List[i].src2;
		res = IR_List[i].res;
		if (op == op_func || op == op_para)
		{
			break;//不进行任何操作
		}
		if (op == op_const)
		{
			if (src1 == "int")
			{
				ASM_Output_MIPS64(res + ":", ".word", "0", mips_64_space);
			}
			else if (src1 == "char")
			{
				ASM_Output_MIPS64(res + ":", ".word", "0", mips_64_space);
			}
		}
		else if (op == op_int)
		{
			ASM_Output_MIPS64(res + ":", ".word", "0", mips_64_space);
		}
		else if (op == op_char)
		{
			ASM_Output_MIPS64(res + ":", ".word", "0", mips_64_space);
		}
		else if (op == op_inta)
		{
			ASM_Output_MIPS64(res + ":", ".space", to_string_asm(atoi(src1.c_str()) * intsize), mips_64_space);
		}
		else if (op == op_chara)
		{
			ASM_Output_MIPS64(res + ":", ".space", to_string_asm(atoi(src1.c_str()) * intsize), mips_64_space);
		}
	}

}
void Data_String_MIPS64()//字符串
{
	map<string, string>::iterator iter;
	iter = String_List.begin();
	while (iter != String_List.end())
	{
		string tmp = "";
		for (int i = 0; i < (int)iter->first.length() - 1; i++)
		{
			tmp += iter->first[i];
		}
		tmp += "\\0\"";
		ASM_Output_MIPS64(iter->second + ":", ".ascii", tmp, mips_64_space);
		iter++;
	}
}

void Text_ASM_MIPS64()//生成MIPS的text段
{
	istext_64 = true;
	bool flag = false;
	for (int i = 1; i <= IR_Num; i++)
	{

		string op = IR_List[i].op;
		src1 = IR_List[i].src1;
		src2 = IR_List[i].src2;
		res = IR_List[i].res;
		ASM_64 << endl;
		if (!flag && (op == op_func || op == op_para))
		{
			ASM_Output_MIPS64(mips_64_j, "main", mips_64_space, mips_64_space);
			flag = true;
		}
		//ASM_64 << "#< ";
		//ASM_64 << setw(8) << left << op << ",";
		//ASM_64 << setw(8) << left << src1 << ",";
		//ASM_64 << setw(8) << left << src2 << ",";
		//ASM_64 << setw(8) << left << res;
		//ASM_64 << " >" << endl;

		if (op == op_int || (op == op_const && src1 == op_int)) mips_64_int();
		else if (op == op_char || (op == op_const && src1 == op_char)) mips_64_char();
		else if (op == op_add) mips_64_daddfun();
		else if (op == op_sub) mips_64_dsubfun();
		else if (op == op_mul) mips_64_dmulfun();
		else if (op == op_div) mips_64_ddivfun();
		else if (op == op_ass) mips_64_ass();
		else if (op == op_para) mips_64_para();
		else if (op == op_func) mips_64_func();
		else if (op == op_call) mips_64_call();
		else if (op == op_ret) mips_64_return();
		else if (op == op_callpara) mips_64_callpara();
		else if (op == op_lab) mips_64_lab();
		else if (op == op_jmp) mips_64_jmp();
		else if (op == op_st) mips_64_bltfun();
		else if (op == op_set) mips_64_blefun();
		else if (op == op_bt) mips_64_bgtfun();
		else if (op == op_eql) mips_64_beqfun();
		else if (op == op_neq) mips_64_bnefun();
		else if (op == op_exits) mips_64_exits();
		else if (op == op_scanf) mips_64_scf();
		else if (op == op_printf) mips_64_prf();
		else if (op == op_assarry) mips_64_assarry();
		else if (op == op_arryass) mips_64_arrassy();
		else if (op == op_inta) mips_64_intarry();
		else if (op == op_chara) mips_64_chararry();

	}
}


void Asm_Code_64_main()//运行汇编语言生成
{
	offset = 0;
	Global_Var.clear();//全局变量
	Local_Var.clear();//局部变量
	Local_Var_Res.clear();//记录上一个函数的局部变量
	reg2var.clear();

	ASM_init_MIPS64();

	ASM_Output_MIPS64(".data", mips_64_space, mips_64_space, mips_64_space);

	Data_Num_MIPS64();//Num变量
	Data_String_MIPS64();//String变量

	ASM_Output_MIPS64(".text", mips_64_space, mips_64_space, mips_64_space);
	ASM_Output_MIPS64(".globl", "main", mips_64_space, mips_64_space);

	Text_ASM_MIPS64();//Text变量
}

