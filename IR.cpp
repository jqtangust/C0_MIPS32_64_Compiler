/****************************************************/
/* 文件: IR.cpp                                     */
/* 功能: 1.基本块划分                                */
/*       2.数据流分析                                */
/*       3.常数传播                                 */
/*       5.消除公共子表达式                          */
/*       6.窥孔优化                                 */
/****************************************************/

#include "IR.h"
//#define debug cout 

using namespace std;

ofstream IR("./IR.txt");
ofstream debug("./Debug.txt");


int varnum = 0;
int labelnum = 0;
int stringnum=0;

int IR_Num = 0;//四元式数量
Quadruple IR_List[MAX_NUM];
map<string, string>stringtable;

int varwidth = 8;

string to_string_mid(int n)
{
	int m = n;
	char s[1000];
	char ss[1000];
	int i = 0, j = 0;
	if (n < 0)// 处理负数
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

void IR_Output(string op, string src1, string src2, string res)//生成四元式
{
	IR << "< ";
	IR << setw(varwidth) << left << op << ", ";
	IR << setw(varwidth) << left << src1 << ", ";
	IR << setw(varwidth) << left << src2 << ", ";
	IR << setw(varwidth) << left << res;
	IR << " >" << endl;

	debug << "< ";
	debug << setw(varwidth) << left << op << ", ";
	debug << setw(varwidth) << left << src1 << ", ";
	debug << setw(varwidth) << left << src2 << ", ";
	debug << setw(varwidth) << left << res;
	debug << " >" << endl;
	++IR_Num;
	IR_List[IR_Num].op = op;
	IR_List[IR_Num].src1 = src1;
	IR_List[IR_Num].src2 = src2;
	IR_List[IR_Num].res = res;
	if (src1 == op_space || src2 == op_space || res == op_space)
	{
		IR_List[IR_Num].islableorfunc = true;
	}
}
string genvar()//生成临时变量
{
	string var = "Var_" + to_string_mid(varnum);
	varnum++;
	string op, src1, src2, res;
	op = op_int;
	src1 = op_space;
	src2 = op_space;
	res = var;
	IR << "< ";
	IR << setw(varwidth) << left << op << ", ";
	IR << setw(varwidth) << left << src1 << ", ";
	IR << setw(varwidth) << left << src2 << ", ";
	IR << setw(varwidth) << left << res;
	IR << " >" << endl;

	debug << "< ";
	debug << setw(varwidth) << left << op << ", ";
	debug << setw(varwidth) << left << src1 << ", ";
	debug << setw(varwidth) << left << src2 << ", ";
	debug << setw(varwidth) << left << res;
	debug << " >" << endl;
	++IR_Num;
	IR_List[IR_Num].op = op;
	IR_List[IR_Num].src1 = src1;
	IR_List[IR_Num].src2 = src2;
	IR_List[IR_Num].res = res;
	return var;
}
string genlable()//生成Function
{
	string label = "Function_" + to_string_mid(labelnum);
	labelnum++;
	return label;
}
string genstring()//生成全局字符串
{
	string strvar = "String_" + to_string_mid(stringnum);
	stringnum++;
	return strvar;
}