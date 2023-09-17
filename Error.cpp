#include "head.h"
#include "Error.h"

using namespace std;

int width = 5;
bool Error_Flag = false;

void Grammar_Error(int err_ID, int err_line, int err_coloumn)
{
	Error_Flag = true;
	cout << setw(width) << left << "Error: " << setw(width) << left << err_ID;
	cout << setw(width) << left << "Line: " << setw(width) << left << err_line;
	cout << setw(width) << left << "Position: " << setw(width) << left << err_coloumn;
	cout << setw(width) << left << "Content: ";
	switch (err_ID)
	{
	case err_const_redefination: //恒定常量重复定义
		cout << "constant redefinition!" << endl;
		break;
	case err_var_redefination:  //变量重复定义
		cout << "variable redefinition!" << endl;
		break;
	case err_function_redefination:  //方程重复定义
		cout << "funtion redefinition!" << endl;
		break;
	case err_lacksemicolon:  //缺乏分号
		cout << "lack of \';\' !" << endl;
		break;
	case err_var_undefination:  //变量未定义
		cout << "variable undefined!" << endl;
		break;
	case err_function_undefination:  //方程未定义
		cout << "funtion undefined!" << endl;
		break;
	case err_const_assign:  //企图更改常量的值
		cout << "constant reassignment!" << endl;
		break;
	case err_addorsub:  //缺乏加减法运算符
		cout << "lack of \'+|-\' !" << endl;
		break;
	case err_mulordiv:  //缺乏乘除法运算符
		cout << "lack of \'*|/\' !" << endl;
		break;
	case err_relation:  //缺乏关系运算符
		cout << "lack of \'relation\' !" << endl;
		break;
	case err_const:  //缺乏常量定义运算符
		cout << "lack of \'const\' !" << endl;
		break;
	case err_intorchar:  //声明语句缺乏int或者char标识符
		cout << "lack of \'int | char\' !" << endl;
		break;
	case err_ass:  //缺乏等于运算符
		cout << "lack of \'=\' !" << endl;
		break;
	case err_number:  //数字未检测到
		cout << "should be a number !" << endl;
		break;
	case err_int:  //int未检测到
		cout << "should be an int type !" << endl;
		break;
	case err_identifier:  //标识符未检测到
		cout << "should be an identifier !" << endl;
		break;
	case err_leftparenthesis:  //缺少（
		cout << "lack of \'(\' !" << endl;
		break;
	case err_rightparenthesis:  //缺少 ）
		cout << "lack of \')\' !" << endl;
		break;
	case err_leftmiddle:  //缺少[
		cout << "lack of \'[\' !" << endl;
		break;
	case err_rightmiddle:  //缺少]
		cout << "lack of \']\' !" << endl;
		break;
	case err_leftbraces:  //缺少{
		cout << "lack of \'{\' !" << endl;
		break;
	case err_rightbraces:   //缺少}
		cout << "lack of \'}\' !" << endl;
		break;
	case err_void:  //缺少 void
		cout << "lack of \'void\' !" << endl;
		break;
	case err_main:  //缺少main
		cout << "lack of \'main\' !" << endl;
		break;
	case err_printf:  //缺少printf
		cout << "lack of \'printf\' !" << endl;
		break;
	case err_scanf:  //缺少scanf
		cout << "lack of \'scanf\' !" << endl;
		break;
	case err_colon:  //缺少:
		cout << "lack of \':\' !" << endl;
		break;
	case err_case:  //缺少case
		cout << "lack of \'case\' !" << endl;
		break;
	case err_switch:  //缺少switch
		cout << "lack of \'switch\' !" << endl;
		break;
	case err_while:  //缺少while
		cout << "lack of \'while\' !" << endl;
		break;
	case err_if:  //缺少 if
		cout << "lack of \'if\' !" << endl;
		break;
	default:  //默认错误
		cout << "default error!" << endl;
	}
	//goto Error;

}