/****************************************************/
/* 文件: Optim.cpp                                  */
/* 功能: 1.基本块划分                                */
/*       2.数据流分析                                */
/*       3.常数传播                                 */
/*       5.消除公共子表达式                          */
/*       6.窥孔优化                                 */
/****************************************************/

#include "head.h"
#include "IR.h"
#include "Asm_Code.h"
#include "Optim.h"

//#define Block cout;

ofstream Block("./Block_IR.txt");
ofstream Optim_IR("./Optim_IR.txt");

Quadruple Optim_IR_List_1[MAX_NUM];
Quadruple Optim_IR_List_2[MAX_NUM];

typedef struct
{
	int start;
	int end;
	int next1, next2;
	string label;
	string condition;
}BLOCK;


int regnum;
int blocknum;
bool istext_optimize;
bool regalldone;


vector<pair<int, int>> blockpos;
vector<string> blockvar[MAX_NUM];
vector<BLOCK> block;
map<string, int> label2Block;
ofstream Data_Stream;

string to_string_optimize(int n)
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


/////////////////////////////////////////////////

//判断是否是目标语句
bool istarget(int index)
{
	string op = IR_List[index].op;
	// label或function，转移语句的目标语句
	if (op == op_lab || op == op_func)
	{
		return true;
	}
	return false;
}

//判断是否是转移语句
bool istransfer(int index)
{
	string op = IR_List[index].op;
	// 条件转移语句
	if (op == op_eql || op == op_bt || op == op_neq || op == op_bet || op == op_st || op == op_set || op == op_jmp || op == op_call || op == op_ret || op == op_exits)
	{
		return true;
	}
	return false;
}

//判断是否是基本块入口
bool isblockenterance(int index)
{
	// label或function，转移语句的目标语句
	if (istarget(index))
	{
		return true;
	}
	// 上一条是条件转移语句
	else if (istransfer(index - 1))
	{
		return true;
	}
	return false;
}

//判断是否是基本块出口
bool isblockexit(int index)
{
	if (isblockenterance(index + 1))// 下一条是入口
	{
		return true;
	}
	return false;
}

//划分基本块
void Divide_Block()
{
	string op, var1, var2, var3;
	int blockstartpos = 0; 
	bool isfirst = true;
	bool isdata = true;

	BLOCK tmp;
	string label = "";

	for (int i = 1; i <= IR_Num; i++)
	{
		op = IR_List[i].op;
		var1 = IR_List[i].src1;
		var2 = IR_List[i].src2;
		var3 = IR_List[i].res;
		
		//切分的第一部分为全局变量和常量部分
		//跳过data部分
		if (isdata)
		{
			if (op == op_func || op == op_para)
			{
				isdata = false;
				blockpos.push_back(make_pair(blockstartpos, i - 1));
				tmp.start = blockstartpos;
				tmp.end = i - 1;
				tmp.next1 = -1;
				tmp.next2 = -1;
				tmp.label = label;
				block.push_back(tmp);
				label2Block[label] = blocknum;
				blockstartpos = i;
			}
			else
			{
				IR_List[i].Which_Block = blocknum;
				continue;
			}
		}

		// 程序第一条语句
		if (isfirst)
		{
			blocknum++;
			IR_List[i].isentrance = true;
			isfirst = false;
			blockstartpos = i;
			label = "main";
		}
		else if (isblockenterance(i))// 基本块入口
		{
			blocknum++;
			IR_List[i].isentrance = true;
			blockstartpos = i;
			label = var3;
		}

		if (isblockexit(i))
		{
			IR_List[i].isexit = true;
			blockpos.push_back(make_pair(blockstartpos, i));
			tmp.start = blockstartpos;
			tmp.end = i;
			tmp.next1 = -1;
			tmp.next2 = -1;
			tmp.label = label;
			block.push_back(tmp);
			label2Block[label] = blocknum;
			blockstartpos = i + 1;
		}
		IR_List[i].Which_Block = blocknum;
	}
}


