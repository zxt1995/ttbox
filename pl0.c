// pl0 compiler source code

#pragma warning(disable:4996)

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "PL0.h"
#include <stdarg.h>
#include "set.h"


symset uniteset(symset s1, symset s2)
{
	symset s;
	snode* p;

	s1 = s1->next;
	s2 = s2->next;

	s = p = (snode*)malloc(sizeof(snode));
	while (s1 && s2)
	{
		p->next = (snode*)malloc(sizeof(snode));
		p = p->next;
		if (s1->elem < s2->elem)
		{
			p->elem = s1->elem;
			s1 = s1->next;
		}
		else
		{
			p->elem = s2->elem;
			s2 = s2->next;
		}
	}

	while (s1)
	{
		p->next = (snode*)malloc(sizeof(snode));
		p = p->next;
		p->elem = s1->elem;
		s1 = s1->next;

	}

	while (s2)
	{
		p->next = (snode*)malloc(sizeof(snode));
		p = p->next;
		p->elem = s2->elem;
		s2 = s2->next;
	}

	p->next = NULL;

	return s;
} // uniteset

void setinsert(symset s, int elem)
{
	snode* p = s;
	snode* q;

	while (p->next && p->next->elem < elem)
	{
		p = p->next;
	}

	q = (snode*)malloc(sizeof(snode));
	q->elem = elem;
	q->next = p->next;
	p->next = q;
} // setinsert

symset createset(int elem, .../* SYM_NULL */)
{
	va_list list;
	symset s;

	s = (snode*)malloc(sizeof(snode));
	s->next = NULL;

	va_start(list, elem);
	while (elem)
	{
		setinsert(s, elem);
		elem = va_arg(list, int);
	}
	va_end(list);
	return s;
} // createset

void destroyset(symset s)
{
	snode* p;

	while (s)
	{
		p = s;
		p->elem = -1000000;
		s = s->next;
		free(p);
	}
} // destroyset

int inset(int elem, symset s)
{
	s = s->next;
	while (s && s->elem < elem)
		s = s->next;

	if (s && s->elem == elem)
		return 1;
	else
		return 0;
} // inset

  // EOF set.c
  //////////////////////////////////////////////////////////////////////
  // print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

  //////////////////////////////////////////////////////////////////////
