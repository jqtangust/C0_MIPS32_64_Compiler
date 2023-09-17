#include "head.h"
#include "LEX.h"
#include "Symbol_Table.h"

using namespace std;

ofstream LEX_out("./LEX.txt");

//string keyword[keywordNum] =
//{
//	"error","int","char","float","double","bool","void","const","if","else",
//	"for","while","do","switch","case","default","printf","scanf",
//	"return","main","indentifer",
//	"+","-","*","/","%","<",">","<=",">=","==","!=","=",",",";",
//	":",".","\'","\"","(",")","[","]","{","}",
//	"123456789","string","#","_","2","0","1","0001","struct"
//};//关键字集合
string keywordID[keywordNum] =
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

const string reskeyword[reskeywordNum] =
{
	"int","char","float","double","bool","void","const","if","else",
	"for","while","do","switch","case","default","printf","scanf",
	"return","main","indentifer","struct"
};
const string reskeywordID[reskeywordNum] =
{
	"lex_int_id","lex_char_id","lex_float_id","lex_double_id","lex_bool_id",
	"lex_void_id","lex_const_id","lex_if_id","lex_else_id","lex_for_id","lex_while_id",
	"lex_do_id","lex_switch_id","lex_case_id","lex_default_id","lex_printf_id","lex_scanf_id",
	"lex_return_id","lex_main_id","lex_indentifer","lex_struct_id"
};

vector<string> LEX_File;//源文件

int Line_of_File = 0;
// string lex_line,lex_preline;//每一行的字符串

int lex_indexOfFile = -1;
int lex_indexOfLine = 0;
int lex_lenOfline = 0;
int fileISend = false;
int lex_preindexOfFile, lex_preindexOfLine;//源文件行索引，表示读到行的第几个数字
int lex_prelenOfline;//源文件的行长度
char nowCH, preCH;//当前读入的字符
bool prefileSend;//源文件是否读完
string lex_sym;//当前读入的字符串
int lex_ID;//当前读入的字符串所对应的ID

string lex_line, lex_preline;

ifstream myFile;

ifstream Readin(string FILE)
{
	ifstream myfile(FILE, ios::in);

	if (!myfile.is_open())
	{
		cout << "can not open this file" << endl;
		return myfile;
	}
	return myfile;
}

void Read_File(ifstream& File)//读源文件，并计算源文件有多少行
{
	string lex_line;
	while (getline(File, lex_line))
	{
		LEX_File.push_back(lex_line);
		Line_of_File++;
	}
}