/////////////////////////////////////////////////

void optimize_local()
{
	int k = 0;
	for (int t = 0; t < blocknum; t++)
	{
		for (int i = 1; i <= IR_Num; i++)
		{
			for (int j = i + 1; j<= IR_Num; j++)
			{
				if (IR_List[i].Which_Block == IR_List[j].Which_Block)
				{
					if (IR_List[i].op == IR_List[j].op && IR_List[i].res == IR_List[j].res &&
						IR_List[i].src1 == IR_List[j].src1 && IR_List[i].src2 == IR_List[j].src2
						&& IR_List[i].op != "scanf" && IR_List[i].op != "printf")
					{
						IR_List[j].IsRepeat = 1;
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////

void optimize_peephole()
{
	int k = 0;
	int flag;
	for (int i = 1; i <= IR_Num; i++)
	{
		flag = 0;
		string op, var1, var2, var3;
		int block, isrepeat;
		op = IR_List[i].op;
		var1 = IR_List[i].src1;
		var2 = IR_List[i].src2;
		var3 = IR_List[i].res;
		if (op==op_int || op==op_char || op==op_inta || op==op_chara)
		{
			for (int j = i + 1; j <= IR_Num; j++)
			{
				if (var3 == IR_List[j].res || var3 == IR_List[j].src1 || var3 == IR_List[j].src2)
				{
					flag = 1;
				}
			}
			if (flag == 0)
			{
				IR_List[i].IsRepeat = 1;
			}	
		}

		
		}
	}

/////////////////////////////////////////////////

void optimize_ConstantPro()
{
	vector<pair<string,string>> const_var;
	int k = 0;
	//构建一个从const list
	for (int i = 1; i <= IR_Num; i++)
	{
		string op, var1, var2, var3;
		int block, isrepeat;
		op = IR_List[i].op;
		var1 = IR_List[i].src1;
		var2 = IR_List[i].src2;
		var3 = IR_List[i].res;
		block = IR_List[i].Which_Block;
		isrepeat = IR_List[i].IsRepeat;
		if (op == op_const)
		{
			string num = IR_List[i + 1].src1;
			const_var.push_back(make_pair(var3,num)); // 向容器末尾添加一个元素    
		}
	}

	//将对应的数字进行替换
	vector<pair<string, string>>::iterator it = const_var.begin();
	for (; it != const_var.end(); ++it)
	{
		for (int i = 1; i <= IR_Num; i++)
		{
			string op, var1, var2, var3;
			var1 = IR_List[i].src1;
			var2 = IR_List[i].src2;

			if (var1 == (*it).first)
			{
				IR_List[i].src1 = (*it).second;
			}
			if (var2 == (*it).first)
			{
				IR_List[i].src2 = (*it).second;
			}

		}
	}
}


/////////////////////////////////////////////////

//void optimize_out(string op, string var1, string var2, string var3)//优化输出
//{
//	optimize << "\n#< ";
//	optimize << setw(varwidth) << left << op << ",";
//	optimize << setw(varwidth) << left << var1 << ",";
//	optimize << setw(varwidth) << left << var2 << ",";
//	optimize << setw(varwidth) << left << var3;
//	optimize << " >" << endl;
//}

void Block_Output(string op, string src1, string src2, string res, int block)//生成四元式
{
	std::cout << "< ";
	std::cout << setw(5) << left << op << ", ";
	std::cout << setw(5) << left << src1 << ", ";
	std::cout << setw(5) << left << src2 << ", ";
	std::cout << setw(5) << left << res;
	std::cout << " >";
	std::cout << setw(5) << ("  ---Block: ") << block;
	std::cout << endl;
}

void IR_Optim_Output(string op, string src1, string src2, string res, int block, int isrepeat)//生成四元式
{
	if (isrepeat!=1)
	{
		Optim_IR << "< ";
		Optim_IR << setw(5) << left << op << ", ";
		Optim_IR << setw(5) << left << src1 << ", ";
		Optim_IR << setw(5) << left << src2 << ", ";
		Optim_IR << setw(5) << left << res;
		Optim_IR << " >";
		//Optim_IR << endl;
		Optim_IR << setw(5) << ("  ---Block: ") << block;
		Optim_IR << endl;
	}
	
}


void optimize_datastream()//数据流分析
{
	string op, var1, var2, var3;
	Data_Stream.open("Data_Stream.txt", ios::out);
	Data_Stream << setw(7) << left << "BLOCK" << setw(12) << left << "condition" << setw(7) << left << "TRUE" << setw(7) << left << "FALSE" << endl;
	for (int i = 1; i <= blocknum; i++)
	{
		op = IR_List[block[i].end].op;
		var1 = IR_List[block[i].end].src1;
		var2 = IR_List[block[i].end].src2;
		var3 = IR_List[block[i].end].res;

		if (op == op_jmp)
		{
			block[i].condition = op;
			block[i].next1 = label2Block[var3];
		}
		else if (op == op_call)
		{
			block[i].condition = op;
			block[i].next1 = label2Block[var1];
		}
		else if (op == op_exits)
		{
			block[i].condition = op;
			block[i].next1 = 0;
		}
		else if (op == op_eql || op == op_neq || op == op_bt || op == op_bet || op == op_st || op == op_set)
		{
			block[i].condition = var1 + op + var2;
			block[i].next1 = label2Block[var3];
			block[i].next2 = i + 1;
		}
		else
		{
			block[i].condition = "TRUE";
			block[i].next1 = i + 1;
		}
		Data_Stream << setw(7) << left << i << setw(12) << left << block[i].condition << setw(7) << left << block[i].next1 << setw(12) << left << block[i].next2 << endl;
	}
	Data_Stream.close();
}

void Optimize_main()//优化运行
{
	//state = opt_sta;
	//optimize_init();
	blocknum = 0; //初始化设置块的编号为0
	istext_optimize = false;
	blockpos.clear();
	block.clear();
	regalldone = false;

	Divide_Block();

	int k = 1;
	
	optimize_ConstantPro();  //1.常数传播

	optimize_local();  //2.消除局部子表达式

	for (int i = 1; i <= IR_Num; i++)
	{
		string op, var1, var2, var3;
		int block, isrepeat;
		op = IR_List[i].op;
		var1 = IR_List[i].src1;
		var2 = IR_List[i].src2;
		var3 = IR_List[i].res;
		block = IR_List[i].Which_Block;
		isrepeat = IR_List[i].IsRepeat;
		if (isrepeat != 1)
		{
			Optim_IR_List_1[k] = IR_List[i];
			k++;
		}
		//IR_Optim_Output(op, var1, var2, var3, block,isrepeat);
	}
	for (int n = 0; n < MAX_NUM; n++)
	{
		IR_List[n] = Optim_IR_List_1[n];
	}
	//memset(Optim_IR_List_1, 0, sizeof Optim_IR_List_1);

	optimize_peephole();  //3.窥孔优化,删除死代码
	
	k = 1;
	for (int i = 1; i <= IR_Num; i++)
	{
		string op, var1, var2, var3;
		int block, isrepeat;
		op = IR_List[i].op;
		var1 = IR_List[i].src1;
		var2 = IR_List[i].src2;
		var3 = IR_List[i].res;
		block = IR_List[i].Which_Block;
		isrepeat = IR_List[i].IsRepeat;
		if (isrepeat != 1)
		{
			Optim_IR_List_2[k] = IR_List[i];
			k++;
		}
		IR_Optim_Output(op, var1, var2, var3, block, isrepeat);
	}
	for (int n = 0; n < MAX_NUM; n++)
	{
		IR_List[n] = Optim_IR_List_2[n];
	}

	optimize_datastream(); //数据流分析

	return;
}