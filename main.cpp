/****************************************************/
/* 文件: main.cpp                                   */
/* 功能: 入口函数（主函数）                           */
/* 用法: 运行此文件，并且输入测试用例的文件名           */
/* 一种针对C0文法的编译器                             */
/* 作者：唐家祺 西北工业大学计算机学院                 */
/* 单位: 西北工业大学计算机学院                       */
/****************************************************/

#include "head.h"
#include "LEX.h"
#include "Syntax.h"
#include "Symbol_Table.h"
#include "Asm_Code.h"
#include "Asm_Code_MIPS64.h"
#include "Error.h"
#include "Optim.h"
#include "Register_Assign.h"

using namespace std;

int main()
{
	printf("Please enter your test case: ");
	//读入文件
	string File;
	cin >> File;

	//前端
	//词法分析
	Lex_Aanlysis_main(File);
	printf("Lex analysis completed! Please type Enter to the next step\n");
	auto key=getch();
	
	//语法分析,语义检查,生成中间代码
	Syntax_Analysis_main();
	printf("Syntax analysis completed! Please type Enter to the next step\n");
	key=getch();

	//后端

	//优化器
	Optimize_main();
	printf("Optimized IR generated! Please type Enter to the next step\n");
	key=getch();

	//使用引用计数实现寄存器分配
	Register_Assign();

	//通过四元式生成MIPS汇编代码
	Asm_Code_main();
	Asm_Code_64_main();
	printf("ASM generated! This is the end of C0 Compiler.\n");
	key = getch();

	printf("That is the end of our C0 Compiler.\n");

	return 0;
}