void getch(void)	//得到下一个字符
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ((!feof(infile)) // added & modified by alex 01-02-09
			&& ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

  //////////////////////////////////////////////////////////////////////
  // gets a symbol from input stream.
void getsym(void)		//获取单词
{
	int i, k;
	char a[MAXIDLEN + 1];


	while (ch == ' ' || ch == '\t')
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		} while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
		{
			if (ch == '[')
				sym = SYM_ARRAY;		//symbol 数组，左中括号
			else
				sym = SYM_IDENTIFIER;   // symbol is an identifier
		}
	}//如果之后有【】进行数组操作
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		} while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_COLON;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else if (ch == ']')
	{
		getch();
		sym = SYM_RSPAREN;
	}
	else if (ch == '|')
	{
		getch();
		if (ch == '|')
		{
			sym = SYM_OR;
			getch();
		}
		else sym = SYM_OR;
	}
	else if (ch == '&')
	{
		getch();
		if (ch == '&')
		{
			sym = SYM_AND;
			getch();
		}
		else sym = SYM_bitAND;
	}
	else if (ch == '/')
	{
		getch();
		if (ch == '/')
		{ 
			cc = ll; 
			getch();
			getsym(); 
		} //行注释
		else if (ch == '*') // 块注释
		{
			getch(); 
			while (1)
			{
				if (ch == '*') 
				{
					getch();
					if (ch == '/') break;
				}
				getch();
			}
			getch();
			getsym();
		}
		else sym = SYM_SLASH;
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

  //////////////////////////////////////////////////////////////////////
  // generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

  //////////////////////////////////////////////////////////////////////
  // tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (!inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while (!inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

  //////////////////////////////////////////////////////////////////////
int dx,pdx = 3;  // data allocation index

		 // enter object(constant, variable or procedre) into table.
void enter(int kind, int initial)
{
	mask* mk;

	if ((kind == ID_PARAMETER_V) || (kind == ID_PARAMETER_A)) {
		tx++;
		paranum[level + 1]++;
		strcpy(table[tx].name, id);
		table[tx].kind = kind;
		mk = (mask *)&table[tx];
		mk->level = level + 1;
		mk->address = pdx++;
	}
	else {
		tx++;
		strcpy(table[tx].name, id);
		table[tx].kind = kind;
		switch (kind)
		{
		case ID_CONSTANT:
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			table[tx].value = num;
			break;
		case ID_VARIABLE:
			mk = (mask*)&table[tx];
			mk->level = level;
			mk->address = dx++;
			gen(LIT, 0, initial);
			gen(STO, 0, mk->address);
			break;
		case ID_PROCEDURE:
			mk = (mask*)&table[tx];
			mk->level = level;
			break;
		} // switch
	}
} // enter

  //////////////////////////////////////////////////////////////////////
  // locates identifier in symbol table.
void array_enter(int *aa)
{
	ax++; //printf("ax++");
	array_table[ax] = array_t;
	strcpy(array_table[ax].name, id);
	enter(ID_VARIABLE,aa[0]);
	array_table[ax].addr = tx;
	for (int i = array_table[ax].sum - 1; i > 0; i--)
		enter(ID_VARIABLE, aa[i]);				//划分空间
}//enter一个数组名

int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position


int array_position(char *id)
{
	int i = 0;
	while (strcmp(array_table[++i].name, id));
	if (i <= ax)
		return i;
	else
		return 0;
}//找到数组位置
  //////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT, 0);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	}
	else	error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

  //////////////////////////////////////////////////////////////////////
// void vardeclaration(void)
// {
// 	if (sym == SYM_IDENTIFIER)
// 	{
// 		enter(ID_VARIABLE);
// 		getsym();
// 	}
// 	else if (sym == SYM_ARRAY)
// 	{
// 		int dim = 0;
// 		while (ch == '[')
// 		{
// 			dim++;
// 			getch();
// 			getsym();
// 			array_t.dim[dim - 1] = num;
// 			getsym();
// 		}
// 		array_t.n = dim;
// 		array_t.size[dim - 1] = 1;
// 		for (int i = dim - 1; i > 0; i--)
// 			array_t.size[i - 1] = array_t.size[i] * array_t.dim[i];
// 		array_t.sum = array_t.size[0] * array_t.dim[0];
// 		array_enter();
// 		getsym();
// 	}//sym==symarray，是一个数组
// 	else
// 	{
// 		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
// 	}
// } // vardeclaration
//   //或许在这里加入定义数组的东西

  //////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU)
		{
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_VARIABLE, num);
			}
			getsym();
		}
		else {
			enter(ID_VARIABLE, 0);
		}
	}
	else if (sym == SYM_ARRAY)
	{
		int flag[20] = { 0 };
		int aa[1024] = { 0 };
		int u = 0;
		int flag1 = 0;
		int dim = 0;
		while (ch == '[')
		{
			dim++;
			getch();
			getsym();
			if (sym == SYM_RSPAREN)
			{
				array_t.dim[dim - 1] = 1;
				flag[dim - 1] = 1;
				flag1 = 1;
			}
			else {
				array_t.dim[dim - 1] = num;
				getsym();
			}
		}

		array_t.n = dim;
		getsym();
		if (sym == SYM_EQU)
		{
			int dim2;
			getsym();
			dim2 = 0;
			do
			{
				if (sym == SYM_LGPAREN)
				{
					dim2++;
					getsym();
				}
				else if (sym == SYM_RGPAREN)
				{
					dim2--;
					flag[dim2] = 0;
					getsym();
				}
				else if (sym == SYM_COMMA)
				{
					if (flag[dim2 - 1] == 1)
						array_t.dim[dim2 - 1]++;
					getsym();
				}
				else if (sym == SYM_NUMBER)
				{
					aa[u] = num;
					u++;
					getsym();
					//printf("aaa");
				}
				else {
					error(0);
				}
			} while (dim2 != 0);
		}

		array_t.size[dim - 1] = 1;
		for (int i = dim - 1; i > 0; i--)
			array_t.size[i - 1] = array_t.size[i] * array_t.dim[i];
		array_t.sum = array_t.size[0] * array_t.dim[0];
		//printf("%d   ", array_t.dim[1]);
		array_enter(aa);
		//getsym();
	}//sym==symarray，是一个数组
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration
  //或许在这里加入定义数组的东西

  //////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;

	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

  //////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	void  InerProc(int i, symset fsys);
	void  ex_expression(symset fsys);
	void  callproc(int i, symset fsys);
	int i;
	mask* mk = NULL;
	symset set;
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys)) {
		if (sym == SYM_IDENTIFIER) {
			if (!(i = position(id))) {
				error(11); // Undeclared identifier.
			}
			mk = (mask *)&table[i];
			switch (mk->kind) {
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					getsym();
					break;
				case ID_VARIABLE:
					gen(LOD, level - mk->level, mk->address);
					getsym();
					break;
				case ID_PARAMETER_V:
					gen(LOD, level - mk->level, mk->address);
					getsym();
					break;
				case ID_PARAMETER_A:
					gen(LOD, level - mk->level, mk->address);
					getsym();
					break;
				case ID_PROCEDURE:
					if(i > 2)
						callproc(i, fsys);
					else 
						InerProc(i, fsys);
					break;
			} // switch
		}
		else if (sym == SYM_ARRAY)
		{
			mask* mk;
			int dim = 0;
			if (!(i = array_position(id))) error(11);
			else 
			{
				int j = array_table[i].addr;

				mk = (mask*)&table[j];
				gen(LIT, 0, 0);
				while (ch == '[')
				{
					dim++;
					getch();
					getsym();
					set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
					ex_expression(set);
					destroyset(set);
					gen(LIT, 0, array_table[i].size[dim - 1]);
					gen(OPR, 0, OPR_MUL);
					gen(OPR, 0, OPR_ADD);
				}
				gen(LDA, level - mk->level, mk->address);
			}
			getsym();
		}//sym=array，数组
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			offset++;
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			ex_expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				for (int i = 0; i < 10; i++)
				{
					if (true_list[offset][i]) code[true_list[offset][i]].a = cx +1 ;
					true_list[offset][i] = 0;
				}
				offset--;
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if (sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{
			getsym();
			factor(fsys);
			gen(OPR, 0, OPR_NEG);
		}
		else if (sym == SYM_NOT)
		{
			getsym();
			ex_expression(fsys);
			gen(OPR, 0, OPR_NOT);
		}
		else if (sym == SYM_ODD) 
		{
			getsym();
			ex_expression(fsys);
			gen(OPR, 0, OPR_ODD);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
} // factor

  //////////////////////////////////////////////////////////////////////lab3 

void term(symset fsys)
{
  int mulop;
  symset set;

  set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
  factor(set);
  while (sym == SYM_TIMES || sym == SYM_SLASH) {
  mulop = sym;
  getsym();
  factor(set);
  if (mulop == SYM_TIMES) {
  gen(OPR, 0, OPR_MUL);
  }
  else {
  gen(OPR, 0, OPR_DIV);
  }
  } // while
  destroyset(set);
  } // term

void func_mod(symset fsys)////////////////
{
	int modop;
	symset set;

	set = uniteset(fsys, createset(SYM_MOD, SYM_NULL));
	term(set);
	while (sym == SYM_MOD)
	{
		modop = sym;
		getsym();
		term(set);
		gen(OPR, 0, OPR_MOD);
	} // while
	destroyset(set);
}
//////////////////////////////////////////////////////////////////////
void func_add(symset fsys)			//改表达式
{
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));

	func_mod(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		func_mod(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);//OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} //
void func_equ(symset fsys)////////////////
{
	int equop;
	symset set;

	set = uniteset(fsys, createset(SYM_GEQ, SYM_GTR,SYM_EQU,SYM_LEQ,SYM_NEQ,SYM_LES,SYM_NULL));
	func_add(set);
	while (sym == SYM_GEQ|| sym == SYM_GTR|| sym == SYM_EQU|| sym == SYM_LEQ|| sym == SYM_NEQ|| sym == SYM_LES)
	{
		equop = sym;
		getsym();
		func_add(set);
		if (equop == SYM_GEQ)
		{
			gen(OPR, 0, OPR_GEQ);
		}
		else if (equop == SYM_GTR)
		{
			gen(OPR, 0, OPR_GTR);
		}
		else if (equop == SYM_EQU)
		{
			gen(OPR, 0, OPR_EQU);
		}
		else if (equop == SYM_LEQ)
		{
			gen(OPR, 0, OPR_LEQ);
		}
		else if (equop == SYM_NEQ)
		{
			gen(OPR, 0, OPR_NEQ);
		}
		else
		{
			gen(OPR, 0, OPR_LES);
		}
	} // while
	destroyset(set);
}
void func_bitor(symset fsys)////////////////
{
	int norop;
	symset set;

	set = uniteset(fsys, createset(SYM_bitOR, SYM_NULL));
	func_equ(set);
	while (sym == SYM_bitOR)
	{
		norop = sym;
		getsym();
		func_equ(set);
		gen(OPR, 0, OPR_OR);
	} // while
	destroyset(set);
}
void func_nor(symset fsys)////////////////
{
	int norop;
	symset set;

	set = uniteset(fsys, createset(SYM_NOR, SYM_NULL));
	func_bitor(set);
	while (sym == SYM_NOR)
	{
		norop = sym;
		getsym();
		func_bitor(set);
		gen(OPR, 0, OPR_NOR);
	} // while
	destroyset(set);
}
void func_bitand(symset fsys)////////////////
{
	int norop;
	symset set;

	set = uniteset(fsys, createset(SYM_bitAND, SYM_NULL));
	func_nor(set);
	while (sym == SYM_bitAND)
	{
		norop = sym;
		getsym();
		func_nor(set);
		gen(OPR, 0, OPR_AND);
	} // while
	destroyset(set);
}

void func_and(symset fsys)////////////////
{
	int norop;
	symset set;
	//printf("%d\n", true_list);
	set = uniteset(fsys, createset(SYM_AND, SYM_NULL));
	func_bitand(set);
	while (sym == SYM_AND)
	{
		if (!not_cx)
		{
			false_list[offset][false_level[offset]] = cx;
			false_level[offset]++;
			gen(JPC, 0, 0);
		}
		else
		{
			true_list[offset][true_level[offset]] = cx;
			true_level[offset]++;
			for (int i = 0; i < 10; i++)
			{
				for (int j = 0; j < 5; j++)
					if (false_list[j][i])
					{
						code[false_list[j][i]].a = cx + 1;
						false_list[j][i] = 0;
					}
			}
			gen(JPC, 0, 0);
		}
		norop = sym;
		getsym();
		func_bitand(set);
		//gen(OPR, 0, OPR_AND);
	} // while
	destroyset(set);
}
void func_or(symset fsys)////////////////
{
	int norop;
	symset set;
	set = uniteset(fsys, createset(SYM_OR, SYM_NULL));
	func_and(set);
	while (sym == SYM_OR)
	{
		if (!not_cx)
		{
			true_list[offset][true_level[offset]] = cx;
			true_level[offset]++;
			for (int i = 0; i < 10; i++)//更新假值出口
			{
				for (int j = 0; j < 5; j++)
					if (false_list[j][i])
					{
						code[false_list[j][i]].a = cx ;
						false_list[j][i] = 0;
					}
			}
			gen(JNP, 0, 0);
		}
		else
		{
			false_list[offset][false_level[offset]] = cx;
			false_level[offset]++;
			gen(JNP, 0, 0);
		}
		norop = sym;
		getsym();
		func_and(set);
		//printf("or %d ", cx);
		//gen(OPR, 0, OPR_OR);
	} // while
	destroyset(set);
}
void ex_expression(symset fsys)///////////////
{
	symset set;
	set = uniteset(fsys, createset(SYM_NOT, SYM_RSPAREN, SYM_LSPAREN, SYM_RPAREN, SYM_NULL));
	if (sym == SYM_NOT)//
	{
		not_cx = 1;
		getsym();
		ex_expression(set);//递归调用！
		//gen(OPR, 0, OPR_NOT);
	} // while
	else  func_or(set);
	destroyset(set);
}
  //////////////////////////////////////////////////////////////////////
void expression(symset fsys) {	//不再使用归入ex_expressiong
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_RPAREN, SYM_NULL));

	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS) {
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS) {
			gen(OPR, 0, OPR_ADD);
		}
		else {
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // expression
//////////////////////////////////////////////////////////////////////
void condition(symset fsys)			//如何比较（比较归入ex_expression 不再使用condition）
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (!inset(sym, relset))
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition
void  callproc(int i, symset fsys) {
	int paranum = 0;
	mask* mk;
	mask* para_A;
	symset set = uniteset(fsys, createset(SYM_COMMA, SYM_NULL));

	mk = (mask *)&table[i];
	getsym();
	if (sym == SYM_LPAREN) {
		getsym();
		gen(LIT, 0, 0);
		if (sym == SYM_RPAREN)
			getsym();
		else {
			if(sym == SYM_bitAND) {
				getsym();
				int j = position(id);
				para_A = (mask *)&table[j];
				gen(LEA, level - para_A->level, para_A->address);
				getsym();
			}
			else 
				ex_expression(set);
			while (sym == SYM_COMMA) {
				paranum++;
				getsym();
				if(sym == SYM_bitAND) {
					getsym();
					int j = position(id);
					para_A = (mask *)&table[j];
					gen(LEA, level - para_A->level, para_A->address);
					getsym();
				}
				else
					ex_expression(set);
			}
			if (sym == SYM_RPAREN) {
				if(paranum == mk->num - 1)
					getsym();
				else
					error(26);
			}
			else
				error(22);
		}
		gen(CAL, level - mk->level, mk->address);
	}
	else
		error(22);
}

void callRandom(symset fsys)
{
	int t, m;

	getsym();
	srand(time(NULL));
	if (sym == SYM_LPAREN) {
		getsym();
		if (sym == SYM_RPAREN) {
			getsym();
			t = rand();
			gen(LIT, 0, t);
		}
		else {
			t = rand() % num;
			gen(LIT, 0, t);
			getsym();
			if (sym == SYM_RPAREN)
				getsym();
			else
				error(22);
		}
	}
	else
		error(22);
}

void callPrint(symset fsys)
{
	symset set1, set;
	set1 = createset(SYM_COMMA, SYM_NULL);
	set = uniteset(set1, fsys);
	getsym();
	if (sym == SYM_LPAREN) {
		getsym();
		if (sym == SYM_RPAREN) {
			getsym();
			gen(PRT, 0, 0);
		}
		else {
			ex_expression(set);
			gen(PRT, 0, 1);
			while (sym == SYM_COMMA) {
				getsym();
				ex_expression(set);
				gen(PRT, 0, 1);
			}
			if (sym == SYM_RPAREN) {
				getsym();
				gen(PRT, 0, 0);
			}
			else
				error(22);
		}
		destroyset(set1);
			destroyset(set);
	}
	else
		error(22);
}

void InerProc(int i, symset fsys)
{
	if(i == 1)
		callRandom(fsys);
	else
		callPrint(fsys);
}

  //////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx2,cx3;
	symset set1, set;

	if (sym == SYM_IDENTIFIER) { // variable assignment
		mask *mk = NULL;
		if (!(i = position(id))) {
			int j = 0;
			while (1)
			{
				char c = id[j];
				goto_label[goto_fi][j] = c;
				if (c == '\0') break;
				j++;
			}
			gt_end_cx[goto_fi] = cx;
			goto_fi++;
			j = 0;
			getsym();
			if(sym==SYM_COLON)
				getsym();
			statement(fsys);
		}
		mk = (mask *)&table[i];
		if (mk!=NULL &&(mk->kind == ID_VARIABLE || mk->kind == ID_PARAMETER_A || mk->kind == ID_PARAMETER_V)) {
			getsym();
			if (sym == SYM_BECOMES)
				getsym();
			else
				error(13); // ':=' expected.

			set1 = createset(SYM_COMMA, SYM_NULL);
			set = uniteset(set1, fsys);
			ex_expression(set);
			destroyset(set1);
			destroyset(set);
			gen(STO, level - mk->level, mk->address);
		}
		else if (mk != NULL && mk->kind == ID_PROCEDURE)
			if(i > 2)
				callproc(i, fsys);
			else
				InerProc(i, fsys);
		else {
			//error(28); // Wrong Identifier.
			i = 0;
		}
	}
	else if (sym == SYM_ARRAY)//sym=array
	{
		mask* mk;
		int j;
		if (!(i = array_position(id))) error(11);
		else
		{
			j = array_table[i].addr;
			mk = (mask*)&table[j];
			int dim = 0;
			gen(LIT, 0, 0);
			while (ch == '[')
			{
				dim++;
				getch();
				getsym();
				set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
				expression(set);
				destroyset(set);
				gen(LIT, 0, array_table[i].size[dim - 1]);
				gen(OPR, 0, OPR_MUL);
				gen(OPR, 0, OPR_ADD);
			}
		}
		getsym();
		if (sym == SYM_BECOMES)
			getsym();
		else error(13);
		set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
		ex_expression(set);
		destroyset(set);
		j = array_table[i].addr;
		mk = (mask*)&table[j];
		if (j)
			gen(STA, level - mk->level, mk->address);
	}//数组结束
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (!(i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*)&table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	}
	else if (sym == SYM_IF)//if else调用
	{ // if statement
		getsym();
		set1 = createset(SYM_LPAREN, SYM_ELSE, SYM_RPAREN, SYM_NULL);
		if (sym == SYM_LPAREN)
		{
			getsym();
		}
		set = uniteset(set1, fsys);
		ex_expression(set);//ok?
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error(16);
		}
		cx_if = cx;//记录当前的位置
		gen(JPC, 0, 0);//先假定一个跳转地址1 等待回填
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 5; j++)
				{
					if (!not_cx) {
						if (true_list[j][i]) code[true_list[j][i]].a = cx;
					}
					else
						if (false_list[j][i]) code[false_list[j][i]].a = cx;
			}
		}
		statement(fsys);
		if (else_swit == 0)
		{
			code[cx_if].a = cx;//暂定回填1(没else的时候)
			for (int i = 0; i < 10; i++)
			{
				for (int j = 0; j < 5; j++)
				{
					if (not_cx) {
						if (true_list[j][i]) code[true_list[j][i]].a = cx;
					}
					else 
						if (false_list[j][i]) code[false_list[j][i]].a = cx;
				}
			}
		}
		else_swit = 0;
	}
	else if (sym == SYM_ELSE)
	{
		else_swit = 1;
		getsym();
		cx2 = cx;
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				if (not_cx) {
					if (true_list[j][i]) code[true_list[j][i]].a = cx +1 ;
				}
				else
					if (false_list[j][i]) code[false_list[j][i]].a = cx + 1;
			}
		}
		code[cx_if].a = cx + 1;//有else的时候更新回填地址1 这是下一个指令(else)位置
		gen(JMP, 0, 0);//等待回填2
		statement(fsys);
		code[cx2].a = cx;//地址回填2
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_GOTO)
	{
		getsym();
		getsym();
		int j = 0;
		while (1)
		{
			char c = id[j];
			goto_start[goto_si][j] = c;
			if (c == '\0') break;
			j++;
		}
		gt_start_cx[goto_si] = cx;
		goto_si++;
		j = 0;
		gen(JMP, 0, 0);
	}
	else if (sym == SYM_WHILE )
	{ // while statement
		loopfi++;	//进入循环loop++
		cx1 = cx;
		getsym();
		set1 = createset(SYM_NULL);
		set = uniteset(set1, fsys);
		if (sym == SYM_LPAREN)
		{
			getsym();
		}
		ex_expression(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
		code[break_cx[loopfi]].a = cx;//回填break
		code[continue_cx[loopfi]].a = cx1;//回填continue
		break_cx[loopfi] = -1;//重置
		continue_cx[loopfi] = -1;
		if(loopfi > 0) loopfi--;//出循环loop--
	}
	else if (sym == SYM_DO)
	{ // while statement
		cx1 = cx;
		getsym();
		statement(fsys);
		cx3 = cx;
		if (sym == SYM_WHILE)//检测到while
		{
			getsym();
		}
		if (sym == SYM_LPAREN)
		{
			getsym();
		}
		set1 = createset(SYM_NULL);
		set = uniteset(set1, fsys);
		ex_expression(set);//处理为表达式
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);//为0跳转等待回填
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
		code[break_cx[loopfi]].a = cx;
		code[continue_cx[loopfi]].a = cx3;
		break_cx[loopfi] = -1;
		continue_cx[loopfi] = -1;
		if(loopfi > 0) loopfi--;
	}
	else if (sym == SYM_FOR)
	{ // for statement
		//break_cx++;以后会加上break的方案
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_LPAREN, SYM_RPAREN,SYM_NULL);
		set = uniteset(set1, fsys);
		if (sym == SYM_LPAREN)
			getsym();
		if (sym == SYM_IDENTIFIER)
		{
			i = position(id);
			getsym();
			if (sym == SYM_BECOMES)
				getsym();
			else
				error(13);
			ex_expression(set);
			if (i != 0)
			{
				mask* mk;
				mk = (mask*)&table[i];
				gen(STO, level - mk->level, mk->address);//将栈顶的值赋予该变量
			}
			if (sym == SYM_SEMICOLON)
			{	
				getsym();
				cx1 = cx;			//cx1记录比较表达式之前的位置 
				ex_expression(set);
				cx2 = cx;			//cx1记录比较表达式之后的位置
				gen(JPC, 0, 0);//等待回填出口
				if (sym == SYM_SEMICOLON)
					getsym();
				else
					error(18);
				int sign = 1;
				cx3 = cx;
				gen(JMP, 0, 0);//等待回填for内容入口 跳过步长表达式
				if (sym == SYM_IDENTIFIER)//步长表达式例如i:=1+1
				{
					getsym();
					if (sym == SYM_BECOMES)
						getsym();
					ex_expression(set);
					mask* mk;
					mk = (mask*)&table[i];
					gen(STO, level - mk->level, mk->address);
					gen(JMP, 0, cx1);
				}
				if (sym == SYM_RPAREN)
					getsym();
				code[cx3].a = cx;		//回填for内容入口给JMP指令
				statement(set);
				gen(JMP, 0, cx3+1);		//无条件跳转到步长表达式
				code[cx2].a = cx;		//循环出口回填JPC指令
				code[break_cx[loopfi]].a = cx;
				code[continue_cx[loopfi]].a = cx3+1;
				break_cx[loopfi] = -1;
				continue_cx[loopfi] = -1;
				if(loopfi > 0) loopfi--;
			}
		}
	}

	else if (sym == SYM_SWITCH)
	{ // while statement
		cx1 = cx;
		getsym();
		set1 = createset(SYM_NULL);
		set = uniteset(set1, fsys);
		if (sym == SYM_LPAREN)
		{
			getsym();
		}
		ex_expression(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		statement(fsys);
		if (case_cx>0 && default_fi == 0)
			code[case_cx].a = cx;
		//没有default语句出现的时候最后一个case不正确
		//就跳转到switch语句的结束
		default_fi = 0;
		for (int i = loopfi; i >= 0; i--)
		{
			code[break_cx[i]].a = cx;

			break_cx[loopfi] = -1;
		}
		loopfi = 0;
	}
	else if (sym == SYM_CASE)
	{
		loopfi++;
		if (case_cx > 0)
		{
			code[case_cx].a = cx + 1;
			case_sta_cx = cx;
			gen(JMP, 0, 0);
			//上个case语句的结尾 跳转到下个case语句的开始
		}
		getsym();
		if (sym == SYM_NUMBER)
		{
			getsym();
		}
		gen(LIT, 0, num);
		if (sym == SYM_COLON)
		{
			getsym();
		}
		gen(OPR, 0, OPR_CMP);
		case_cx = cx;
		gen(JPC, 0, 0);
		//这个case语句的条件判断结束跳转到下个case语句的开始
		code[case_sta_cx].a = cx;
		statement(fsys);
	}
	else if (sym == SYM_DEFAULT)
	{
		loopfi++;
		getsym();
		if (sym == SYM_COLON)
		{
			getsym();
		}
		if (case_cx > 0)
		{
			code[case_cx].a = cx;
			//如果有default语句将成为上一次case JPC的位置
		}
		default_fi = 1;
		statement(fsys);
	}
	else if (sym == SYM_BREAK)
	{
		break_cx[loopfi] = cx;
		gen(JMP, 0, 0);//无条件跳转等待回填
		getsym();//
	}
	else if (sym == SYM_CONTINUE)
	{
		continue_cx[loopfi] = cx;
		gen(JMP, 0, 0);//无条件跳转等待回填
		getsym();
	}

	else if (sym == SYM_EXIT)//EXIT 实现直接return就行
	{
		getsym();
		gen(OPR, 0, OPR_RET);
	}
	else if (sym == SYM_RETURN) {
		getsym();
		expression(fsys);
	}
	else
		test(fsys, phi, 19);
} // statement

