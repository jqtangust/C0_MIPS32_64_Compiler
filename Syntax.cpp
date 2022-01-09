/****************************************************/
/* 文件: Optim.cpp                                  */
/* 功能: 1.基本块划分                                */
/*       2.数据流分析                                */
/*       3.常数传播                                 */
/*       5.消除公共子表达式                          */
/*       6.窥孔优化                                 */
/****************************************************/

#include "head.h"

#include "Syntax.h"
#include "Symbol_Table.h"
#include "Error.h"
#include "IR.h"
#include "Asm_Code.h"

#define cout Syntax_out

using namespace std;

ofstream Syntax_out("./Syntax.txt");

map<string, bool> SYN_Const_Var;
map<string, bool> SYN_Local_Var;
map<string, bool> SYN_Global_Var;

map<string, bool> Return_Func;
map<string, bool> No_Return_Func;

map<string, string> String_List;

string glbtemp;
string functionname = "";
int syn_number = Lex_Table.size();
int syn_indexInlex_symbolTable = 0;
int syn_preindexInlex_symbolTable = 0;
string symbol = "";
string presymbol = "";
int syn_ID = 0;
int syn_preID = 0;

int syn_errorline = -1;
int syn_preerrorline = -1;

int syn_errorcolon = -1;
int syn_preerrorcolon = -1;

string syn_keywordID[keywordNum] =
{
	"lex_error_id","lex_int_id","lex_char_id","lex_float_id","lex_double_id","lex_bool_id",
	"lex_void_id","lex_const_id","lex_if_id","lex_else_id","lex_for_id","lex_while_id",
	"lex_do_id","lex_switch_id","lex_case_id","lex_default_id","lex_printf_id","lex_scanf_id",
	"lex_return_id","lex_main_id","lex_indentifer","lex_add_id","lex_subtract_id","lex_multiply_id",
	"lex_divide_id","lex_percent_id","lex_less_id","lex_greater_id","lex_lessequal_id","lex_greaterequal_id",
	"lex_equalequal_id","lex_notequal_id","lex_equal_id","lex_comma_id","lex_semicolon_id","lex_colon_id",
	"lex_dot_id","lex_singlemark_id","lex_doublemark_id","lex_leftparenthesis_id","lex_rightparenthesis_id",
	"lex_leftmiddle_id","lex_rightmiddle_id","lex_leftbraces_id","lex_rightbraces_id","lex_NotZeroHeadNum_id",
	"lex_string_id","lex_character_id","lex_letter_id","lex_ZeroToNine","lex_Zero_id","lex_OneToNine_id",
	"lex_ZeroHeadNum_id","lex_struct_id"
};//关键字ID


void syn_nextsymbol()//读下一个
{
	syn_ID = Lex_Table[syn_indexInlex_symbolTable].lex_symbol_ID;
	symbol = Lex_Table[syn_indexInlex_symbolTable].lex_symbol;
	syn_errorline = Lex_Table[syn_indexInlex_symbolTable].lex_indexOfFile;
	syn_errorcolon = Lex_Table[syn_indexInlex_symbolTable].lex_indexOfLine;
	syn_indexInlex_symbolTable++;
	cout << setw(25) << left << syn_keywordID[syn_ID];
	cout << setw(10) << left << syn_ID << symbol << endl;
}

void syn_read_ahead()//预读一个symbol
{
	cout << "***[readAhead]***" << "   ";
	syn_nextsymbol();
}
void syn_record()//记录当前位置
{
	syn_preID = syn_ID;
	syn_preindexInlex_symbolTable = syn_indexInlex_symbolTable;
	syn_preerrorline = syn_errorline;
	syn_preerrorcolon = syn_errorcolon;
	presymbol = symbol;
}
void syn_rollback()//回退到之前的位置
{
	cout << "***[rollBack]***" << endl;
	syn_ID = syn_preID;
	syn_indexInlex_symbolTable = syn_preindexInlex_symbolTable;
	syn_errorline = syn_preerrorline;
	syn_errorcolon = syn_preerrorcolon;
	symbol = presymbol;
}

/**
	入口程序决定了全部的程序流程，是分析的入口参数
	＜程序＞ ::= ［＜常量说明＞］［＜变量说明＞］ {＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
**/
void Syntax_Analysis()
{
	cout << "*语法分析：程序开始" << endl;
	if (syn_ID == lex_const_id)//常量说明
	{
		syn_const_dec();
	}
	if (syn_ID == lex_int_id || syn_ID == lex_char_id)//变量说明
	{
		syn_record();
		syn_read_ahead();
		syn_read_ahead();
		if (syn_ID != lex_leftparenthesis_id)
		{
			syn_rollback();
			syn_var_dec();
		}
		else
		{
			syn_rollback();
		}
	}
	//{＜有返回值函数定义＞|＜无返回值函数定义＞}
	syn_record();
	syn_read_ahead();
	int remsynID = syn_ID;
	syn_read_ahead();
	while (remsynID != lex_main_id && syn_ID == lex_leftparenthesis_id)
	{
		syn_rollback();
		if (syn_ID == lex_int_id || syn_ID == lex_char_id)
		{
			syn_functionwithreturn();
		}
		else
		{
			syn_functionwithoutreturn();
		}
		syn_record();
		syn_read_ahead();
		remsynID = syn_ID;
		syn_read_ahead();
	}
	syn_rollback();
	// ＜主函数＞
	syn_record();
	syn_read_ahead();
	if (syn_ID != lex_main_id)
	{
		Grammar_Error(err_main, syn_errorline, syn_errorcolon);
		return;
	}
	else
	{
		syn_rollback();
		syn_main();
	}
	cout << "*语法分析：程序结束" << endl;
}

void Syntax_Analysis_main()
{
	SYN_Const_Var.clear();
	SYN_Local_Var.clear();
	SYN_Global_Var.clear();
	//state = syn_sta;
	//syn_init();
	syn_nextsymbol();
	Syntax_Analysis();
}

