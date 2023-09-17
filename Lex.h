#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

#include "head.h"
#include "Symbol_Table.h"

#define keywordNum    54
#define reskeywordNum 21

ifstream Readin(string FILE);
void Read_File(ifstream& File);
void Lex_Aanlysis_main(string File);

//extern string keyword[keywordNum];//关键字集合
extern string keywordID[keywordNum];//关键字ID

extern const string reskeyword[reskeywordNum];
extern const string reskeywordID[reskeywordNum];

extern vector<string> LEX_File;//源文件

#endif