void procdeclaration() {
	mask* mk;
	if (sym == SYM_IDENTIFIER) {
		enter(ID_PROCEDURE, 0);
		getsym();
		if (sym == SYM_LPAREN) { // '('
			getsym();
			while (sym == SYM_IDENTIFIER || sym == SYM_bitAND) {
				if(sym == SYM_IDENTIFIER)
					enter(ID_PARAMETER_V, 0);
				else {
					getsym();
					enter(ID_PARAMETER_A, 0);
				}
				getsym();
				if (sym == SYM_COMMA)
					getsym();
				else if (sym == SYM_RPAREN)
					break;
				else
					error(26);    // 参数错误
			}
			for (int i = tx - paranum[level +1]+1; i <= tx; ++i) {
				mk = (mask *)&table[i];
				gen(MOV, paranum[level + 1] - 1, mk->address);
			}
			mk = (mask *)&table[tx - paranum[level + 1]];
			mk->num = paranum[level + 1];
			if (sym == SYM_RPAREN) {
				getsym();
				if (sym == SYM_SEMICOLON)
					getsym();
				else
					error(5); // Missing ';'.
			}
			else
				error(22);  // Missing ')'.
		}
		else
			error(22);
	}
	else
		error(27);  // Identifier expected.
}
  //////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx, savedpx;
	symset set1, set;

	dx = pdx;
	pdx = 3;
	block_dx = dx;
	mk = (mask*)&table[tx- paranum[level]];
	if(!level)
		tx += 2;
	printf("%s\n", mk->name);
	
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; //save dx before handling procedure call!
		mk->address = cx;
		gen(JMP, 0, 0);
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			blocked_cx[level+1] = cx;

			savedTx = tx;
			procdeclaration();
			level++;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx + 1;
			level--;

			if (sym == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	} while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = blocked_cx[level];
	cx0 = cx - paranum[level+1];
	//  paranum = 0;
	gen(INT, 0, block_dx - paranum[level]);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, paranum[level], OPR_RET); // return
	paranum[level] = 0;
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(0, cx);
} // block

  //////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;

	while (levelDiff--)
		b = stack[b];
	return b;
} // base

  //////////////////////////////////////////////////////////////////////
  // interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 1;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];

		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				stack[b - i.l - 1] = stack[top];
				top = b - i.l - 1;
				pc = stack[top + i.l + 3];
				b = stack[top + i.l + 2];
				//            top = b - 1;
				//            pc = stack[top + 3];
				//            b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_NOR:
				top--;
				stack[top] ^= stack[top + 1];
				break;
			case OPR_OR:
				top--;
				if (stack[top + 1] || stack[top])
				{
					stack[top] = 1;
					break;
				}
				else
				{
					stack[top] = 0;
					break;
				}
			case OPR_NOT:						
				stack[top] = !stack[top];
				break;
			case OPR_AND:
				top--;
				if (stack[top + 1] == 0)
				{
					stack[top] = 0;
					break;
				}
				else if (stack[top] == 0)
				{
					stack[top] = 0;
					break;
				}
				else
				{
					stack[top] = 1;
					break;
				}
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
				///////////////////////////////////////10.8
			case OPR_MOD:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Moded by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] %= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			case OPR_CMP:
				stack[top] = stack[top] == stack[top - 1];
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		case JNP:
			if (stack[top] != 0)
				pc = i.a;
			top--;
			break;
		case LDA:
			stack[top] = stack[base(stack, b, i.l) + i.a + stack[top]];
			break;
		case STA:
			stack[base(stack, b, i.l) + i.a + stack[top - 1]] = stack[top];
			printf("%d\n", stack[top]);
			break;//加入了lda与sta指令
		case MOV:
			//  stack[base(stack, b, i.l) + i.a] = stack[top - i.l];
			stack[b + i.a] = stack[top - i.l];
			top++;
			break;
		case POP:
			printf(" #%d# ", stack[top]);
			top--;
			printf("#%d# ", stack[top]);
			break;
		case PRT:
			if(i.a)
				printf("%d\t", stack[top--]);
			else
				printf("\n");
			break;
		case LEA:
			stack[++top] = base(stack, b, i.l) + i.a;
			break;
		} // switch
	} while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

