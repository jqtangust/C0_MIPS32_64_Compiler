#include "head.h"
#include "Asm_Code.h"
#include "IR.h"
#include "Optim.h"
#include "Syntax.h"


using namespace std;

//#define ASM cout

ofstream ASM("./code.s");


const string intsym = "int";
const string charsym = "char";

typedef struct
{
	string op;
	string var1;
	string var2;
	string var3;
	bool istext;
	bool isenterance;
	bool isexit;
	int blocknum;
	string line;
}ASM_Word;

ASM_Word ASM_List[MAX_NUM];

int asmsentnum = 0;
bool istext = false;

map<string, Var_Address> Global_Var;//全局变量
map<string, Var_Address> Local_Var;//局部变量
map<string, Var_Address> Local_Var_Res;//记录上一个函数的局部变量

map<string, string> reg2var;

string src1, src2, res;
int offset = 0;
string nowfunc;

int paranum = 0;
int preparanum;
int valparanum = 0;

bool newfunc;
int state = 0;//表示程序运行到第几状态

//计算地址偏移
string shamt(int a, string b)
{
	return to_string_asm(a) + "(" + b + ")";
}

//保存局部变量
void savelocvar()
{
	Local_Var_Res.clear();
	map<string, Var_Address>::iterator iter;
	iter = Local_Var.begin();
	while (iter != Local_Var.end())
	{
		Local_Var_Res[iter->first] = iter->second;
		iter++;
	}
	Local_Var.clear();
}
//恢复局部变量
void recoverlocvar()
{
	Local_Var.clear();
	map<string, Var_Address>::iterator iter;
	iter = Local_Var_Res.begin();
	while (iter != Local_Var_Res.end())
	{
		Local_Var[iter->first] = iter->second;
		iter++;
	}
	Local_Var_Res.clear();
}
//输出局部变量
void outlocvar()
{
	map<string, Var_Address>::iterator iter;
	iter = Local_Var.begin();
	while (iter != Local_Var.end())
	{
		ASM << iter->first << ":" << iter->second.offset << endl;
		iter++;
	}
}

//判断是否是数字
bool isnumber(string str)
{
	if ((str[0] >= '0' && str[0] <= '9') || str[0] == '-')
	{
		return true;
	}
	return false;
}
//判断是否是字符串
bool isstring(string str)
{
	if (str[0] == '\"')
	{
		return true;
	}
	return false;
}
//判断是否是寄存器
bool isreg(string str)
{
	if (str[0] == '$')
	{
		return true;
	}
	return false;
}

//转化为字符串
string to_string_asm(int n)
{
	int m = n;
	char s[100];
	char ss[100];
	int i = 0, j = 0;
	if (n < 0)
	{
		m = 0 - m;
		j = 1;
		ss[0] = '-';
	}
	while (m > 0)
	{
		s[i++] = m % 10 + '0';
		m /= 10;
	}
	s[i] = '\0';
	i = i - 1;
	while (i >= 0)
	{
		ss[j++] = s[i--];
	}
	ss[j] = '\0';
	return ss;
}

/////////////////////////////////////////////////////
//< int, , , a >
void mips_int()
{
	if (nowfunc == "")//全局变量
	{
		Global_Var[res].type = intsym;
	}
	else
	{
		offset -= intsize;
		Local_Var[res].offset = offset;
		Local_Var[res].type = intsym;
		ASM_Output(mips_subi, reg_sp, reg_sp, to_string_asm(intsize));
	}
}

//< char, , , a >
void mips_char()
{
	if (nowfunc == "")
	{
		Global_Var[res].type = charsym;
	}
	else
	{
		offset -= charsize;
		Local_Var[res].offset = offset;
		Local_Var[res].type = charsym;
		ASM_Output(mips_subi, reg_sp, reg_sp, to_string_asm(charsize));
	}
}

/**
int a[10];				< inta, 10, , a >
**/
void mips_intarry()
{
	if (nowfunc == "")
	{
		Global_Var[res].type = intsize;
	}
	else
	{
		Local_Var[res].offset = offset - intsize;
		offset -= atoi(src1.c_str()) * intsize;
		Local_Var[res].type = intsym;
		ASM_Output(mips_subi, reg_sp, reg_sp, to_string_asm(atoi(src1.c_str()) * intsize));//atoi:字符串转整形
	}
}