void lex_nextchar()//读取下一个字符
{
	if (lex_indexOfLine >= lex_lenOfline)
	{
		lex_indexOfFile++;//指向每一行文件
		if (lex_indexOfFile == Line_of_File)
		{
			fileISend = true;
			return;
		}
		lex_line = LEX_File[lex_indexOfFile];//读取每一行文件
		lex_lenOfline = lex_line.length();
		lex_indexOfLine = 0;
		nowCH = '\n';
	}
	else
	{
		nowCH = lex_line[lex_indexOfLine];
		lex_indexOfLine++;
	}
}
void lex_recordCH()//记录
{
	lex_preindexOfFile = lex_indexOfFile;
	lex_preindexOfLine = lex_indexOfLine;
	lex_prelenOfline = lex_lenOfline;
	lex_preline = lex_line;
	prefileSend = fileISend;
	preCH = nowCH;
}
void lex_rollbackCH()//回滚
{
	lex_indexOfFile = lex_preindexOfFile;
	lex_indexOfLine = lex_preindexOfLine;
	lex_lenOfline = lex_prelenOfline;
	lex_line = lex_preline;
	fileISend = prefileSend;
	nowCH = preCH;
}
string to_string_lex(int n)
{
	int m = n;
	char s[100];
	char ss[100];
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

void write()//写入lexical.txt文档
{
	LEX_out << setw(25) << left << keywordID[lex_ID];
	LEX_out << setw(10) << left << lex_ID << lex_sym << endl;
}
bool isspace()//是否是空格
{
	if (nowCH == ' ')
	{
		return true;
	}
	return false;
}
bool istab()//是否是Tab
{
	if (nowCH == '\t')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isenter()//是否是回车
{
	if (nowCH == '\n')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isnumber()//是否是数字
{
	if (nowCH >= '0' && nowCH <= '9')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool iszero()//是否是0
{
	if (nowCH == '0')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isnotzeronumber()//是否是不以零开头的数字
{
	if (nowCH >= '1' && nowCH <= '9')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isletter()//是否是字母，“_”因为定义变量函数名时可以加入
{
	if ((nowCH == '_') || (nowCH >= 'a' && nowCH <= 'z') || (nowCH >= 'A' && nowCH <= 'Z'))
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool ischaracter()//是否是字符
{
	if (nowCH == 32 || nowCH == 33 || (nowCH >= 35 && nowCH <= 126))
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isadd()//是否是加号
{
	if (nowCH == '+')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool issubtract()//是否是减号
{
	if (nowCH == '-')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool ismultiply()//是否是乘号
{
	if (nowCH == '*')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isdivide()//是否是除号
{
	if (nowCH == '/')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool ispercent()//是否是百分号
{
	if (nowCH == '%')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isless()//是否是小于号
{
	if (nowCH == '<')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isgreater()//是否是大于号
{
	if (nowCH == '>')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isnot()//是否是非号
{
	if (nowCH == '!')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isequal()//是否是等号
{
	if (nowCH == '=')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool iscomma()//是否是逗号
{
	if (nowCH == ',')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool issemicolon()//是否是分号
{
	if (nowCH == ';')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool iscolon()//是否是冒号
{
	if (nowCH == ':')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isdot()//是否是点
{
	if (nowCH == '.')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool issinglemark()//是否是单引号
{
	if (nowCH == '\'')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isdoublemark()//是否是双引号
{
	if (nowCH == '\"')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isleftparenthesis()//是否是左小括号
{
	if (nowCH == '(')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isrightparenthesis()//是否是右小括号
{
	if (nowCH == ')')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isleftmiddle()//是否是左中括号
{
	if (nowCH == '[')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isrightmiddle()//是否是右中括号
{
	if (nowCH == ']')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isleftbraces()//是否是左大括号
{
	if (nowCH == '{')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isrightbraces()//是否是右大括号
{
	if (nowCH == '}')
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isExclamation()//是否是叹号
{
	if (nowCH == '!')
	{
		return true;
	}
	else
	{
		return false;
	}
}

void lex_Analysis()//读取一个字符串
{
	lex_sym = "";
	lex_ID = 0;
	lex_nextchar();
	if (fileISend)
	{
		return;
	}
	while (nowCH == ' ' || nowCH == '\t' || nowCH == '\n')
	{
		lex_nextchar();
	}

	bool Switch_list[23] = { isadd(),issubtract(),ismultiply(),isdivide(),ispercent(),
		isless(),isgreater(),isExclamation(),isequal(),iscomma(),
		issemicolon(),iscolon(),issinglemark(),isdoublemark(),isleftparenthesis(),
		isrightparenthesis(),isleftmiddle(),isrightmiddle(),isleftbraces(),isrightbraces(),
		isdot(),isletter(),isnumber() };

	int Switch_num = 0;
	for (; Switch_list[Switch_num] != true; Switch_num++)
	{
	}

	switch (Switch_num)
	{
	case 0:
	{
		lex_sym += nowCH;
		lex_ID = lex_add_id;
		break;
	}
	case 1:
	{
		lex_sym += nowCH;
		lex_ID = lex_subtract_id;
		break;
	}
	case 2:
	{
		lex_sym += nowCH;
		lex_ID = lex_multiply_id;
		break;
	}
	case 3://除号判断后要判断后面是否还有/或者*，以用来跳过注释
	{
		lex_sym += nowCH;
		lex_recordCH();
		lex_nextchar();
		if (isdivide())
		{
			while (!isenter())
			{
				lex_nextchar();
			}
			return;
		}

		else if (ismultiply())
		{
			char lex_remCH = nowCH;
			while (1)
			{
				lex_nextchar();
				if (lex_remCH == '*' && nowCH == '/')
				{
					break;
				}
				lex_remCH = nowCH;
			}
			return;
		}
		else
		{
			lex_ID = lex_divide_id;
			lex_rollbackCH();
		}
		break;
	}
	case 4:
	{
		lex_sym += nowCH;
		lex_ID = lex_percent_id;
		break;
	}
	case 5://小于号要考虑小于等于，同理大于号，等号，非号
	{
		lex_sym += nowCH;
		lex_nextchar();
		if (isequal())
		{
			lex_sym += nowCH;
			lex_ID = lex_lessequal_id;
		}
		else
		{
			lex_ID = lex_less_id;
			lex_indexOfLine--;
		}
		break;
	}
	case 6:
	{
		lex_sym += nowCH;
		lex_nextchar();
		if (isequal())
		{
			lex_sym += nowCH;
			lex_ID = lex_greaterequal_id;
		}
		else
		{
			lex_ID = lex_greater_id;
			lex_indexOfLine--;
		}
		break;
	}
	case 7://叹号
	{
		lex_sym += nowCH;
		lex_nextchar();
		if (isequal())
		{
			lex_sym += nowCH;
			lex_ID = lex_notequal_id;
		}
		break;
	}

	case 8:
	{
		lex_sym += nowCH;
		lex_nextchar();
		{
			if (isequal())
			{
				lex_sym += nowCH;
				lex_ID = lex_equalequal_id;
			}
			else
			{
				lex_ID = lex_equal_id;
				lex_indexOfLine--;
			}
		}
		break;
	}
	case 9:
	{
		lex_sym += nowCH;
		lex_ID = lex_comma_id;
		break;
	}
	case 10:
	{
		lex_sym += nowCH;
		lex_ID = lex_semicolon_id;
		break;
	}
	case 11:
	{
		lex_sym += nowCH;
		lex_ID = lex_colon_id;
		break;
	}

	case 12://单引号要考虑匹配问题
	{
		lex_nextchar();
		lex_sym = to_string_lex(int(nowCH));
		lex_ID = lex_NotZeroHeadNum_id;
		lex_nextchar();
		if (!issinglemark())
		{
			lex_sym = "error";
			lex_ID = lex_error_id;
		}
		break;
	}
	case 13://双引号要考虑匹配问题
	{
		lex_sym += nowCH;
		while (1)
		{
			lex_nextchar();
			if (isdoublemark())
			{
				break;
			}
			lex_sym += nowCH;
		}
		lex_sym += nowCH;
		lex_ID = lex_string_id;
		break;
	}

	case 14://括号类考虑匹配问题
	{
		lex_sym += nowCH;
		lex_ID = lex_leftparenthesis_id;
		break;
	}
	case 15:
	{
		lex_sym += nowCH;
		lex_ID = lex_rightparenthesis_id;
		break;
	}
	case 16:
	{
		lex_sym += nowCH;
		lex_ID = lex_leftmiddle_id;
		break;
	}
	case 17:
	{
		lex_sym += nowCH;
		lex_ID = lex_rightmiddle_id;
		break;
	}
	case 18:
	{
		lex_sym += nowCH;
		lex_ID = lex_leftbraces_id;
		break;
	}
	case 19:
	{
		lex_sym += nowCH;
		lex_ID = lex_rightbraces_id;
		break;
	}
	case 20:
	{
		lex_sym += nowCH;
		lex_ID = lex_dot_id;
		break;
	}
	case 21://字母分两种情况，一种是关键字，一种是用户自定义的标识符
	{
		lex_sym += nowCH;
		while (1)
		{
			lex_recordCH();
			lex_nextchar();
			if (!isletter() && !isnumber())
			{
				lex_rollbackCH();
				break;
			}
			lex_sym += nowCH;
		}
		for (int i = 0; i < reskeywordNum; i++)
		{
			//cout<<lex_sym<<endl;
			//cout<<reskeyword[i]<<endl;
			if (lex_sym == reskeyword[i])
			{
				lex_ID = i + 1;
				break;
			}
		}
		if (lex_ID == 0)
		{
			lex_ID = lex_indentifer_id;
		}
		break;
	}
	case 22://字母分是否是0开头
	{
		if (iszero())
		{
			lex_ID = lex_ZeroHeadNum_id;
		}
		else
		{
			lex_ID = lex_NotZeroHeadNum_id;
		}
		lex_sym += nowCH;
		while (1)
		{
			lex_recordCH();
			lex_nextchar();
			if (!isnumber())
			{
				lex_rollbackCH();
				break;
			}
			lex_sym += nowCH;
		}
		if (lex_sym.length() == 1)
		{
			lex_ID = lex_NotZeroHeadNum_id;
		}
		break;
	}
	}

	word One_Word;
	One_Word.lex_symbol = lex_sym;
	One_Word.lex_symbol_ID = lex_ID;
	One_Word.lex_indexOfFile = lex_indexOfFile;
	One_Word.lex_indexOfLine = lex_indexOfLine;
	//cout << lex_sym << endl;
	Lex_Table.push_back(One_Word);//存到lex_symbolTable中
	write();
}

void Lex_Aanlysis_main(string File)
{
	myFile = Readin(File);
	Read_File(myFile);
	//Init();
	lex_nextchar();
	while (!fileISend)
	{
		lex_Analysis();
	}
	return;
}