void inProc() 
{
	mask* mk;
	tx++;
	strcpy(table[tx].name, "random");
	table[tx].kind = ID_PROCEDURE;
	mk = (mask*)&table[tx];
	mk->level = level;

	tx++;
	strcpy(table[tx].name, "print");
	table[tx].kind = ID_PROCEDURE;
	mk = (mask*)&table[tx];
	mk->level = level;

	tx = 0;
}

  //////////////////////////////////////////////////////////////////////
int main()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;


	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}
	// printf("Please input source file name: "); // get file name to be compiled
	// scanf("%s", s);
	//if ((infile = fopen("example1.txt", "r")) == NULL)
	//{
	//	printf("File %s can't be opened.\n", s);
	//	exit(1);
	//}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);

	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE,SYM_LABEL,SYM_GOTO, SYM_NULL);
	facbegsys = createset(SYM_ARRAY, SYM_RPAREN,SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NOT, SYM_bitAND, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();
	inProc();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	for (int i = 0; i < goto_si; i++)//goto label进行字符串匹配
	{
		for (int j = 0; j < goto_fi; j++)
		{
			int k = 0;
			while (1)
			{
				char c = goto_start[i][k];
				char d = goto_label[j][k];
				if (c != d || k==5) break;
				k++;
			}
			if (k == 5)
				code[gt_start_cx[i]].a = gt_end_cx[j];
		}
	}

	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);

	return 0;
} // main

  //////////////////////////////////////////////////////////////////////
  // eof pl0.c