//< char, 10, , a >
void mips_chararry()
{
	if (nowfunc == "")
	{
		Global_Var[res].type = charsym;
	}
	else
	{
		Local_Var[res].offset = offset - charsize;
		offset -= atoi(src1.c_str()) * charsize;
		Local_Var[res].type = charsym;
		ASM_Output(mips_subi, reg_sp, reg_sp, to_string_asm(atoi(src1.c_str()) * charsize));
	}
}

//////////////////////////////////////////////////////

/**
fun(int a,)				< para, int, , a >
fun(char a,)			< para, char, , a >
**/
void mips_para()//< para, int, , a >
{
	if (!newfunc)
	{
		newfunc = true;
		savelocvar();
	}
	paranum++;
	Local_Var[res].offset = (paranum + 1) * 4;//？
	Local_Var[res].type = src1;
}

/**
int a();				< func, int, , a >
char a();				< func, char, , a >
void a();				< func, void, , a >
**/
void mips_func()
{
	ASM_Output(res + ":", mips_space, mips_space, mips_space);
	if (!newfunc)
	{
		newfunc = true;
		savelocvar();
	}
	nowfunc = res;
}

/**
return ;				< ret, , , >
return a;				< ret, , , a >
**/
void mips_return()
{
	if (res != op_space)
	{
		if (isnumber(res))//imm
		{
			ASM_Output(mips_li, reg_t1, res, mips_space);
		}
		else if (Local_Var.find(res) != Local_Var.end())//local
		{
			ASM_Output(mips_lw, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_space);
		}
		else//global
		{
			ASM_Output(mips_la, reg_t0, res, mips_space);
			ASM_Output(mips_lw, reg_t1, shamt(0, reg_t0), mips_space);
		}
		ASM_Output(mips_move, reg_v0, reg_t1, mips_space);
	}
	if (res != "main")
	{
		ASM_Output(mips_jr, reg_ra, mips_space, mips_space);
	}
}

/**
	<exit, , a>
**/
void mips_exits()
{
	recoverlocvar();//恢复局部变量
	newfunc = false;
	paranum = 0;
	if (res == "main")//指示程序退出
	{
		ASM_Output(mips_li,reg_v0, main_end, mips_space);
		ASM_Output(mips_syscall, mips_space, mips_space, mips_space);
	}
}

