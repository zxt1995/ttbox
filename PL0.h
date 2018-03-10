#include <stdio.h>

#define NRW        21     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       18     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array
#define PXMAX      10
#define MAXSYM     30     // maximum number of symbols  

#define STACKSIZE  1000   // maximum storage
#define MAXDIM 1000

enum symtype
{
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
    SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	SYM_OR,
	SYM_AND,
	SYM_NOT,
	SYM_MOD,
	SYM_NOR,
	SYM_bitAND,
	SYM_bitOR,
	SYM_FOR,
	SYM_ELSE,
	SYM_EXIT,
	SYM_BREAK,
	SYM_ARRAY,
	SYM_RSPAREN,
	SYM_LSPAREN,
	SYM_RETURN,
	SYM_CONTINUE,
	SYM_SWITCH,
	SYM_CASE,
	SYM_COLON,
	SYM_DEFAULT,
	SYM_LABEL,
	SYM_GOTO,
	SYM_LGPAREN,
	SYM_RGPAREN
};


enum idtype
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE, ID_PARAMETER_A, ID_PARAMETER_V
};

enum opcode
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC, LDA, STA, MOV , JNP, POP, PRT, LEA
};//加入了lda，sta用来载入写入数组元素

enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_MOD, OPR_NOR, OPR_OR,
	OPR_AND, OPR_NOT, OPR_CMP
};

int true_list[5][10] = { 0 };//真值链
int false_list[5][10] = { 0 };//假值链
int true_level[5] = { 0 };//
int false_level[5] = { 0 };//
int offset = 0;
int not_cx = 0;//是否not开头

int break_cx[5] = { -1,-1,-1,-1,-1};//break_cx数组最多支持5层嵌套
int continue_cx[5] = { -1,-1,-1,-1,-1};//同上
int case_cx = -1;//case前jpc
int case_sta_cx = -1;//case后语句结束jmp
int default_cx = -1;//记录default出现的位置
int default_fi = 0;//标志是否出现default
int loopfi = 0;//记录循环层数or switch中case出现次数


char goto_label[10][5] = { '\0' };//原标识符
int goto_fi = 0;//原标志符个数
char goto_start[10][5] = { '\0' };//goto后面的标识符
int goto_si = 0;//goto后标志符个数

int gt_start_cx[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
int gt_end_cx[10]= { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };

typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
const char* err_msg[] =
{
/*  0 */    "",
/*  1 */    "Found ':=' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",
/*  7 */    "Statement expected.",
/*  8 */    "Follow the statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "A constant or variable can not be called.",
/* 16 */    "'then' expected.",
/* 17 */    "';' or 'end' expected.",
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a factor.",
/* 24 */    "The symbol can not be as the beginning of an expression.",
/* 25 */    "The number is too great.",
/* 26 */    "Wrong parameters num",
/* 27 */    "",
/* 28 */    "",
/* 29 */    "",
/* 30 */    "",
/* 31 */    "",
/* 32 */    "There are too many levels."
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx;         // index of current instruction to be generated.
int  level = 0;
int  tx = 0;
int  ax = 0;//数组的计数
int	 px = 0;//函数
int cx_if;//if_else
int else_swit;
int	 paranum[5];
int  blocked_cx[5];
char line[400];

instruction code[CXMAX];

typedef struct {
	char  name[MAXIDLEN + 1];
	int   kind;
} para;

const char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while","for","else","exit","break","return","continue","switch","case","default","goto"
};

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE, SYM_FOR,SYM_ELSE,SYM_EXIT,SYM_BREAK,SYM_RETURN,
	SYM_CONTINUE,SYM_SWITCH,SYM_CASE,SYM_DEFAULT,SYM_GOTO
};

int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,SYM_NOT,SYM_MOD,SYM_NOR,
	SYM_LSPAREN,SYM_RSPAREN,SYM_COLON, SYM_LGPAREN, SYM_RGPAREN
};

char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';','!','%','^','[',']',':', '{', '}'
};

#define MAXINS   15
const char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC","LDA","STA","MOV","JNP","POP", "PRT", "LEA"
};

typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
	int  num;
} comtab;

comtab table[TXMAX];

typedef struct
{
	char  name[MAXIDLEN + 1];
	int   kind;
	short level;
	short address;
	int	  num;
} mask;


typedef struct
{
	char name[MAXIDLEN + 1];
	int sum;
	int n;
	int dim[MAXDIM];
	int size[MAXDIM];
	int addr;
} arr;

//数组结构
arr array_t, array_table[TXMAX];

FILE* infile;

// EOF PL0.h