/**
	＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}
**/
void syn_const_dec()//常量说明
{
	cout << "*语法分析：常量说明开始" << endl;
	if (syn_ID != lex_const_id)//检查是不是const
	{
		Grammar_Error(err_const, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_const_def();
	syn_nextsymbol();
	while (syn_ID == lex_const_id)
	{
		syn_nextsymbol();
		syn_const_def();//进常量定义
		syn_nextsymbol();
	}
	cout << "*语法分析：常量说明结束" << endl;
}
/**
	＜常量定义＞   ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
						  | float＜标识符＞＝＜实数＞{,＜标识符＞＝＜实数＞}
						  | char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
**/
void syn_const_def()//常量定义
{
	cout << "*语法分析：常量定义开始" << endl;
	string op, s1, s2, res;
	string type;
	op = op_const;
	if (syn_ID != lex_int_id && syn_ID != lex_float_id && syn_ID != lex_char_id)
	{
		Grammar_Error(err_intorchar, syn_errorline, syn_errorcolon);
		return;
	}
	else//if(symID == INT_SYMID)///int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
	{
		if (syn_ID == lex_int_id)
		{
			type = op_int;
		}
		else///char
		{
			type = op_char;
		}
		syn_nextsymbol();
		if (SYN_Const_Var.find(symbol) == SYN_Const_Var.end())
		{
			SYN_Const_Var[symbol] = true;
		}
		else
		{
			Grammar_Error(err_const_redefination, syn_errorline, syn_errorcolon);
			return;
		}
		res = symbol;
		syn_identifier();
		if (syn_ID != lex_equal_id)
		{
			Grammar_Error(err_assign, syn_errorline, syn_errorcolon);
			return;
		}
		op = op_const;
		s1 = type;
		s2 = op_space;
		IR_Output(op, s1, s2, res);
		syn_nextsymbol();
		if (syn_ID == lex_subtract_id)
		{
			op = op_sub;
			s1 = op_zero;
			syn_nextsymbol();
			s2 = symbol;
			IR_Output(op, s1, s2, res);
		}
		else
		{
			op = op_ass;
			s1 = symbol;
			s2 = op_space;
			IR_Output(op, s1, s2, res);
		}
		syn_nextsymbol();
		while (syn_ID == lex_comma_id)//多个在一行定义
		{
			syn_nextsymbol();
			if (SYN_Const_Var.find(symbol) == SYN_Const_Var.end())
			{
				SYN_Const_Var[symbol] = true;
			}
			else
			{
				Grammar_Error(err_const_redefination, syn_errorline, syn_errorcolon);
				return;
			}
			res = symbol;
			syn_identifier();
			if (syn_ID != lex_equal_id)
			{
				Grammar_Error(err_assign, syn_errorline, syn_errorcolon);
				return;
			}
			op = op_const;
			s1 = type;
			s2 = op_space;
			IR_Output(op, s1, s2, res);
			syn_nextsymbol();
			if (syn_ID == lex_subtract_id)
			{
				op = op_sub;
				s1 = op_zero;
				syn_nextsymbol();
				s2 = symbol;
				IR_Output(op, s1, s2, res);
			}
			else
			{
				op = op_ass;
				s1 = symbol;
				s2 = op_space;
				IR_Output(op, s1, s2, res);
			}
			syn_nextsymbol();
		}
	}
	cout << "*语法分析：常量定义结束" << endl;
}
/**
	＜声明头部＞   ::=  int＜标识符＞ |float ＜标识符＞|char＜标识符＞
**/
string funcname;//记录有返回值的函数名
string functype;//记录有返回值的函数类型
void syn_headFile()//头部声明
{
	cout << "*语法分析：头部声明开始" << endl;
	if (syn_ID == lex_int_id || syn_ID == lex_char_id || syn_ID == lex_float_id)
	{
		if (syn_ID == lex_int_id)
		{
			functype = op_int;
		}
		else
		{
			functype = op_char;
		}
		syn_nextsymbol();
		if (Return_Func.find(symbol) == Return_Func.end())//检查函数是否已经定义过
		{
			//如果还没定义就记录下来
			Return_Func[symbol] = true;
			functionname = symbol;
		}
		else
		{
			//如果重复定义了就报错
			Grammar_Error(err_intorchar, syn_errorline, syn_errorcolon);
			return;
		}
		funcname = symbol;
		syn_identifier();
	}
	else
	{
		Grammar_Error(err_intorchar, syn_errorline, syn_errorcolon);
		return;
	}
	cout << "*语法分析：头部声明结束" << endl;
}
/**
	＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
**/
void syn_var_dec()//变量说明
{
	cout << "*语法分析：变量声明开始" << endl;
	syn_var_def();//进变量定义
	if (syn_ID != lex_semicolon_id)
	{
		Grammar_Error(err_lacksemicolon, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	while (syn_ID == lex_int_id || syn_ID == lex_char_id)
	{
		syn_record();//记录当前的symbol
		syn_read_ahead();//预读一个symbol
		syn_read_ahead();//再预读一个symbol
		if (syn_ID == lex_leftparenthesis_id)//如果读到（说明是函数定义，先回退，然后结束变量说明
		{
			syn_rollback();
			break;
		}
		//否则先回退，继续进行变量定义
		syn_rollback();
		syn_var_def();
		if (syn_ID != lex_semicolon_id)
		{
			Grammar_Error(err_lacksemicolon, syn_errorline, syn_errorcolon);//不是；报错
			return;
		}
		syn_nextsymbol();
	}
	cout << "*语法分析：变量声明结束" << endl;
}
/**
	＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’){,(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’ )}
**/
void syn_var_def()//变量定义
{
	cout << "*语法分析：变量定义开始" << endl;
	string op, s1, s2, res;//四元式
	int remsymID = syn_ID;//记录变量类型
	syn_typeidentifier();
	//查找变量名是否已经定义过
	if (functionname == "")//全局变量定义
	{
		//如果常量或全局变量已经定义过，报错
		if (SYN_Const_Var.find(symbol) != SYN_Const_Var.end() || SYN_Global_Var.find(symbol) != SYN_Global_Var.end())
		{
			Grammar_Error(err_var_redefination, syn_errorline, syn_errorcolon);
			return;
		}
		else//如果没有定义过，则记录下来
		{
			SYN_Global_Var[symbol] = true;
		}
	}
	else//局部变量定义
	{
		//如果局部变量已经定义过，则报错
		if (SYN_Local_Var.find(symbol) != SYN_Local_Var.end())
		{
			Grammar_Error(err_var_redefination, syn_errorline, syn_errorcolon);
			return;
		}
		//如果未定义则记录下来
		else
		{
			SYN_Local_Var[symbol] = true;
		}
	}
	res = symbol;
	syn_identifier();
	if (syn_ID == lex_leftmiddle_id)//如果读到[则说明是数组
	{
		if (remsymID == lex_int_id)
		{
			op = op_inta;
		}
		else
		{
			op = op_chara;
		}
		syn_nextsymbol();
		s1 = symbol;
		syn_noSignInt();
		if (syn_ID == lex_rightmiddle_id)
		{
			syn_nextsymbol();
		}
		else
		{
			Grammar_Error(err_rightmiddle, syn_errorline, syn_errorcolon);
			return;
		}
		s2 = op_space;
		IR_Output(op, s1, s2, res);
	}
	else//否则不是数组
	{
		if (remsymID == lex_int_id)
		{
			op = op_int;
		}
		else
		{
			op = op_char;
		}
		s1 = op_space;
		s2 = op_space;
		IR_Output(op, s1, s2, res);
	}
	while (syn_ID == lex_comma_id)//如果是，则重复上述操作，继续定义
	{
		syn_nextsymbol();
		if (functionname == "")
		{
			if (SYN_Const_Var.find(symbol) != SYN_Const_Var.end() || SYN_Global_Var.find(symbol) != SYN_Global_Var.end())
			{
				Grammar_Error(err_var_redefination, syn_errorline, syn_errorcolon);
				return;
			}
			else
			{
				SYN_Global_Var[symbol] = true;
			}
		}
		else
		{
			if (SYN_Local_Var.find(symbol) != SYN_Local_Var.end())
			{
				Grammar_Error(err_var_redefination, syn_errorline, syn_errorcolon);
				return;
			}
			else
			{
				SYN_Local_Var[symbol] = true;
			}
		}
		res = symbol;
		syn_identifier();
		if (syn_ID == lex_leftmiddle_id)
		{
			if (remsymID == lex_int_id)
			{
				op = op_inta;
			}
			else
			{
				op = op_chara;
			}
			syn_nextsymbol();
			s1 = symbol;
			syn_noSignInt();
			if (syn_ID == lex_rightmiddle_id)
			{
				syn_nextsymbol();
			}
			else
			{
				Grammar_Error(err_rightmiddle, syn_errorline, syn_errorcolon);
				return;
			}
			s2 = op_space;
			IR_Output(op, s1, s2, res);
		}
		else
		{
			if (remsymID == lex_int_id)
			{
				op = op_int;
			}
			else
			{
				op = op_char;
			}
			s1 = op_space;
			s2 = op_space;
			IR_Output(op, s1, s2, res);
		}
	}
	cout << "*语法分析：变量定义结束" << endl;
}
/**
	＜参数＞    ::= ＜参数表＞
**/
void syn_parameter()//参数
{
	cout << "*语法分析：参数开始" << endl;
	syn_parameter_table();//进参数表
	cout << "*语法分析：参数结束" << endl;
}
/**
	＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
**/
void syn_parameter_table()//参数表
{
	cout << "*语法分析：参数表开始" << endl;
	string op, s1, s2, res;//四元式
	op = op_para;
	s1 = symbol;
	syn_typeidentifier();//进类型标识符
	res = symbol;
	if (SYN_Local_Var.find(symbol) != SYN_Local_Var.end())//检查局部变量是否定义过
	{
		Grammar_Error(err_var_redefination, syn_errorline, syn_errorcolon);
		return;
	}
	else
	{
		SYN_Local_Var[res] = true;
	}
	syn_identifier();//进标识符
	s2 = op_space;
	IR_Output(op, s1, s2, res);//生成四元式
	while (syn_ID == lex_comma_id)//如果是，说明还有参数，重复上述操作
	{
		syn_nextsymbol();
		op = op_para;
		s1 = symbol;
		syn_typeidentifier();
		res = symbol;
		if (SYN_Local_Var.find(symbol) != SYN_Local_Var.end())
		{
			Grammar_Error(err_var_redefination, syn_errorline, syn_errorcolon);
			return;
		}
		else
		{
			SYN_Local_Var[res] = true;
		}
		syn_identifier();
		s2 = op_space;
		IR_Output(op, s1, s2, res);
	}
	cout << "*语法分析：参数表结束" << endl;
}
/**
	＜有返回值函数定义＞  ::=  ＜声明头部＞‘(’＜参数＞‘)’ ‘{’＜复合语句＞‘}’
**/
void syn_functionwithreturn()//有返回值函数定义
{
	cout << "*语法分析：有返回值函数定义开始" << endl;
	SYN_Local_Var.clear();
	string op, s1, s2, res;//四元式
	syn_headFile();//直接进头部声明
	op = op_func;
	s1 = functype;
	s2 = op_space;
	res = funcname;
	//syn_functionwithreturnname[funcname]=true;
	if (syn_ID != lex_leftparenthesis_id)//（
	{
		Grammar_Error(err_leftparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if(syn_ID != lex_rightparenthesis_id)
	{
		//syn_rollback();
		syn_parameter();//进参数
		IR_Output(op, s1, s2, res);
	}
	else
	{
		//空语句，无参函数
		IR_Output(op, s1, s2, res);
	}
	if (syn_ID != lex_rightparenthesis_id)//）
	{
		Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID != lex_leftbraces_id)//{
	{
		Grammar_Error(err_leftbraces, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_mixsentence();//进复合语句
	if (syn_ID != lex_rightbraces_id)//}
	{
		Grammar_Error(err_rightbraces, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	//getMidCode(op,s1,s2,res);
	IR_Output(op_exits, op_space, op_space, res);
	cout << "*语法分析：有返回值函数定义结束" << endl;
}
/**
	＜无返回值函数定义＞  ::= void＜标识符＞‘(’＜参数＞‘)’‘{’＜复合语句＞‘}’
**/
void syn_functionwithoutreturn()//无返回值函数定义
{
	cout << "*语法分析：无返回值函数定义开始" << endl;
	SYN_Local_Var.clear();
	string op, s1, s2, res;
	if (syn_ID != lex_void_id)
	{
		Grammar_Error(err_void, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (No_Return_Func.find(symbol) == No_Return_Func.end())
	{
		No_Return_Func[symbol] = true;
		functionname = symbol;
	}
	else
	{
		Grammar_Error(err_function_redefination, syn_errorline, syn_errorcolon);
		return;
	}
	op = op_func;
	s1 = op_space;
	s2 = op_space;
	res = symbol;
	//syn_identifier();
	syn_nextsymbol();
	if (syn_ID != lex_leftparenthesis_id)//(
	{
		Grammar_Error(err_leftparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID != lex_rightparenthesis_id)
	{
		//syn_rollback();
		syn_parameter();//进参数
		IR_Output(op, s1, s2, res);
	}
	else
	{
		//空语句，无参函数
		IR_Output(op, s1, s2, res);
	}
	if (syn_ID != lex_rightparenthesis_id)
	{
		Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID != lex_leftbraces_id)
	{
		Grammar_Error(err_leftbraces, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_mixsentence();

	if (syn_ID != lex_rightbraces_id)
	{
		Grammar_Error(err_rightbraces, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	IR_Output(op_ret, op_space, op_space, op_space);
	IR_Output(op_exits, op_space, op_space, res);
	cout << "*语法分析：无返回值函数定义结束" << endl;
}
/**
	＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
**/

void syn_mixsentence()//复合语句
{
	cout << "*语法分析：复合语句开始" << endl;
	if (syn_ID == lex_const_id)//如果读到const，则进入常量声明
	{
		syn_const_dec();
	}
	if (syn_ID == lex_int_id || syn_ID == lex_float_id || syn_ID == lex_char_id)//如果读到int，float，char进变量声明
	{
		syn_var_dec();
	}
	syn_sentence_colomn();//进语句列
	cout << "*语法分析：复合语句结束" << endl;
}
/**
	＜主函数＞    ::= void main‘(’‘)’‘{’＜复合语句＞‘}’
**/
void syn_main()//主函数
{
	cout << "*语法分析：主函数开始" << endl;
	SYN_Local_Var.clear();
	string op, s1, s2, res;//四元式
	if (syn_ID != lex_void_id)
	{
		Grammar_Error(err_void, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID != lex_main_id)//main
	{
		Grammar_Error(err_main, syn_errorline, syn_errorcolon);
		return;
	}
	string remsymbol = symbol;
	op = op_func;
	s1 = op_space;
	s2 = op_space;
	res = symbol;
	IR_Output(op, s1, s2, res);
	syn_nextsymbol();
	if (syn_ID != lex_leftparenthesis_id)//（
	{
		Grammar_Error(err_leftparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID != lex_rightparenthesis_id)//）
	{
		Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID != lex_leftbraces_id)//{
	{
		Grammar_Error(err_leftbraces, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_mixsentence();//进复合语句
	if (syn_ID != lex_rightbraces_id)//}
	{
		Grammar_Error(err_rightbraces, syn_errorline, syn_errorcolon);
		return;
	}
	op = op_exits;
	s1 = op_space;
	s2 = op_space;
	res = remsymbol;
	IR_Output(op, s1, s2, res);
	cout << "*语法分析：主函数结束" << endl;
}

/**
	＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
**/
void syn_expression()//表达式
{
	cout << "*语法分析：表达式开始" << endl;
	string op, s1, s2, res;//四元式
	if (syn_ID == lex_add_id || syn_ID == lex_subtract_id)//+or-
	{
		int remsynID = syn_ID;
		syn_add();
		syn_term();
		if (remsynID == lex_add_id)
		{
			res = glbtemp;
		}
		else
		{
			op = op_sub;
			s1 = op_zero;
			s2 = glbtemp;
			res = genvar();
			IR_Output(op, s1, s2, res);//生成四元式，将-x变为0-x
		}
	}
	else//*or/
	{
		syn_term();//进入项
		res = glbtemp;
	}
	while (syn_ID == lex_add_id || syn_ID == lex_subtract_id)
	{
		if (syn_ID == lex_add_id)
		{
			op = op_add;
		}
		else
		{
			op = op_sub;
		}
		syn_add();
		syn_term();
		s1 = res;//上一个表达式的结果作为这一个
		s2 = glbtemp;
		res = genvar();//生成一个临时变量
		IR_Output(op, s1, s2, res);//生成四元式
	}
	glbtemp = res;
	cout << "*语法分析：表达式结束" << endl;
}
/**
	＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}
**/
void syn_term()//项
{
	cout << "*语法分析：项开始" << endl;
	string op, s1, s2, res;//四元式
	syn_factor();//因子
	res = glbtemp;
	while (syn_ID == lex_multiply_id || syn_ID == lex_divide_id)//*or/
	{
		if (syn_ID == lex_multiply_id)
		{
			op = op_mul;
		}
		else
		{
			op = op_div;
		}
		syn_multiply();
		syn_factor();
		s1 = res;
		s2 = glbtemp;

		res = genvar();
		IR_Output(op, s1, s2, res);
	}
	
	glbtemp = res;
	cout << "*语法分析：项结束" << endl;
}
/**
	＜因子＞    ::= ＜标识符＞
				  |＜标识符＞‘[’＜表达式＞‘]’
				  |‘(’＜表达式＞‘)’
				  |＜整数＞
				  |＜实数＞
				  |＜字符＞
				  |＜有返回值函数调用语句＞
**/
void syn_factor()//因子
{
	cout << "*语法分析：因子开始" << endl;
	string op, s1, s2, res;
	if (syn_ID == lex_indentifer_id)
	{
		syn_record();//记录当前symbol
		syn_read_ahead();//预读一个symbol
		if (syn_ID == lex_leftmiddle_id)// ＜标识符＞‘[’＜表达式＞‘]’
		{
			syn_rollback();//回退
			//检查变量是否定义过
			if (SYN_Const_Var.find(symbol) == SYN_Const_Var.end() && SYN_Local_Var.find(symbol) == SYN_Local_Var.end() && SYN_Global_Var.find(symbol) == SYN_Global_Var.end())
			{
				//为定义则报错
				Grammar_Error(err_var_redefination, syn_errorline, syn_errorcolon);
				return;
			}
			syn_identifier();//进标识符
			syn_nextsymbol();//[
			syn_expression();//进表达式
			if (syn_ID != lex_rightmiddle_id)//]
			{
				Grammar_Error(err_rightmiddle, syn_errorline, syn_errorcolon);
				return;
			}
			syn_nextsymbol();
		}
		else if (syn_ID == lex_leftparenthesis_id) //＜有返回值函数调用语句＞
		{
			syn_rollback();//回退
			/*if(syn_functionwithreturnname.find(symbol)==syn_functionwithreturnname.end());
			{
				//如果函数为定义则报未定义
				errorcontent(err_function_undefination,syn_errorline,syn_errorcolon);
				return;
			}*/
			syn_functionwithreturn_recall();//进有返回值函数调用语句
		}
		else
		{
			syn_rollback();//回退
			glbtemp = symbol;
			syn_identifier();//进标识符
		}
	}
	else if (syn_ID == lex_leftparenthesis_id)//‘(’＜表达式＞‘)’
	{
		syn_nextsymbol();//（
		syn_expression();//进表达式
		if (syn_ID != lex_rightparenthesis_id)
		{
			Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
	}
	else if (syn_ID == lex_character_id)//＜字符＞
	{
		glbtemp = symbol;
		syn_character();
	}
	else
	{
		if (syn_ID == lex_NotZeroHeadNum_id) //＜整数＞|＜实数＞
		{
			glbtemp = symbol;
			syn_int();
		}
		if (syn_ID == lex_dot_id)
		{
			glbtemp = symbol;
			syn_Fractional();//小数部分
		}
	}
	cout << "*语法分析：因子结束" << endl;
}
/**
	＜语句＞    ::= ＜条件语句＞
				 ｜＜循环语句＞
				 |＜情况语句＞
				 ｜＜读语句＞;
				 ｜＜写语句＞;
				 ｜＜返回语句＞;
				 | ‘{’＜语句列＞‘}’
				 ｜＜赋值语句＞;
				 ｜＜有返回值函数调用语句＞;
				 |＜无返回值函数调用语句＞;
				 ｜＜空＞;
**/
void syn_sentence()//语句
{
	cout << "*语法分析：语句开始" << endl;
	string op, s1, s2, res;
	if (syn_ID == lex_if_id)//＜条件语句＞
	{
		syn_condition_sen();
	}
	else if (syn_ID == lex_while_id || syn_ID == lex_do_id || syn_ID == lex_for_id)// ＜循环语句＞
	{
		syn_loop_sen();
	}
	else if (syn_ID == lex_switch_id)//＜情况语句＞
	{
		syn_situation_sen();
	}
	else if (syn_ID == lex_leftbraces_id)//‘{’＜语句列＞‘}’
	{
		if (syn_ID != lex_leftbraces_id)//{
		{
			Grammar_Error(err_leftbraces, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
		syn_sentence_colomn();//进<语句列>
		if (syn_ID != lex_rightbraces_id)//}
		{
			Grammar_Error(err_rightbraces, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
	}
	else if (syn_ID == lex_scanf_id)//＜读语句＞;
	{
		syn_scanf();
		if (syn_ID != lex_semicolon_id)//；
		{
			Grammar_Error(err_semicolon, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
	}
	else if (syn_ID == lex_printf_id)//＜写语句＞;
	{
		syn_printf();
		if (syn_ID != lex_semicolon_id)//；
		{
			Grammar_Error(err_semicolon, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
	}
	else if (syn_ID == lex_return_id)//＜返回语句＞;
	{
		syn_return();
		if (syn_ID != lex_semicolon_id)
		{
			Grammar_Error(err_semicolon, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
	}
	else if (syn_ID == lex_indentifer_id)
	{
		syn_record();
		syn_read_ahead();
		if (syn_ID == lex_equal_id || syn_ID == lex_leftmiddle_id)//＜赋值语句＞;
		{
			syn_rollback();//回退
			//如果变量名在变量定义和常量定义中都没出现，则报错未定义
			if (SYN_Const_Var.find(symbol) == SYN_Const_Var.end() && SYN_Local_Var.find(symbol) == SYN_Local_Var.end() && SYN_Global_Var.find(symbol) == SYN_Global_Var.end())
			{
				Grammar_Error(err_var_undefination, syn_errorline, syn_errorcolon);
				return;
			}
			if (SYN_Const_Var.find(symbol) != SYN_Const_Var.end())//给常量赋值，报错
			{
				Grammar_Error(err_const_assign, syn_errorline, syn_errorcolon);
				return;
			}
			syn_assign_sen();//进<赋值语句>
			if (syn_ID != lex_semicolon_id)
			{
				Grammar_Error(err_semicolon, syn_errorline, syn_errorcolon);
				return;
			}
			syn_nextsymbol();
		}
		else if (Return_Func.find(symbol) != Return_Func.end())//＜有返回值函数调用语句＞;
		{
			syn_functionwithreturn_recall();
			if (syn_ID != lex_semicolon_id)
			{
				Grammar_Error(err_semicolon, syn_errorline, syn_errorcolon);
				return;
			}
			syn_nextsymbol();
		}
		else if (No_Return_Func.find(symbol) != No_Return_Func.end())//＜无返回值函数调用语句＞;
		{
			syn_functionwithoutreturn_recall();
			if (syn_ID != lex_semicolon_id)
			{
				Grammar_Error(err_semicolon, syn_errorline, syn_errorcolon);
				return;
			}
			syn_nextsymbol();
		}
		else
		{
			///空
		}
	}
	cout << "*语法分析：语句结束" << endl;
}
/**
	＜语句列＞   ::= ｛＜语句＞｝
**/
void syn_sentence_colomn()//语句列
{
	cout << "*语法分析：语句列开始" << endl;
	while (syn_ID != lex_rightbraces_id)
	{
		syn_sentence();
	}
	cout << "*语法分析：语句列结束" << endl;
}
/**
	＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞
					 |＜标识符＞‘[’＜表达式＞‘]’=＜表达式＞
**/
void syn_assign_sen()//赋值语句
{
	cout << "*语法分析：赋值语句开始" << endl;
	string op, s1, s2, res;
	if (syn_ID != lex_indentifer_id)
	{
		Grammar_Error(err_identifier, syn_errorline, syn_errorcolon);
		return;
	}
	//如果变量名在变量定义和常量定义中都没出现，则报错未定义
	if (SYN_Const_Var.find(symbol) == SYN_Const_Var.end() && SYN_Local_Var.find(symbol) == SYN_Local_Var.end() && SYN_Global_Var.find(symbol) == SYN_Global_Var.end())
	{
		Grammar_Error(err_var_undefination, syn_errorline, syn_errorcolon);
		return;
	}
	//如果给常量赋值则报错
	if (SYN_Const_Var.find(symbol) != SYN_Const_Var.end())
	{
		Grammar_Error(err_const_assign, syn_errorline, syn_errorcolon);
		return;
	}
	res = symbol;
	syn_identifier();
	if (syn_ID == lex_equal_id)
	{
		syn_record();
		syn_read_ahead();
		syn_read_ahead();
		if(syn_ID == lex_leftmiddle_id)//将数组赋值给变量
		{
			syn_rollback();
			op = op_arryass;	
			syn_nextsymbol();
			s2 = symbol;
			syn_expression();
			s1 = glbtemp;
			IR_Output(op, s1, s2, res);//生成四元式
		}
		else//将单一值赋给变量
		{
			syn_rollback();
			op = op_ass;
			syn_nextsymbol();
			syn_expression();
			s1 = glbtemp;
			s2 = op_space;
			IR_Output(op, s1, s2, res);//生成四元式
		}

	}
	else if (syn_ID == lex_leftmiddle_id)//赋值给数组
	{
		op = op_assarry;
		syn_nextsymbol();
		syn_expression();
		s2 = glbtemp;
		if (syn_ID != lex_rightmiddle_id)
		{
			Grammar_Error(err_rightmiddle, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
		syn_nextsymbol();
		syn_expression();
		s1 = glbtemp;
		IR_Output(op, s1, s2, res);//生成四元式
	}
	cout << "*语法分析：赋值语句结束" << endl;
}
/**
	＜条件语句＞  ::=  if ‘(’＜条件＞‘)’＜语句＞［else＜语句＞］
**/
string cop, cs1, cs2;
void syn_condition_sen()//条件语句
{
	cout << "*语法分析：条件语句开始" << endl;
	string op, s1, s2, res;//四元式
	//LABEL的功能是定义变量或标号的类型，而变量或标号的段属性和偏移属性由该语句所处的位置确定。
	string label1 = genlable();//生成一个新的label
	string label2 = genlable();//生成一个新的label
	if (syn_ID != lex_if_id)//if
	{
		Grammar_Error(err_if, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID != lex_leftparenthesis_id)//（
	{
		Grammar_Error(err_leftparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_condition();//进条件
	/// skip if
	op = cop;
	s1 = cs1;
	s2 = cs2;
	res = label1;
	IR_Output(op, s1, s2, res);
	if (syn_ID != lex_rightparenthesis_id)//）
	{
		Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_sentence();
	/// skip else
	op = op_jmp;
	s1 = op_space;
	s2 = op_space;
	res = label2;
	IR_Output(op, s1, s2, res);
	/// set label1
	op = op_lab;
	s1 = op_space;
	s2 = op_space;
	res = label1;
	IR_Output(op, s1, s2, res);
	if (syn_ID == lex_else_id)
	{
		syn_nextsymbol();
		syn_sentence();
	}
	/// set label2
	op = op_lab;
	s1 = op_space;
	s2 = op_space;
	res = label2;
	IR_Output(op, s1, s2, res);
	cout << "*语法分析：条件语句结束" << endl;
}
/**
	＜条件＞    ::=  ＜表达式＞＜关系运算符＞＜表达式＞
				  ｜＜表达式＞ //表达式为0条件为假，否则为真
**/
void syn_condition()//条件
{
	cout << "*语法分析：条件开始" << endl;
	string op, s1, s2, res;
	syn_expression();
	s1 = glbtemp;
	if (syn_ID == lex_less_id || syn_ID == lex_lessequal_id || syn_ID == lex_greater_id || syn_ID == lex_greaterequal_id || syn_ID == lex_equalequal_id || syn_ID == lex_notequal_id)
	{
		if (syn_ID == lex_less_id)//<
		{
			op = op_bet;
			syn_relation();
			syn_expression();
			s2 = glbtemp;
			res = op_space;
		}
		else if (syn_ID == lex_lessequal_id)//<=
		{
			op = op_bt;
			syn_relation();
			syn_expression();
			s2 = glbtemp;
			res = op_space;
		}
		else if (syn_ID == lex_greater_id)//>
		{
			op = op_set;
			syn_relation();
			syn_expression();
			s2 = glbtemp;
			res = op_space;
		}
		else if (syn_ID == lex_greaterequal_id)//>=
		{
			op = op_st;
			syn_relation();
			syn_expression();
			s2 = glbtemp;
			res = op_space;
		}
		else if (syn_ID == lex_notequal_id)// !=
		{
			//op=op_neq;
			op = op_eql;
			syn_relation();
			syn_expression();
			s2 = glbtemp;
			res = op_space;
		}
		else if (syn_ID == lex_equalequal_id)//==
		{
			//op=op_eql;
			op = op_neq;
			syn_relation();
			syn_expression();
			s2 = glbtemp;
			res = op_space;
		}
	}
	else
	{
		op = mips_beq;
		s2 = op_zero;
		res = op_space;
	}
	cop = op;
	cs1 = s1;
	cs2 = s2;
	cout << "*语法分析：条件结束" << endl;
}
/**
	＜for赋值语句＞   ::=  ＜标识符＞＝＜表达式＞
					 |＜标识符＞‘[’＜表达式＞‘]’=＜表达式＞
**/
string forop, fors1, fors2, forres;
void syn_forassign()//for赋值语句
{
	cout << "*语法分析：for赋值语句开始" << endl;
	string op, s1, s2, res;
	if (syn_ID != lex_indentifer_id)
	{
		Grammar_Error(err_identifier, syn_errorline, syn_errorcolon);
		return;
	}
	//如果变量名在变量定义和常量定义中都没出现，则报错未定义
	if (SYN_Const_Var.find(symbol) == SYN_Const_Var.end() && SYN_Local_Var.find(symbol) == SYN_Local_Var.end() && SYN_Global_Var.find(symbol) == SYN_Global_Var.end())
	{
		Grammar_Error(err_var_undefination, syn_errorline, syn_errorcolon);
		return;
	}
	// 如果给常量赋值则报错
	if (SYN_Const_Var.find(symbol) != SYN_Const_Var.end())
	{
		Grammar_Error(err_const_assign, syn_errorline, syn_errorcolon);
		return;
	}
	res = symbol;
	syn_identifier();
	if (syn_ID == lex_equal_id)
	{
		op = op_ass;
		syn_nextsymbol();
		syn_expression();
		s1 = glbtemp;
		s2 = op_space;
	}
	else if (syn_ID == lex_leftmiddle_id)//赋值给数组
	{
		op = op_assarry;
		syn_nextsymbol();
		syn_expression();
		s2 = glbtemp;
		if (syn_ID != lex_rightmiddle_id)
		{
			Grammar_Error(err_rightmiddle, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
		if (syn_ID != lex_equal_id)
		{
			Grammar_Error(err_ass, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
		syn_expression();
		s1 = glbtemp;
	}
	forop = op;
	fors1 = s1;
	fors2 = s2;
	forres = res;
	cout << "*语法分析：for赋值语句结束" << endl;
}
/**
	＜循环语句＞   ::=  while ‘(’＜条件＞‘)’＜语句＞
						|do＜语句＞while ‘(’＜条件＞‘)’
**/
void syn_loop_sen()//循环语句
{
	cout << "*语法分析：循环语句开始" << endl;
	string op, s1, s2, res;
	string label1 = genlable();//生成新的label
	string label2 = genlable();//生成新的label
	/// while ‘(’＜条件＞‘)’＜语句＞
	if (syn_ID == lex_while_id)//while
	{
		if (syn_ID != lex_while_id)
		{
			Grammar_Error(err_while, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
		if (syn_ID != lex_leftparenthesis_id)
		{
			Grammar_Error(err_leftparenthesis, syn_errorline, syn_errorcolon);
			return;
		}
		//set label2
		IR_Output(op_lab, op_space, op_space, label2);
		syn_nextsymbol();
		syn_condition();
		op = cop;
		s1 = cs1;
		s2 = cs2;
		res = label1;
		//jump to label1
		IR_Output(op, s1, s2, res);
		if (syn_ID != lex_rightparenthesis_id)
		{
			Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
		syn_sentence();/// 进<语句>
		//jump to label2
		IR_Output(op_jmp, op_space, op_space, label2);//继续循环
		//set label1
		IR_Output(op_lab, op_space, op_space, label1);//跳出循环
	}
	/// do＜语句＞while ‘(’＜条件＞‘)’
	else if (syn_ID == lex_do_id)//do
	{
		if (syn_ID != lex_do_id)
		{
			Grammar_Error(err_do, syn_errorline, syn_errorcolon);
			return;
		}
		IR_Output(op_lab, op_space, op_space, label1);//label1
		syn_nextsymbol();
		syn_sentence();
		if (syn_ID != lex_leftparenthesis_id)
		{
			Grammar_Error(err_leftparenthesis, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
		syn_condition();//条件
		op = cop;
		s1 = cs1;
		s2 = cs2;
		res = label2;
		IR_Output(op, s1, s2, res);//break
		IR_Output(op_jmp, op_space, op_space, label1);//continue
		IR_Output(op_lab, op_space, op_space, label2);//label2
		if (syn_ID != lex_rightparenthesis_id)
		{
			Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
	}
	cout << "*语法分析：循环语句结束" << endl;
}
/**
	＜情况语句＞  ::=  switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞＜缺省＞ ‘}’
**/

string switchtemp;
void syn_situation_sen()//情况语句
{
	cout << "*语法分析：情况语句开始" << endl;
	if (syn_ID != lex_switch_id)//switch
	{
		Grammar_Error(err_switch, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID != lex_leftparenthesis_id)//（
	{
		Grammar_Error(err_leftparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_expression();
	switchtemp = glbtemp;
	if (syn_ID != lex_rightparenthesis_id)//）
	{
		Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID != lex_leftbraces_id)//{
	{
		Grammar_Error(err_leftbraces, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_situation_table();//进情况表
	if (syn_ID == lex_default_id)
	{
		syn_default();//default
	}
	if (syn_ID != lex_rightbraces_id)//}
	{
		Grammar_Error(err_rightbraces, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	cout << "*语法分析：情况语句结束" << endl;
}
/**
	＜情况表＞   ::=  ＜情况子语句＞{＜情况子语句＞}
**/
void syn_situation_table()//情况表
{
	cout << "*语法分析：情况表开始" << endl;
	syn_situation_sen_son();
	while (1)
	{
		syn_situation_sen_son();
		if (syn_ID == lex_default_id || syn_ID == lex_rightbraces_id)//何时终止
		{
			break;
		}
	}
	cout << "*语法分析：情况表结束" << endl;
}
/**
	＜情况子语句＞  ::=  case＜可枚举常量＞：＜语句＞
**/
void syn_situation_sen_son()//情况子语句
{
	cout << "*语法分析：情况子语句开始" << endl;
	if (syn_ID != lex_case_id)//case
	{
		Grammar_Error(err_case, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	string label = genlable();
	IR_Output(op_neq, symbol, switchtemp, label);
	syn_enum();//进可枚举常量
	if (syn_ID != lex_colon_id)//：
	{
		Grammar_Error(err_colon, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_sentence();//进语句
	IR_Output(op_lab, op_space, op_space, label);
	cout << "*语法分析：情况子语句结束" << endl;
}
/**
	＜缺省＞   ::=  default : ＜语句＞
				  |＜空＞
**/
void syn_default()//缺省
{
	cout << "*语法分析：缺省开始" << endl;
	if (syn_ID != lex_default_id)
	{
		return;
	}
	syn_nextsymbol();
	if (syn_ID != lex_colon_id)//:
	{
		Grammar_Error(err_colon, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_sentence();//进语句
	cout << "*语法分析：缺省结束" << endl;
}
/**
	＜有返回值函数调用语句＞ ::= ＜标识符＞‘(’＜值参数表＞‘)’
**/
void syn_functionwithreturn_recall()//有返回值函数调用语句
{
	cout << "*语法分析：有返回值函数调用语句开始" << endl;
	string op, s1, s2, res;//四元式
	if (syn_ID != lex_indentifer_id)
	{
		Grammar_Error(err_identifier, syn_errorline, syn_errorcolon);
		return;
	}
	op = op_call;
	s1 = symbol;
	s2 = op_space;
	res = genvar();//生成临时变量存储返回值
	syn_nextsymbol();
	if (syn_ID != lex_leftparenthesis_id)//（
	{
		Grammar_Error(err_leftparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_value_parameter_table();//进值参数表
	if (syn_ID != lex_rightparenthesis_id)//）
	{
		Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	IR_Output(op, s1, s2, res);//生成四元式
	glbtemp = res;//将结果赋给全局临时变量
	cout << "*语法分析：有返回值函数调用语句结束" << endl;
}
/**
	＜无返回值函数调用语句＞ ::= ＜标识符＞‘(’＜值参数表＞‘)’
**/
void syn_functionwithoutreturn_recall()//无返回值函数调用语句
{
	cout << "*语法分析：无返回值函数调用语句开始" << endl;
	string op, s1, s2, res;//四元式
	if (syn_ID != lex_indentifer_id)
	{
		Grammar_Error(err_identifier, syn_errorline, syn_errorcolon);
		return;
	}
	op = op_call;
	s1 = symbol;
	s2 = op_space;
	res = op_space;
	if (syn_ID != lex_leftparenthesis_id)//（
	{
		Grammar_Error(err_leftparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	syn_value_parameter_table();//进值参数表
	if (syn_ID != lex_rightparenthesis_id)//）
	{
		Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	IR_Output(op, s1, s2, res);//生成四元式
	cout << "*语法分析：无返回值函数调用语句结束" << endl;
}
/**
	＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}
					｜＜空＞
**/
stack<string>parastack;
void syn_value_parameter_table()//值参数表//没有处理<空>的情况
{
	cout << "*语法分析：值参数表开始" << endl;
	string op, s1, s2, res;
	op = op_callpara;
	s1 = op_space;
	s2 = op_space;
	syn_expression();//进<表达式>
	res = glbtemp;//将结果赋给全局临时变量
	parastack.push(res);
	while (syn_ID == lex_comma_id)//，
	{
		syn_nextsymbol();
		syn_expression();
		res = glbtemp;
		parastack.push(res);
	}
	while (!parastack.empty())//参数由右往左压栈
	{
		res = parastack.top();
		parastack.pop();
		IR_Output(op, s1, s2, res);//生成四元式
	}
	cout << "*语法分析：值参数表结束" << endl;
}
/**
	＜读语句＞    ::=  scanf ‘(’＜标识符＞{,＜标识符＞}‘)’
**/
void syn_scanf()//读语句
{
	cout << "*语法分析：读语句开始" << endl;
	string op, s1, s2, res;
	if (syn_ID != lex_scanf_id)//scanf
	{
		Grammar_Error(err_scanf, syn_errorline, syn_errorcolon);
		return;
	}
	op = op_scanf;
	syn_nextsymbol();
	if (syn_ID != lex_leftparenthesis_id)//（
	{
		Grammar_Error(err_leftparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	//检查变量是否定义过如果变量没定义过则报错
	if (SYN_Const_Var.find(symbol) == SYN_Const_Var.end() && SYN_Local_Var.find(symbol) == SYN_Local_Var.end() && SYN_Global_Var.find(symbol) == SYN_Global_Var.end())
	{
		Grammar_Error(err_var_undefination, syn_errorline, syn_errorcolon);
		return;
	}
	//如果给常量赋值则报错
	if (SYN_Const_Var.find(symbol) != SYN_Const_Var.end())
	{
		Grammar_Error(err_const_assign, syn_errorline, syn_errorcolon);
		return;
	}
	s1 = op_space;
	s2 = op_space;
	res = symbol;
	syn_identifier();
	IR_Output(op, s1, s2, res);//生成四元式
	while (syn_ID == lex_comma_id)//如果是，说明有多个输入变量，重复上述操作
	{
		syn_nextsymbol();
		res = symbol;
		syn_identifier();
		IR_Output(op, s1, s2, res);
	}
	if (syn_ID != lex_rightparenthesis_id)//）
	{
		Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	cout << "*语法分析：读语句结束" << endl;
}
/**
	＜写语句＞    ::= printf ‘(’ ＜字符串＞,＜表达式＞ ‘)’
				  | printf ‘(’＜字符串＞ ‘)’
				  | printf ‘(’＜表达式＞‘)’
**/
void syn_printf()//写语句
{
	cout << "*语法分析：写语句开始" << endl;
	string op, s1, s2, res;
	if (syn_ID != lex_printf_id)//printf
	{
		Grammar_Error(err_printf, syn_errorline, syn_errorcolon);
		return;
	}
	op = op_printf;
	syn_nextsymbol();
	if (syn_ID != lex_leftparenthesis_id)//（
	{
		Grammar_Error(err_leftparenthesis, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID == lex_string_id)//string
	{
		s1 = symbol;
		if (String_List.find(symbol) == String_List.end())
		{
			String_List[symbol] = genstring();
		}
		syn_string();
		if (syn_ID == lex_comma_id)
		{
			syn_nextsymbol();
			syn_expression();
			s2 = glbtemp;
			if (syn_ID != lex_rightparenthesis_id)//）
			{
				Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
				return;
			}
			syn_nextsymbol();
		}
		else if (syn_ID == lex_rightparenthesis_id)//）
		{
			s2 = op_space;
			syn_nextsymbol();
		}
		else
		{
			Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
			return;
		}
	}
	else
	{
		s1 = op_space;
		syn_expression();
		s2 = glbtemp;
		if (syn_ID != lex_rightparenthesis_id)//）
		{
			Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
	}
	IR_Output(op, s1, s2, res);//生成四元式
	cout << "*语法分析：写语句结束" << endl;
}
/**
	＜返回语句＞   ::=  return[‘(’＜表达式＞‘)’]

	＜返回语句＞   ::=  return[＜表达式＞]
**/
void syn_return()//返回语句
{
	cout << "*语法分析：返回语句开始" << endl;
	string op, s1, s2, res;
	if (syn_ID != lex_return_id)//return
	{
		Grammar_Error(err_return, syn_errorline, syn_errorcolon);
		return;
	}
	op = op_ret;
	s1 = op_space;
	s2 = op_space;
	syn_nextsymbol();
	//return （表达式）；
	if (syn_ID == lex_leftparenthesis_id)//（
	{
		syn_nextsymbol();
		syn_expression();
		res = glbtemp;//将值赋给全局临时变量
		if (syn_ID != lex_rightparenthesis_id)//）
		{
			Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
	}
	//return 表达式；
	else if (syn_ID == lex_indentifer_id || syn_ID == lex_NotZeroHeadNum_id || syn_ID == lex_character_id)
	{
		syn_expression();
		res = glbtemp;//将值赋给全局临时变量
		if (syn_ID != lex_rightparenthesis_id)//）
		{
			Grammar_Error(err_rightparenthesis, syn_errorline, syn_errorcolon);
			return;
		}
		syn_nextsymbol();
	}
	//return
	else if (syn_ID != lex_semicolon_id)//；
	{
		Grammar_Error(err_semicolon, syn_errorline, syn_errorcolon);
		return;
	}
	//return；
	else
	{
		res = op_space;
	}
	IR_Output(op, s1, s2, res);
	cout << "*语法分析：返回语句结束" << endl;
}
/**
	＜加法运算符＞  ::= +｜-
**/
void syn_add()//加法运算符
{
	cout << "*语法分析：加法运算符开始" << endl;
	if (syn_ID != lex_add_id && syn_ID != lex_subtract_id)
	{
		Grammar_Error(err_addorsub, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	cout << "*语法分析：加法运算符结束" << endl;
}
/**
	＜乘法运算符＞  ::= *｜/
**/
void syn_multiply()//乘法运算符
{
	cout << "*语法分析：乘法运算符开始" << endl;
	if (syn_ID != lex_multiply_id && syn_ID != lex_divide_id)
	{
		Grammar_Error(err_mulordiv, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	cout << "*语法分析：乘法运算符结束" << endl;
}
/**
	＜关系运算符＞  ::=  <｜<=｜>｜>=｜!=｜==
**/
void syn_relation()//关系运算符
{
	cout << "*语法分析：关系运算符开始" << endl;
	if (syn_ID != lex_less_id && syn_ID != lex_lessequal_id && syn_ID != lex_greater_id && syn_ID != lex_greaterequal_id && syn_ID != lex_notequal_id && syn_ID != lex_equalequal_id)
	{
		Grammar_Error(err_relation, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	cout << "*语法分析：关系运算符结束" << endl;
}
/**
	＜字母＞   ::= ＿｜a｜．．．｜z｜A｜．．．｜Z
**/
void syn_letter()//字母
{
	cout << "*语法分析：字母开始" << endl;
	if (syn_ID != lex_letter_id)
	{
		Grammar_Error(err_letter, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	cout << "*语法分析：字母结束" << endl;
}
/**
	＜数字＞   ::= ０｜＜非零数字＞
**/
void syn_digit()//数字
{
	cout << "*语法分析：数字开始" << endl;
	if (syn_ID != lex_ZeroToNine)
	{
		Grammar_Error(err_digit, syn_errorline, syn_errorcolon);
		return;
	}
	if (symbol == "0")
	{
		syn_nextsymbol();
	}
	else
	{
		syn_notZeroDigit();
	}
	cout << "*语法分析：数字结束" << endl;
}
/**
	＜非零数字＞  ::= １｜．．．｜９
**/

void syn_notZeroDigit()//非零数字
{
	cout << "*语法分析：非零数字开始" << endl;
	if (syn_ID != lex_ZeroToNine)
	{
		Grammar_Error(err_digit, syn_errorline, syn_errorcolon);
		return;
	}
	if (!(symbol >= "1" && symbol <= "9"))
	{
		Grammar_Error(err_notzero, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	cout << "*语法分析：非零数字结束" << endl;
}
/**
	＜字符＞    ::=  '＜加法运算符＞'｜'＜乘法运算符＞'｜'＜字母＞'｜'＜数字＞'
**/
void syn_character()//字符
{
	cout << "*语法分析：字符开始" << endl;
	if (syn_ID != lex_singlemark_id)
	{
		Grammar_Error(err_singlemark, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID == lex_add_id)
	{
		syn_add();
	}
	else if (syn_ID == lex_multiply_id)
	{
		syn_multiply();
	}
	else if (syn_ID == lex_letter_id)
	{
		syn_letter();
	}
	else if (syn_ID == lex_ZeroToNine)
	{
		syn_digit();
	}
	else
	{
		Grammar_Error(err_character, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	if (syn_ID != lex_singlemark_id)
	{
		Grammar_Error(err_singlemark, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	cout << "*语法分析：字符结束" << endl;
}
/**
	＜字符串＞   ::=  "｛十进制编码为32,33,35-126的ASCII字符｝"
**/
void syn_string()//字符串
{
	cout << "*语法分析：字符串开始" << endl;
	if (syn_ID != lex_string_id)
	{
		Grammar_Error(err_string, syn_errorline, syn_errorcolon);
		return;
	}
	syn_nextsymbol();
	cout << "*语法分析：字符串结束" << endl;
}
/**
	＜无符号整数＞  ::= ＜非零数字＞｛＜数字＞｝
	###＜无符号整数＞  ::= DIGIT_SYMID
**/
void syn_noSignInt()//无符号整数
{
	cout << "*语法分析：无符号整数开始" << endl;
	if (syn_ID == lex_NotZeroHeadNum_id)
	{
		syn_nextsymbol();
	}
	else
	{
		Grammar_Error(err_number, syn_errorline, syn_errorcolon);
		return;
	}
	cout << "*语法分析：无符号整数结束" << endl;
}
/**
	＜整数＞        ::= ［＋｜－］＜无符号整数＞｜０
**/
void syn_int()//整数
{
	cout << "*语法分析：无符号整数开始" << endl;
	if (syn_ID == lex_add_id || syn_ID == lex_subtract_id)
	{
		syn_nextsymbol();
		syn_noSignInt();
	}
	else if (syn_ID == lex_NotZeroHeadNum_id)
	{
		syn_noSignInt();//进无符号整数
	}
	else
	{
		Grammar_Error(err_int, syn_errorline, syn_errorcolon);
		return;
	}
	cout << "*语法分析：无符号整数结束" << endl;
}
/**
	＜小数部分＞    ::= ＜数字＞｛＜数字＞｝｜＜空＞
**/
void syn_Fractional()//小数部分
{
	cout << "*语法分析：小数部分开始" << endl;
	if (syn_ID == lex_ZeroHeadNum_id)
	{
		syn_nextsymbol();
	}
	else if (syn_ID == lex_NotZeroHeadNum_id)
	{
		syn_nextsymbol();
	}
	else
	{
		syn_nextsymbol();
	}
	cout << "*语法分析：小数部分结束" << endl;
}
/**
	＜实数＞        ::= ［＋｜－］＜整数＞[.＜小数部分＞]
**/
void syn_realNumber()//实数
{
	cout << "*语法分析：实数开始" << endl;
	if (syn_ID == lex_add_id)
	{
		syn_add();
	}
	syn_int();
	if (syn_ID == lex_dot_id)
	{
		syn_nextsymbol();
		syn_Fractional();
	}
	cout << "*语法分析：实数结束" << endl;
}
/**
	＜标识符＞    ::=  ＜字母＞｛＜字母＞｜＜数字＞｝
**/
void syn_identifier()//标识符
{
	cout << "*语法分析：标识符开始" << endl;
	if (syn_ID == lex_indentifer_id)
	{
		syn_nextsymbol();
	}
	else
	{
		Grammar_Error(err_identifier, syn_errorline, syn_errorcolon);
		return;
	}
	cout << "*语法分析：标识符结束" << endl;
}
/**
	＜可枚举常量＞   ::=  ＜整数＞|＜字符＞
**/
void syn_enum()//可枚举常量
{
	cout << "*语法分析：可枚举常量开始" << endl;
	if (syn_ID == lex_NotZeroHeadNum_id)
	{
		syn_int();
	}
	else if (syn_ID == lex_add_id || syn_ID == lex_multiply_id || syn_ID == lex_letter_id || syn_ID == lex_ZeroToNine)
	{
		syn_letter();
	}
	else
	{
		Grammar_Error(err_enumerabel, syn_errorline, syn_errorcolon);
		return;
	}
	cout << "*语法分析：可枚举常量结束" << endl;
}
/**
	＜类型标识符＞      ::=  int | char
**/
void syn_typeidentifier()//标识符
{
	cout << "*语法分析：类型标识符开始" << endl;
	if (syn_ID == lex_int_id || syn_ID == lex_float_id || syn_ID == lex_char_id)
	{
		syn_nextsymbol();
	}
	else
	{
		Grammar_Error(err_intorchar, syn_errorline, syn_errorcolon);
		return;
	}
	cout << "*语法分析：类型标识符结束" << endl;
}