/**
a + b					< +, a, b, >
**/
void mips_addfun()
{
	if (isnumber(src1))//imm
	{
		ASM_Output(mips_li, reg_t1, src1, mips_space);
	}
	else if (isreg(src1))//reg
	{
		ASM_Output(mips_move, reg_t1, src1, mips_space);
	}
	else
	{
		if (Local_Var.find(src1) != Local_Var.end())//local
		{
			ASM_Output(mips_lw, reg_t1, shamt(Local_Var[src1].offset, reg_fp), mips_space);
		}
		else//global
		{
			ASM_Output(mips_la, reg_t0, src1, mips_space);
			ASM_Output(mips_lw, reg_t1, shamt(0, reg_t0), mips_space);
		}
	}

	if (isnumber(src2))//imm
	{
		ASM_Output(mips_li, reg_t2, src2, mips_space);
	}
	else if (isreg(src2))//reg
	{
		ASM_Output(mips_move, reg_t2, src2, mips_space);
	}
	else
	{
		if (Local_Var.find(src2) != Local_Var.end())//local
		{
			ASM_Output(mips_lw, reg_t2, shamt(Local_Var[src2].offset, reg_fp), mips_space);
		}
		else//global
		{
			ASM_Output(mips_la, reg_t0, src2, mips_space);
			ASM_Output(mips_lw, reg_t2, shamt(0, reg_t0), mips_space);
		}
	}

	ASM_Output(mips_add, reg_t1, reg_t1, reg_t2);

	if (isreg(res))
	{
		ASM_Output(mips_move, res, reg_t2, mips_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output(mips_sw, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, res, mips_space);
		ASM_Output(mips_sw, reg_t1, shamt(0, reg_t0), mips_space);
	}

}

/**
a - b					< -, a, b, >
**/
void mips_subfun()
{
	if (isnumber(src1))
	{
		ASM_Output(mips_li, reg_t1, src1, mips_space);
	}
	else if (isreg(src1))
	{
		ASM_Output(mips_move, reg_t1, src1, mips_space);
	}
	else
	{
		if (Local_Var.find(src1) != Local_Var.end())
		{
			ASM_Output(mips_lw, reg_t1, shamt(Local_Var[src1].offset, reg_fp), mips_space);
		}
		else
		{
			ASM_Output(mips_la, reg_t0, src1, mips_space);
			ASM_Output(mips_lw, reg_t1, shamt(0, reg_t0), mips_space);
		}
	}

	if (isnumber(src2))
	{
		ASM_Output(mips_li, reg_t2, src2, mips_space);
	}
	else if (isreg(src2))
	{
		ASM_Output(mips_move, reg_t1, src1, mips_space);
	}
	else if (isreg(src1))
	{
		ASM_Output(mips_move, reg_t1, src1, mips_space);
	}
	else
	{
		if (Local_Var.find(src2) != Local_Var.end())
		{
			ASM_Output(mips_lw, reg_t2, shamt(Local_Var[src2].offset, reg_fp), mips_space);
		}
		else
		{
			ASM_Output(mips_la, reg_t0, src2, mips_space);
			ASM_Output(mips_lw, reg_t2, shamt(0, reg_t0), mips_space);
		}
	}

	ASM_Output(mips_sub, reg_t1, reg_t1, reg_t2);

	if (isreg(res))
	{
		ASM_Output(mips_move, res, reg_t2, mips_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output(mips_sw, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, res, mips_space);
		ASM_Output(mips_lw, reg_t1, shamt(0, reg_t0), mips_space);
	}

}

/**
a * b					< *, a, b, >
**/
void mips_mulfun()
{
	if (isnumber(src1))
	{
		ASM_Output(mips_li, reg_t1, src1, mips_space);
	}
	else if (isreg(src1))
	{
		ASM_Output(mips_move, reg_t1, src1, mips_space);
	}
	else
	{
		if (Local_Var.find(src1) != Local_Var.end())
		{
			ASM_Output(mips_lw, reg_t2, shamt(Local_Var[src1].offset, reg_fp), mips_space);
		}
		else
		{
			ASM_Output(mips_la, reg_t0, src1, mips_space);
			ASM_Output(mips_lw, reg_t1, shamt(0, reg_t0), mips_space);
		}
	}

	if (isnumber(src2))
	{
		ASM_Output(mips_li, reg_t2, src2, mips_space);
	}
	else if (isreg(src2))
	{
		ASM_Output(mips_move, reg_t2, src2, mips_space);
	}
	else
	{
		if (Local_Var.find(src2) != Local_Var.end())
		{
			ASM_Output(mips_lw, reg_t1, shamt(Local_Var[src2].offset, reg_fp), mips_space);
		}
		else
		{
			ASM_Output(mips_la, reg_t0, src2, mips_space);
			ASM_Output(mips_lw, reg_t2, shamt(0, reg_t0), mips_space);
		}
	}

	ASM_Output(mips_mul, reg_t1, reg_t1, reg_t2);

	if (isreg(res))
	{
		ASM_Output(mips_move, res, reg_t2, mips_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output(mips_sw, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, res, mips_space);
		ASM_Output(mips_sw, reg_t1, shamt(0, reg_t0), mips_space);
	}
}

/**
a / b					< /, a, b, >
**/
void mips_divfun()
{
	if (isnumber(src1))
	{
		ASM_Output(mips_li, reg_t1, src1, mips_space);
	}
	else if (isreg(src1))
	{
		ASM_Output(mips_move, reg_t1, src1, mips_space);
	}
	else
	{
		if (Local_Var.find(src1) != Local_Var.end())
		{
			ASM_Output(mips_lw, reg_t2, shamt(Local_Var[src1].offset, reg_fp), mips_space);
		}
		else
		{
			ASM_Output(mips_la, reg_t0, src1, mips_space);
			ASM_Output(mips_lw, reg_t1, shamt(0, reg_t0), mips_space);
		}
	}

	if (isnumber(src2))
	{
		ASM_Output(mips_li, reg_t2, src2, mips_space);
	}
	else if (isreg(src2))
	{
		ASM_Output(mips_move, reg_t2, src2, mips_space);
	}
	else
	{
		if (Local_Var.find(src2) != Local_Var.end())
		{
			ASM_Output(mips_lw, reg_t1, shamt(Local_Var[src2].offset, reg_fp), mips_space);
		}
		else
		{
			ASM_Output(mips_la, reg_t0, src2, mips_space);
			ASM_Output(mips_lw, reg_t2, shamt(0, reg_t0), mips_space);
		}
	}

	ASM_Output(mips_div, reg_t1, reg_t1, reg_t2);
	ASM_Output("mfhi", reg_t1, mips_space, mips_space);

	if (isreg(res))
	{
		ASM_Output(mips_move, res, reg_t2, mips_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output(mips_sw, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, res, mips_space);
		ASM_Output(mips_sw, reg_t1, shamt(0, reg_t0), mips_space);
	}
}

/**
a = b					< =, b, , a >
**/
void mips_ass()
{
	if (isnumber(src1))
	{
		ASM_Output(mips_li, reg_t1, src1, mips_space);
	}
	else if (isreg(src1))
	{
		ASM_Output(mips_move, reg_t1, src1, mips_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t1, shamt(Local_Var[src1].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src1, mips_space);
		ASM_Output(mips_lw, reg_t1, shamt(0, reg_t0), mips_space);
	}

	if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output(mips_sw, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, res, mips_space);
		ASM_Output(mips_sw, reg_t1, shamt(0, reg_t0), mips_space);
	}
}

/**
a[i] = b;				< []=, b, i, a >
**/
void mips_assarry()
{
	if (isnumber(src1))
	{
		ASM_Output(mips_li, reg_t1, src1, mips_space);
	}
	else if (isreg(src1))
	{
		ASM_Output(mips_move, reg_t1, src1, mips_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t1, shamt(Local_Var[src1].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src1, mips_space);
		ASM_Output(mips_lw, reg_t1, shamt(0, reg_t0), mips_space);
	}


	if (isnumber(src2))
	{
		ASM_Output(mips_li, reg_t0, to_string(stoi(src2)*4), mips_space);
	}
	else if (isreg(src2))
	{
		ASM_Output(mips_move, reg_t2, src2, mips_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t2, shamt(Local_Var[src2].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src2, mips_space);
		ASM_Output(mips_lw, reg_t2, shamt(0, reg_t0), mips_space);
	}

	if (isreg(res))
	{
		std::cout << reg2var[res] << endl;
		if (Local_Var[reg2var[res]].type == "int")
		{
			ASM_Output(mips_mul, reg_t2, reg_t2, "4");
		}
		//else
		//{
		//}
		ASM_Output(mips_addi, reg_t0, reg_fp, to_string_asm(Local_Var[reg2var[res]].offset));
		ASM_Output(mips_sub, reg_t0, reg_t0, reg_t2);
		ASM_Output(mips_sw, reg_t1, shamt(0, reg_t0), mips_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		if (Local_Var[res].type == "int")
		{
			ASM_Output(mips_mul, reg_t2, reg_t2, "4");

		}
		//else
		//{(res+"("+reg_t0+")")
		//}
		//ASM_Output(mips_addi, reg_t0, reg_fp, mips_space);
		//ASM_Output(mips_sub, reg_t0, reg_t0, reg_t2);
		ASM_Output(mips_sw, reg_t1, res + shamt(0, reg_t0), mips_space);
	}
	else
	{
		if (Local_Var[res].type == "int")
		{
			ASM_Output(mips_mul, reg_t2, reg_t2, "4");
		}
		else
		{
			//ASM_Output(mips_la, reg_t0, res, mips_space);
			//ASM_Output(mips_sub, reg_t0, reg_t0, reg_t2);
			ASM_Output(mips_sw, reg_t1, res+shamt(0, reg_t0), mips_space);
		}
	}
}

/**
a = b[i];				< =[], i, b, a >
**/
void mips_arrassy() {
	
	ASM_Output(mips_li, reg_t0, to_string(stoi(src1) * 4), mips_space);//存偏移量
	ASM_Output(mips_lw, reg_t1, src2+'('+ reg_t0 +')', mips_space);//取出a[]
	ASM_Output(mips_sw, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_space);//存入b

}

/**
(int|char) fun();		< call, , , fun >
(void) fun();			< call, , , >
**/
void mips_call()
{
	ASM_Output(mips_subi, reg_sp, reg_sp, "4");// $sp = $sp - 4;
	ASM_Output(mips_sw, reg_ra, shamt(0, reg_sp), mips_space);// ($sp) = $ra
	ASM_Output(mips_subi, reg_sp, reg_sp, "4");// $sp = $sp - 4;
	ASM_Output(mips_sw, reg_fp, shamt(0, reg_sp), mips_space);// ($sp) = $fp
	ASM_Output(mips_move, reg_fp, reg_sp, mips_space);// $fp = $sp

	ASM_Output(mips_jal, src1, mips_space, mips_space);
	ASM_Output(mips_nop, mips_space, mips_space, mips_space);
	ASM_Output(mips_move, reg_sp, reg_fp, mips_space);// recover sp
	ASM_Output(mips_lw, reg_ra, shamt(4, reg_fp), mips_space);// recover ra
	ASM_Output(mips_lw, reg_fp, shamt(0, reg_fp), mips_space);// recover fp
	// store return Value
	if (isreg(res))
	{
		ASM_Output(mips_move, res, reg_v0, mips_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output(mips_sw, reg_v0, shamt(Local_Var[res].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, res, mips_space);
		ASM_Output(mips_sw, reg_v0, shamt(0, reg_t0), mips_space);
	}
}

/**
fun(a);					< callpara, int|char, , a >
**/
void mips_callpara()
{
	if (isnumber(res))
	{
		ASM_Output(mips_li, reg_t1, res, mips_space);
	}
	else if (isreg(res))
	{
		ASM_Output(mips_lw, reg_t1, res, mips_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t1, shamt(Local_Var[res].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, res, mips_space);
		ASM_Output(mips_lw, reg_t1, shamt(0, reg_t0), mips_space);
	}
	ASM_Output(mips_subi, reg_sp, reg_sp, "4");
	ASM_Output(mips_sw, reg_t1, shamt(0, reg_sp), mips_space);
}


//////////////////////////////////////////////////////////////

/**
set label				< lab, , , label >
**/
void mips_lab()
{
	ASM_Output(res + ":", mips_space, mips_space, mips_space);
}

/**
jmp	label		 		< j, , , label>
**/
void mips_jmp()
{
	ASM_Output(mips_j, res, mips_space, mips_space);
}

//////////////////////////////////////////////////////////////

/**
if(a < b)				< blt, a, b, label >
**/
void mips_bltfun()
{

	if (isnumber(src1))
	{
		ASM_Output(mips_li, reg_t8, src1, mips_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output(mips_move, reg_t8, src1, mips_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src1, mips_space);
		ASM_Output(mips_lw, reg_t8, shamt(0, reg_t0), mips_space);
	}

	if (isnumber(src2))
	{
		ASM_Output(mips_li, reg_t9, src2, mips_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output(mips_move, reg_t9, src2, mips_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src2, mips_space);
		ASM_Output(mips_lw, reg_t9, shamt(0, reg_t0), mips_space);
	}

	ASM_Output(mips_blt, reg_t8, reg_t9, res);
}

/**
if(a <= b)				< ble, a, b, label >
**/
void mips_blefun()
{
	if (isnumber(src1))
	{
		ASM_Output(mips_li, reg_t8, src1, mips_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output(mips_move, reg_t8, src1, mips_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src1, mips_space);
		ASM_Output(mips_lw, reg_t8, shamt(0, reg_t0), mips_space);
	}
	if (isnumber(src2))
	{
		ASM_Output(mips_li, reg_t9, src2, mips_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output(mips_move, reg_t9, src2, mips_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src2, mips_space);
		ASM_Output(mips_lw, reg_t9, shamt(0, reg_t0), mips_space);
	}
	ASM_Output(mips_ble, reg_t8, reg_t9, res);
}

/**
if(a > b)				< bgt, a, b, label >
**/
void mips_bgtfun()
{
	if (isnumber(src1))
	{
		ASM_Output(mips_li, reg_t8, src1, mips_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output(mips_move, reg_t8, src1, mips_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src1, mips_space);
		ASM_Output(mips_lw, reg_t8, shamt(0, reg_t0), mips_space);
	}
	if (isnumber(src2))
	{
		ASM_Output(mips_li, reg_t9, src2, mips_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output(mips_move, reg_t9, src2, mips_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src2, mips_space);
		ASM_Output(mips_lw, reg_t9, shamt(0, reg_t0), mips_space);
	}
	ASM_Output(mips_bgt, reg_t8, reg_t9, res);
}

/**
if(a >= b)				< bge, a, b, label >
**/
void mips_bgefun()
{
	if (isnumber(src1))
	{
		ASM_Output(mips_li, reg_t8, src1, mips_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output(mips_move, reg_t8, src1, mips_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src1, mips_space);
		ASM_Output(mips_lw, reg_t8, shamt(0, reg_t0), mips_space);
	}
	if (isnumber(src2))
	{
		ASM_Output(mips_li, reg_t9, src2, mips_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output(mips_move, reg_t9, src2, mips_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src2, mips_space);
		ASM_Output(mips_lw, reg_t9, shamt(0, reg_t0), mips_space);
	}
	ASM_Output(mips_bge, reg_t8, reg_t9, res);
}

/**
if(a == b)				< beq, a, b, label >
**/
void mips_bnefun()
{
	if (isnumber(src1))
	{
		ASM_Output(mips_li, reg_t8, src1, mips_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output(mips_move, reg_t8, src1, mips_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src1, mips_space);
		ASM_Output(mips_lw, reg_t8, shamt(0, reg_t0), mips_space);
	}
	if (isnumber(src2))
	{
		ASM_Output(mips_li, reg_t9, src2, mips_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output(mips_move, reg_t9, src2, mips_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src2, mips_space);
		ASM_Output(mips_lw, reg_t9, shamt(0, reg_t0), mips_space);
	}
	ASM_Output(mips_bne, reg_t8, reg_t9, res);
}

/**
if(a != b)				< bne, a, b, label >
**/
void mips_beqfun()
{
	if (isnumber(src1))
	{
		ASM_Output(mips_li, reg_t8, src1, mips_space);
	}
	else if (src1[0] == '$')
	{
		ASM_Output(mips_move, reg_t8, src1, mips_space);
	}
	else if (Local_Var.find(src1) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t8, shamt(Local_Var[src1].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src1, mips_space);
		ASM_Output(mips_lw, reg_t8, shamt(0, reg_t0), mips_space);
	}
	if (isnumber(src2))
	{
		ASM_Output(mips_li, reg_t9, src2, mips_space);
	}
	else if (src2[0] == '$')
	{
		ASM_Output(mips_move, reg_t9, src2, mips_space);
	}
	else if (Local_Var.find(src2) != Local_Var.end())
	{
		ASM_Output(mips_lw, reg_t9, shamt(Local_Var[src2].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_la, reg_t0, src2, mips_space);
		ASM_Output(mips_lw, reg_t9, shamt(0, reg_t0), mips_space);
	}
	ASM_Output(mips_beq, reg_t8, reg_t9, res);
}

//////////////////////////////////////////////////////////////

/**
scanf(a);				< scf, , , a >
**/
void mips_scf()
{
	if (Local_Var.find(res) != Local_Var.end())
	{
		if (Local_Var[res].type == "int")
		{
			ASM_Output(mips_li, reg_v0, scfint, mips_space);
		}
		else

		{
			ASM_Output(mips_li, reg_v0, scfchar, mips_space);
		}
	}
	else//glb
	{
		if (Global_Var[res].type == "int")
		{
			ASM_Output(mips_li, reg_v0, scfint, mips_space);
		}
		else
		{
			ASM_Output(mips_li, reg_v0, scfchar, mips_space);
		}
	}
	ASM_Output(mips_syscall, mips_space, mips_space, mips_space);//syscall
	if (isreg(res))
	{
		ASM_Output(mips_sw, reg_v0, res, mips_space);
	}
	else if (Local_Var.find(res) != Local_Var.end())
	{
		ASM_Output(mips_sw, reg_v0, shamt(Local_Var[res].offset, reg_fp), mips_space);
	}
	else
	{
		ASM_Output(mips_li, reg_v1, res, mips_space);
		ASM_Output(mips_sw, reg_v0, shamt(0, reg_v1), mips_space);
	}
	// 处理读入时的行末回车
	if (Local_Var.find(res) != Local_Var.end())
	{
		if (Local_Var[res].type == "int")
		{
		}
		else
		{
			ASM_Output(mips_li, reg_v0, scfchar, mips_space);
			ASM_Output(mips_syscall, mips_space, mips_space, mips_space);
		}
	}
	else
	{
		if (Global_Var[res].type == "int")
		{
		}
		else
		{
			ASM_Output(mips_li, reg_v0, scfchar, mips_space);
			ASM_Output(mips_syscall, mips_space, mips_space, mips_space);
		}
	}
}

/**
print("a",b);			< prf, a, b,  >
**/
void mips_prf()
{
	if (src1 == op_space)//src1==""
	{
	}
	else if (isstring(src1))// src1==string
	{
		ASM_Output(mips_la, reg_a0, String_List[src1], mips_space);
		ASM_Output(mips_li, reg_v0, prfstr, mips_space);
		ASM_Output(mips_syscall, mips_space, mips_space, mips_space);
	}
	else// src1==expr
	{
		if (isnumber(src1))
		{
			ASM_Output(mips_li, reg_a0, res, mips_space);
			ASM_Output(mips_li, reg_v0, prfint, mips_space);
		}
		else if (isreg(src1))
		{
			ASM_Output(mips_move, reg_a0, src1, mips_space);
		}
		else if (Local_Var.find(src1) != Local_Var.end())
		{
			ASM_Output(mips_lw, reg_a0, shamt(Local_Var[src1].offset, reg_fp), mips_space);
			if (Local_Var[src1].type == "int")
			{
				ASM_Output(mips_li, reg_v0, prfint, mips_space);
			}
			else
			{
				ASM_Output(mips_li, reg_v0, prfchar, mips_space);
			}
		}
		else
		{
			ASM_Output(mips_la, reg_t0, src1, mips_space);
			ASM_Output(mips_lw, reg_a0, shamt(0, reg_t0), mips_space);
			if (Global_Var[src1].type == "int")
			{
				ASM_Output(mips_li, reg_v0, prfint, mips_space);
			}
			else
			{
				ASM_Output(mips_li, reg_v0, prfchar, mips_space);
			}
		}
		ASM_Output(mips_syscall, mips_space, mips_space, mips_space);
	}
	//src2
	if (src2 != op_space)
	{
		if (isnumber(src2))
		{
			ASM_Output(mips_li, reg_a0, res, mips_space);
			ASM_Output(mips_li, reg_v0, prfint, mips_space);
		}
		else if (isreg(src2))
		{
			ASM_Output(mips_move, reg_a0, src2, mips_space);
		}
		else if (Local_Var.find(src2) != Local_Var.end())
		{
			ASM_Output(mips_lw, reg_a0, shamt(Local_Var[src2].offset, reg_fp), mips_space);
			if (Local_Var[src2].type == "int")
			{
				ASM_Output(mips_li, reg_v0, prfint, mips_space);
			}
			else
			{
				ASM_Output(mips_li, reg_v0, prfchar, mips_space);
			}
		}
		else
		{
			ASM_Output(mips_la, reg_t0, src2, mips_space);
			ASM_Output(mips_lw, reg_a0, shamt(0, reg_t0), mips_space);
			if (Global_Var[src2].type == "int")
			{
				ASM_Output(mips_li, reg_v0, prfint, mips_space);
			}
			else
			{
				ASM_Output(mips_li, reg_v0, prfchar, mips_space);
			}
		}
		ASM_Output(mips_syscall, mips_space, mips_space, mips_space);
	}
}

//////////////////////////////////////////////////////////////

void ASM_init()//初始化
{
	offset = 0;
	paranum = 0;
	preparanum = 0;
	valparanum = 0;
	nowfunc = "";
	Local_Var.clear();
	Global_Var.clear();
	Local_Var_Res.clear();
	newfunc = false;
}

void ASM_Output(string mips_op, string var1, string var2, string var3)//输出汇编语言
{
	if (mips_op[mips_op.length() - 1] == ':')//label or func
	{
		ASM << endl;
	}
	else if (istext)//text
	{
		ASM << "      ";
	}
	else//data
	{
		ASM << "      ";
	}

	if (var1 == mips_space && var2 == mips_space && var3 == mips_space)
	{
		ASM << mips_op;
		ASM << endl;
	}
	else if (var2 == mips_space && var3 == mips_space)
	{
		ASM << mips_op << "    " << var1;
		ASM << endl;
	}

	else if (mips_op == ".data:" || mips_op == ".text:")
	{
		ASM << mips_op;
		ASM << endl;
	}
	else if (var1 == ".ascii" || var1 == ".space" || var1 == ".word")
	{
		ASM << mips_op << "    " << var1 << "    " << var2 << endl;
	}
	else
	{
		ASM << mips_op << "    " << var1 << "," << var2;
		if (var3 != " ")
			ASM << "," << var3 << endl;
		else
			ASM << endl;
	}
}

void Data_Num()//数值型变量
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
				ASM_Output(res + ":", ".word", "0", mips_space);
			}
			else if (src1 == "char")
			{
				ASM_Output(res + ":", ".word", "0", mips_space);
			}
		}
		else if (op == op_int)
		{
			ASM_Output(res + ":", ".word", "0", mips_space);
		}
		else if (op == op_char)
		{
			ASM_Output(res + ":", ".word", "0", mips_space);
		}
		else if (op == op_inta)
		{
			ASM_Output(res + ":", ".space", to_string_asm(atoi(src1.c_str()) * intsize), mips_space);
		}
		else if (op == op_chara)
		{
			ASM_Output(res + ":", ".space", to_string_asm(atoi(src1.c_str()) * intsize), mips_space);
		}
	}

}
void Data_String()//字符串
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
		ASM_Output(iter->second + ":", ".ascii", tmp, mips_space);
		iter++;
	}
}

void Text_ASM()//生成MIPS的text段
{
	istext = true;
	bool flag = false;
	for (int i = 1; i <= IR_Num; i++)
	{

		string op = IR_List[i].op;
		src1 = IR_List[i].src1;
		src2 = IR_List[i].src2;
		res = IR_List[i].res;
		ASM << endl;
		if (!flag && (op == op_func || op == op_para))
		{
			ASM_Output(mips_j, "main", mips_space, mips_space);
			flag = true;
		}
		ASM << "#< ";
		ASM << setw(8) << left << op << ",";
		ASM << setw(8) << left << src1 << ",";
		ASM << setw(8) << left << src2 << ",";
		ASM << setw(8) << left << res;
		ASM << " >" << endl;
		if (op == op_int || (op==op_const && src1==op_int))
		{
			mips_int();
		}
		else if (op == op_char || (op == op_const && src1 == op_char))
		{
			mips_char();
		}
		else if (op == op_add)
		{
			mips_addfun();
		}
		else if (op == op_sub)
		{
			mips_subfun();
		}
		else if (op == op_mul)
		{
			mips_mulfun();
		}
		else if (op == op_div)
		{
			mips_divfun();
		}
		else if (op == op_ass)
		{
			mips_ass();
		}
		else if (op == op_para)
		{
			mips_para();
		}
		else if (op == op_func)
		{
			mips_func();
		}
		else if (op == op_call)
		{
			mips_call();
		}
		else if (op == op_ret)
		{
			mips_return();
		}
		else if (op == op_callpara)
		{
			mips_callpara();
		}
		else if (op == op_lab)
		{
			mips_lab();
		}
		else if (op == op_jmp)
		{
			mips_jmp();
		}
		else if (op == op_st)
		{
			mips_bltfun();
		}
		else if (op == op_set)
		{
			mips_blefun();
		}
		else if (op == op_bt)
		{
			mips_bgtfun();
		}
		else if (op == op_eql)
		{
			mips_beqfun();
		}
		else if (op == op_neq)
		{
			mips_bnefun();
		}
		else if (op == op_exits)
		{
			mips_exits();
		}
		else if (op == op_scanf)
		{
			mips_scf();
		}
		else if (op == op_printf)
		{
			mips_prf();
		}
		else if (op == op_assarry)
		{
			mips_assarry();
		}
		else if (op == op_arryass)
		{
			mips_arrassy();
		}
		else if (op == op_inta)
		{
			mips_intarry();
		}
		else if (op == op_chara)
		{
			mips_chararry();
		}
	}
}


void Asm_Code_main()//运行汇编语言生成
{

	ASM_init();

	ASM_Output(".data:", mips_space, mips_space, mips_space);
	
	Data_Num();//Num变量
	Data_String();//String变量

	ASM_Output(".text:", mips_space, mips_space, mips_space);
	
	Text_ASM();//Text变量
}

