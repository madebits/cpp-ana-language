/* (c) Vasian CEPA 2002, http://madebits.com */
%{

#include "interpreter.h"

#define YYERROR_VERBOSE 1
#define SYM_NO_FUNC(s) if(s->type == SYM_FUNC) { yyerror("func name used as variable"); YYERROR; }
#define NOT_IMPL yyerror("not implemented"); YYERROR;

extern FILE* yyin;
extern int lineCount;

SYMREC* defineVar(char*);
STNODE* nodeDefVar(SYMREC*);
SYMREC* checkSym(char*);
STNODE* createAssignNode(SYMREC*, STNODE*);
STNODE* createFuncNode(char*, SEMTYPE);
STNODE* createCallFunNode(char*, STNODE*);
void createPreFunc(char*, SEMTYPE);
STNODE* nodeDefArray(SYMREC*, STNODE*);
STNODE* createAssignArrayNode(SYMREC*, STNODE*);
STNODE* createArrayNode(char*, STNODE* ae);

void showHelp();
void isValidExpr(STNODE*);
int errorFlag = 0;

SYMREC symInit[] = {
		"program", 	SYM_KEYWD, PROGRAM, NULL,
		"if", 		SYM_KEYWD, IF, NULL,
		"else", 	SYM_KEYWD, ELSE, NULL,
		"while",	SYM_KEYWD, WHILE, NULL,
		"for",		SYM_KEYWD, FOR, NULL,
		"real", 	SYM_KEYWD, REAL, NULL,
		"int", 		SYM_KEYWD, INT, NULL,
		"void",		SYM_KEYWD, VOID, NULL,
		"break",	SYM_KEYWD, BREAK, NULL,
		"and",		SYM_KEYWD, AND, NULL,
		"or",		SYM_KEYWD, OR, NULL,
		"not",		SYM_KEYWD, NOT, NULL,
		"return",	SYM_KEYWD, RETURN, NULL,
		"function",	SYM_KEYWD, FUNCTION, NULL,
		"dump",		SYM_IFUN, IDUMP, NULL,
		"print",	SYM_IFUN, IPRINT, NULL,
		"nl",		SYM_IFUN, IPRINTNL, NULL,
		"read",		SYM_IFUN, IREAD, NULL,
		"size",		SYM_IFUN, ISIZE, NULL,
		"len",		SYM_IFUN, ILEN, NULL,
		"exit",		SYM_IFUN, IEXIT, NULL,
		"random",	SYM_IFUN, IRAND, NULL,
		
		"RAND_MAX",	SYM_KEYWD, SRANDMAX, NULL,
		"INT_MAX",	SYM_KEYWD, SINTMAX, NULL,
		"INT_MIN",	SYM_KEYWD, SINTMIN, NULL,
		"REAL_MAX",	SYM_KEYWD, SREALMAX, NULL,
		"REAL_MIN",	SYM_KEYWD, SREALMIN, NULL,
		"PI",	SYM_KEYWD, SMATHPI, NULL,
	};

int contextSemType = SEM_UNDEF; /* tie */
SYMREC *fname = NULL;
const char *STR_MAIN = "main";

%}

%union {
	int 	vint;
	double	vdouble;
	char *string;
	STNODE *node;
}


%token <vint>		PROGRAM REAL INT IF ELSE WHILE RETURN VOID
%token <vint>		AND OR NOT FUNCTION BREAK FOR
%token <vint>		IPRINT IREAD IEXIT IDUMP ISIZE ILEN IPRINTNL IRAND
%token <vint>		SRANDMAX SINTMAX SINTMIN
%token <vdouble>	CREAL SMATHPI SREALMAX SREALMIN
%token <vint>		CINTEGER 
%token <string>		ID STRING

%type <node>	varidexpr endvarid startvarid startvaridlist varidlist vardecls decls vardec
%type <node>	onearg arglist funchead funcdec funcdecls arraybound vararrayexpr exprlist
%type <node>	expr stmt stmtlist block internfunc blockstart blockend returnExpr program 
%type <node>	interfuncexpr forstmtlist symexpr assignExpr
%type <string>	fid
%type <vint>	vartype

%nonassoc IFX
%nonassoc ELSE

%left '='
%left AND OR
%right NOT
%left GE LE EQ NE '>' '<'
%left	'+' '-'
%left	','
%left	'*' '/' '%'
%right	UMINUS

%start startana

%%

startana: program	{
				if(errorFlag) { return 1; }
				if(debug_sym){
					sy_dumpSymTable();
					str_dumpStrTable();
				}
				if(debug_ftable){
					in_dumpFunctions();
					puts("START PROGRAM");
					in_dumpSynTree($1, 0);
					puts("END PROGRAM\n\n");
				}
				if(debug_eval) puts("D: eval start\n");
				eval($1);
				if(debug_eval) puts("D: eval returned\n");
			}
	;

program: PROGRAM ID '{' decls '}'	{	
						STNODE *n;
						$$ = NULL;
						n = createCallFunNode((char*)STR_MAIN, NULL);
						if($4 != NULL)
							n = st_node(NODE_OPR, ';', 2, $4, n);
						$$ = n;
					}
	;

decls: vardecls funcpdecs funcdecls	{ $$ = $1; }
	| vardecls funcdecls		{ $$ = $1; }
	| vardecls			{ $$ = $1; }
	| funcpdecs funcdecls		{ $$ = NULL; }
	| funcdecls			{ $$ = NULL; }
	| /* empty */			{ $$ = NULL; }
	;

vardecls: vardecls vardec	{ $$ = st_node(NODE_OPR, ';', 2, $1, $2); }
	| vardec		{ $$ = $1; }
	;

vardec: vartype varidlist	{ $$ = $2; contextSemType = SEM_UNDEF; }
	;

vartype: REAL	{ $$ = SEM_REAL; contextSemType = SEM_REAL; }
	| INT	{ $$ = SEM_INT; contextSemType = SEM_INT; }
	;

varidlist: startvaridlist endvarid	{ $$ = st_node(NODE_OPR, ';', 2, $1, $2); }
	| endvarid			{ $$ = $1; }
	;

startvaridlist: startvaridlist startvarid	{ $$ = st_node(NODE_OPR, ';', 2, $1, $2); }
	| startvarid				{ $$ = $1; }
	;

startvarid: ID ','		{ $$ = NULL; $$ = nodeDefVar(defineVar($1)); }
	| ID varidexpr ','	{
					SYMREC *s;
					$$ = NULL;
					s = defineVar($1);
					$$ = st_node(NODE_OPR, ';', 2,
						nodeDefVar(s), createAssignNode(s, $2));
				}
	| ID vararrayexpr	{
					$$ = NULL;
					$$ = createArrayNode($1, $2);
					if($$ == NULL) yyerror("wrong array decl");

				}
	;
	
endvarid: ID ';'		{ $$ = NULL; $$ = nodeDefVar(defineVar($1)); }
	| ID varidexpr ';'	{
					SYMREC *s;
					$$ = NULL;
					s = defineVar($1);
					$$ = st_node(NODE_OPR, ';', 2,
						nodeDefVar(s), createAssignNode(s, $2));
				}
	| ID vararrayexpr ';'	{
					$$ = NULL;
					$$ = createArrayNode($1, $2);
					if($$ == NULL) yyerror("wrong array decl");
					/*
					SYMREC* s;
					STNODE* ab = $2;
					STNODE* as = NULL;
					$$ = NULL;
					s = defineVar($1);
					if($2 != NULL){
						if($2->value.vint == '='){
							ab = $2->op[0];
							as = $2->op[1];
							free($2->op);
							free($2);
						}
						$$ = nodeDefArray(s, ab);
						if(as != NULL)
							$$ = st_node(NODE_OPR, ';', 2,
								$$, createAssignArrayNode(s, as));
					} else {
						$$ = NULL;
						yyerror("wrong array decl");
					}
					*/
				}
	;

varidexpr: '=' expr	{
				$$ = NULL; isValidExpr($2);
				$$ = $2;
			}
	;

vararrayexpr: arraybound			{ $$ = $1; }
	| arraybound '=' '{' exprlist '}'	{ $$ = st_node(NODE_OPR, '=', 2, $1, $4); }
	;

arraybound: '[' exprlist ']'	{	
					$$ = $2;
				}
	;

exprlist: exprlist ',' expr	{	
					$$ = NULL; isValidExpr($3);
					$$ = st_node(NODE_OPR,';', 2, $1, $3);
				}
	| expr			{
					$$ = NULL; isValidExpr($1);
					$$ = $1;
				}
	;

funcpdecs: funcpdecs funcpdec 	{ }
	| funcpdec		{ }
	;

funcpdec: FUNCTION vartype ID ';'	{ createPreFunc($3, $2); }
	| FUNCTION VOID ID ';'		{ createPreFunc($3, SEM_VOID); }
	;

funcdecls: funcdecls funcdec	{ $$ = NULL; }	
	| funcdec		{ $$ = NULL; }
	;

funcdec: funchead arglist ')' block	{ $$ = NULL; $$ = fu_patchFunctionNode($1, $2, $4); fname = NULL; }
	;

funchead: vartype fid	{ $$ = NULL; $$ = createFuncNode($2, $1);  }
	| VOID fid	{ $$ = NULL; $$ = createFuncNode($2, SEM_VOID); }
	;
	
fid: ID '('	{ $$ = $1; }
	;
	
arglist: arglist ',' onearg	{ $$ = st_node(NODE_OPR,';', 2, $1, $3); }
	| onearg		{ $$ = $1; }
	| /*empty*/		{ $$ = NULL; }
	;

returnExpr: RETURN expr		{
					$$ = NULL; isValidExpr($2);
					if(fname == NULL){
						yyerror("return outside any function");
						YYERROR;
					}
					if(((STNODE*)fname->vint)->value.semType == SEM_VOID){
						yyerror("void func must not return value");
						YYERROR;
					}
					$$ = st_node(NODE_OPR, O_RET, 1,
						createAssignNode(fname, $2));
				}
	| RETURN		{
					$$ = NULL; 
					if(fname == NULL){
						yyerror("return outside any function");
						YYERROR;
					}
					if(((STNODE*)fname->vint)->value.semType != SEM_VOID){
						yyerror("return value missing");
						YYERROR;
					}
					$$ = st_node(NODE_OPR, O_RET, 0);
				}
	;

onearg: vartype ID		{ $$ = NULL; $$ = nodeDefVar(defineVar($2)); contextSemType = SEM_UNDEF; }
	| vartype ID '[' ']'	{
					$$ = NULL; 
					$$ = nodeDefVar(defineVar($2));
					switch($$->op[0]->value.semType){
						case SEM_INT: $$->op[0]->value.semType = SEM_RAINT; break;
						case SEM_REAL: $$->op[0]->value.semType = SEM_RAREAL; break;
					}
				}
	;

block: stmt				{ $$ = $1; }
	| '{' blockstart vardecls
		stmtlist blockend '}'	{
						$$ = st_node(NODE_OPR,';', 4, $2, $3, $4, $5);
					}
	| '{' blockstart vardecls
		blockend '}'		{
						
						$$ = st_node(NODE_OPR, ';', 3, $2, $3, $4);
						printf(" line(%d) Warning: no statements in block!\n",
							lineCount);
					}
	| '{' blockstart stmtlist
		blockend '}'		{
						$$ = st_node(NODE_OPR, ';', 3, $2, $3, $4);
					}
	| '{' blockstart /* empty */
		blockend '}'		{ 
						$$ = st_node(NODE_OPR, ';', 2, $2, $3);
					}
	;

blockstart:	{ $$ = st_node(NODE_OPR, O_INB, 0); } ;

blockend:	{ $$ = st_node(NODE_OPR, O_OUTB, 0); } ;

stmtlist: stmt 		{ $$ = $1; }
	| stmtlist stmt	{ $$ = st_node(NODE_OPR, ';', 2, $1, $2); }
	;
	
stmt: ';'			{ $$ = st_node(NODE_OPR, ';', 2, NULL, NULL); }
	| expr ';'		{ $$ = $1; }
	| assignExpr ';'	{ $$ = $1; }
	| IF '(' expr ')' block %prec IFX 	{ $$ = NULL; isValidExpr($3); $$ = st_node(NODE_OPR, O_IF, 2, $3, $5); }
	| IF '(' expr ')' block ELSE block 	{ $$ = NULL; isValidExpr($3); $$ = st_node(NODE_OPR, O_IF, 3, $3, $5, $7); }
	| WHILE '(' expr ')' block		{ $$ = NULL; isValidExpr($3); $$ = st_node(NODE_OPR, O_WHILE, 2, $3, $5); }
	| FOR '(' forstmtlist '|' expr
		'|' forstmtlist ')' block	{
							STNODE *n;
							STNODE *before = $3;
							STNODE *after = $7;
							$$ = NULL;
							isValidExpr($5);
							if(after != NULL){
								if(in_isBlockNode($9)){
									n = $9->op[$9->nops - 2];
									n = st_node(NODE_OPR, ';', 2, n, after);
									$9->op[$9->nops - 2] = n;
									n = $9;
								} else {
									n = st_node(NODE_OPR, ';', 2, $9, after);
									n = st_node(NODE_OPR, ';', 2, n,
										st_node(NODE_OPR, O_OUTB, 0));
									n = st_node(NODE_OPR, ';', 2,
										st_node(NODE_OPR, O_INB, 0), n);
								}
							} else n = $9;
							$$ = st_node(NODE_OPR, O_WHILE, 2, $5, n);
							if(before != NULL)
								$$ = st_node(NODE_OPR, ';', 2, before, $$);
							//puts("for");
							//in_dumpSynTree($$, 0);
						}
	| internfunc ';'			{ $$ = $1; }
	| returnExpr ';'			{ $$ = $1; }
	| BREAK ';'				{ $$ = st_node(NODE_OPR, O_BREAK, 0); }
	| error ';'				{ $$ = NULL; yyerrok; }
	;

assignExpr : ID '=' expr	{ 
					SYMREC *s;
					$$ = NULL; 
					isValidExpr($3);
					s = checkSym($1);
					if(s == NULL) YYERROR;
					SYM_NO_FUNC(s)
					else $$ = createAssignNode(s, $3);
					$$->value.semType = $3->value.semType;
				}
	| ID '[' exprlist ']' '=' expr	{
							SYMREC *s;
							$$ = NULL;
							isValidExpr($6);
							s = checkSym($1);
							if(s == NULL) YYERROR;
							SYM_NO_FUNC(s);
							$$ = st_node(NODE_OPR, '=', 3, sym2node(s), $3, $6);
							$$->value.semType = $6->value.semType;
							//NOT_IMPL
						}
	;

forstmtlist: stmtlist	{ $$ = $1; }
	| /* empty */	{ $$ = NULL; }
	;
	
internfunc: IPRINT '(' expr ')'	{ $$ = NULL; isValidExpr($3); $$ = st_node(NODE_OPR, O_PRINT, 1, $3); }
	| IPRINT '(' STRING ')'	{ $$ = st_node(NODE_OPR, O_PRINT, 1, st_node(NODE_STR, str_addStr($3), 0)); }
	| IREAD '(' ID ')'		{
						SYMREC *s;
						$$ = NULL;
						s = checkSym($3);
						if(s == NULL) YYERROR;
						SYM_NO_FUNC(s)
						$$ = st_node(NODE_OPR, O_READ, 1, sym2node(s));
					}
	| IEXIT	'(' ')'			{ $$ = st_node(NODE_OPR, O_EXIT, 0); }
	| IDUMP	'(' ')'			{ $$ = st_node(NODE_OPR, O_DUMP, 0); }
	| IPRINTNL '(' ')'		{ $$ = st_node(NODE_OPR, O_PRINT, 1, st_node(NODE_STR, str_getNEWLINE(), 0)); }
	;
	
interfuncexpr: ISIZE '(' ID ')'		{
						SYMREC *s;
						$$ = NULL;
						s = checkSym($3);
						if(s == NULL) YYERROR;
						SYM_NO_FUNC(s)
						$$ = st_node(NODE_OPR, O_SIZE, 1, sym2node(s));
						$$->value.semType = SEM_INT;
					}
	| ISIZE	'(' ID ',' expr ')'	{
						SYMREC *s;
						$$ = NULL;
						s = checkSym($3);
						if(s == NULL) YYERROR;
						SYM_NO_FUNC(s)
						$$ = st_node(NODE_OPR, O_SIZE, 2, sym2node(s), $5);
						$$->value.semType = SEM_INT;
					}
	| ILEN '(' ID ')'		{
						SYMREC *s;
						$$ = NULL;
						s = checkSym($3);
						if(s == NULL) YYERROR;
						SYM_NO_FUNC(s)
						$$ = st_node(NODE_OPR, O_LEN, 1, sym2node(s));
						$$->value.semType = SEM_INT;
					}
	| IRAND	'(' ')'			{
						$$ = st_node(NODE_OPR, O_RAND, 0);
						$$->value.semType = SEM_INT;
					}
	;

expr: CINTEGER 			{ 
					$$ = st_node(NODE_CON, $1, 0);
					$$->value.semType = SEM_INT;
				}
	| CREAL			{
					$$ = st_node(NODE_CON, 0, 0);
					$$->value.vdouble = $1;
					$$->value.semType = SEM_REAL;
				}
	| ID			{
					SYMREC *s;
					$$ = NULL; 
					s = checkSym($1);
					if(s == NULL) YYERROR;
					SYM_NO_FUNC(s)
					else {
						$$ = sym2node(s);
					}
				}
	| assignExpr		{ $$ = $1; }
	| '-' expr %prec UMINUS {
					$$ = NULL; isValidExpr($2); 
					$$ = st_node(NODE_OPR, 'u', 1, $2);
					$$->value.semType = $2->value.semType;
				}
	| expr '+' expr		{
					$$ = NULL; $$ = st_node(NODE_OPR, '+', 2, $1, $3);
					$$->value.semType = se_semType($1->value.semType,
						$3->value.semType);
				}
	| expr '-' expr		{ 	
					$$ = NULL; $$ = st_node(NODE_OPR, '-', 2, $1, $3);
					$$->value.semType = se_semType($1->value.semType,
						$3->value.semType);
				}
	| expr '*' expr		{ 	
					$$ = NULL; $$ = st_node(NODE_OPR, '*', 2, $1, $3);
					$$->value.semType = se_semType($1->value.semType,
						$3->value.semType);
				}
	| expr '/' expr		{ 	
					$$ = NULL; $$ = st_node(NODE_OPR, '/', 2, $1, $3);
					$$->value.semType = se_semType($1->value.semType,
						$3->value.semType);
				}
	| expr '%' expr		{ 	
					$$ = NULL; $$ = st_node(NODE_OPR, '%', 2, $1, $3);
					$$->value.semType = se_semType($1->value.semType,
						$3->value.semType);
				}
	| expr '<' expr		{	
					$$ = NULL; $$ = st_node(NODE_OPR, '<', 2, $1, $3);
					$$->value.semType = SEM_INT;
				}
	| expr '>' expr		{	
					$$ = NULL; $$ = st_node(NODE_OPR, '>', 2, $1, $3);
					$$->value.semType = SEM_INT;
				}
	| expr GE expr		{	
					$$ = NULL; $$ = st_node(NODE_OPR, O_GE, 2, $1, $3);
					$$->value.semType = SEM_INT;
				}
	| expr LE expr		{	
					$$ = NULL; $$ = st_node(NODE_OPR, O_LE, 2, $1, $3);
					$$->value.semType = SEM_INT;
				}
	| expr NE expr		{	
					$$ = NULL; $$ = st_node(NODE_OPR, O_NE, 2, $1, $3);
					$$->value.semType = SEM_INT;
				}
	| expr EQ expr		{	
					$$ = NULL; $$ = st_node(NODE_OPR, O_EQ, 2, $1, $3);
					$$->value.semType = SEM_INT;
				}
	| expr AND expr		{
					$$ = NULL; $$ = st_node(NODE_OPR, O_AND, 2, $1, $3);
					$$->value.semType = SEM_INT;
				}
	| expr OR expr		{
					$$ = NULL; $$ = st_node(NODE_OPR, O_OR, 2, $1, $3);
					$$->value.semType = SEM_INT;
				}
	| NOT expr		{
					$$ = NULL; isValidExpr($2); 
					$$ = st_node(NODE_OPR, O_NOT, 1, $2);
					$$->value.semType = SEM_INT;
				}
	| '(' expr ')'		{ $$ = NULL; isValidExpr($2); $$ = $2; }
	| ID '(' exprlist ')'	{ $$ = NULL; $$ = createCallFunNode($1, $3); }
	| ID '(' ')'		{ $$ = NULL; $$ = createCallFunNode($1, NULL); }
	| ID '[' exprlist ']'	{
					STNODE* n;
					SYMREC *s;
					$$ = NULL;
					s = checkSym($1);
					if(s == NULL) YYERROR;
					SYM_NO_FUNC(s)
					$$ = st_node(NODE_OPR, O_GETA, 2, sym2node(s), $3);
					$$->value.semType = SEM_VOID;
				}
	| interfuncexpr		{ $$ = $1; }
	| symexpr		{ $$ = $1; }
	;

symexpr : SRANDMAX 		{
					$$ = st_node(NODE_CON, SRANDMAX, 0);
					$$->value.semType = SEM_INT;
					$$->value.vint = RAND_MAX;
				}
	| SMATHPI		{
					$$ = st_node(NODE_CON, SMATHPI, 0);
					$$->value.semType = SEM_REAL;
					$$->value.vdouble = 3.14159265358979323846;
				}
	| SINTMAX		{
					$$ = st_node(NODE_CON, SINTMAX, 0);
					$$->value.semType = SEM_INT;
					$$->value.vint = INT_MAX;
				}				
	| SINTMIN		{
					$$ = st_node(NODE_CON, SINTMIN, 0);
					$$->value.semType = SEM_INT;
					$$->value.vint = INT_MIN;
				}				
	| SREALMAX		{
					$$ = st_node(NODE_CON, SREALMAX, 0);
					$$->value.semType = SEM_REAL;
					$$->value.vdouble = DBL_MAX;
				}				
	| SREALMIN		{
					$$ = st_node(NODE_CON, SREALMIN, 0);
					$$->value.semType = SEM_REAL;
					$$->value.vdouble = DBL_MIN; /* -DBL_MAX; */
				}				

%%


int main(int argc, char** argv){
	int i, r;
	int initRandom = 1;
	double ver;
	#include "version.txt"
	fprintf(stderr, "*** AnA Demo Interpreter. Version %1.2lf ***\n", ver);
	fprintf(stderr, "* (c) 2002 Vasian CEPA, http://madebits.com *\n\n");
	for(i = 1; i < argc; i++){
		char *arg = argv[i];
		if((arg[0] == '-') || (arg[0] == '/')){
			char c = toupper(arg[1]);
			char c2 = toupper(arg[2]);
			switch(c){
			case 'D':	
					switch(c2){
					case 'M': debug_mem++;
							if(debug_mem > 1) debug_eval = 1;
							break;
					case 'S': debug_sym++; break;
					case 'E': debug_eval++; break;
					case 'T': debug_stree++; break;
					case 'F': debug_ftable++; break;
					case 'A': debug_access++; break;
					case 'C': debug_fcall++; break;
					case 'X': debug_fatal++; break;
					default: showHelp();
					}
					break;
			case 'T': 	yydebug = 1; break;
			case 'N':	print_name = 0; break;
			case 'R':	initRandom = 0; break;
			default:	showHelp();
			}	
		} else {
			char* file = NULL;
			int len = strlen(arg);
			if((len < 5)
				|| ( (len >= 5) && (strcmp(arg + len - 4, ".ana") != 0) )
			){
				file = (char*)malloc(len + 5);
				if(file == NULL) fatal(STR_OUT_OF_MEMORY);
				sprintf(file, "%s.ana", arg);
				file[len + 5 - 1] = '\0';
			}
			else file = arg;
			yyin = fopen(file, "r");
			if(yyin == NULL){
				printf("File not found: %s", file);
				if(file != arg) free(file);
				return 1;
			}
			if(file != arg) free(file);
		}
	}
	// for random
	srand((unsigned)(initRandom ? time(NULL) : 1));
	sy_initSymTable(vLength(symInit));
	r = yyparse();
	freeMemory();
	return r;
}

int yyerror(char * err){
	errorFlag = 1;
	fprintf(stderr, " line(%d) ", lineCount);
	printf("err: %s\n", err);
	return 0;
}

void showHelp(){
	puts("Usage: ana [-dm -ds -de -dt -df -dc -da -dx -t -n -r -?] file.ana");
	puts("\t-?  this help");
	puts("\t-d* various debug options:");
	puts("\t\tm memory (verbose)");
	puts("\t\t  better use dump() inside code");
	puts("\t\ts symbol table");
	puts("\t\te interpreter eval calls");
	puts("\t\tt syntax tree each eval (verbose)");
	puts("\t\tf function table");
	puts("\t\ta access #");
	puts("\t\tc function calls");
	puts("\t\tx fatal messages");
	puts("\t\t  use with -dm");
	puts("\t-t  parser  (verbose)");
	puts("\t-n  do not print var names in print()");
	puts("\t-r  do not initialize seed in random()");
	puts(" # this option can be repeteated");
	puts("\nThis software is provided as is.");
	puts("No warranty is made upon its use.");
	exit(1);
}

/* ----------------------- */

STNODE* createArrayNode(char *name, STNODE* ae){
	SYMREC* s;
	STNODE* ab = ae;
	STNODE* as = NULL;
	STNODE* n = NULL;
	s = defineVar(name);
	if(ae != NULL){
		if(ae->value.vint == '='){
			ab = ae->op[0];
			as = ae->op[1];
			free(ae->op);
			free(ae);
		}
		n = nodeDefArray(s, ab);
		if(as != NULL)
			n = st_node(NODE_OPR, ';', 2,
				n, createAssignArrayNode(s, as));
	} else return NULL;
	return n;
}


SYMREC* defineVar(char *name){
	SYMREC* temp = sy_getSym(name);
	if(temp == NULL){
		temp = sy_putSym(name, SYM_VAR);
		if(temp == NULL) fatal(STR_OUT_OF_MEMORY);
	}
	free(name);
	return temp;
}

STNODE* nodeDefVar(SYMREC* s){
	STNODE *n = sym2node(s);
	n->value.semType = contextSemType;
	return st_node(NODE_OPR, O_ADDV, 1, n);
}

STNODE* nodeDefArray(SYMREC* s, STNODE *eList){
	STNODE *n = sym2node(s);
	switch(contextSemType){
		case SEM_INT: n->value.semType = SEM_AINT; break;
		case SEM_REAL: n->value.semType = SEM_AREAL; break;
	}
	return st_node(NODE_OPR, O_ADDA, 2, n, eList);
}

SYMREC* checkSym(char* name){
	SYMREC* s;
	s = sy_getSym(name);
	if(s == NULL){
		printf("[%s] ", name);
		free(name);
		yyerror("undefined");
		//return NULL;
		exit(1);
	}
	free(name);
	return s;
}

STNODE* createAssignNode(SYMREC* s, STNODE* e){
	return st_node(NODE_OPR, '=', 2, sym2node(s), e);
}

STNODE* createAssignArrayNode(SYMREC* s, STNODE* e){
	return st_node(NODE_OPR, O_INIA, 2, sym2node(s), e);
}

void createPreFunc(char *name, SEMTYPE type){
	STNODE *n = NULL;
	SYMREC *s = sy_getSym(name);
	if(s != NULL){
		printf("%s ", name);
		free(name);
		yyerror("symbol redefinition\n");
		return;
	} else {
		s = sy_putSym(name, SYM_PRE);
		free(name);
		if(s == NULL) fatal(STR_OUT_OF_MEMORY);
		n = st_createEmptyNode(NODE_FUN);
		n->value.vint = (int)s; /* to find its name */
		s->vint = (int)n; /* save func node to sym table */
		n->value.semType = type;
	}
	contextSemType = SEM_UNDEF;
}

STNODE* createFuncNode(char* name, SEMTYPE type){
	STNODE *n = NULL;
	SYMREC *s = sy_getSym(name);
	if(s != NULL){
		if(s->type != SYM_PRE){
			printf("%s ", name);
			free(name);
			yyerror("symbol redefinition");
			return NULL;
		} else {
			free(name);
			n = (STNODE*)s->vint;
			s->type = SYM_FUNC;
			if(n->value.semType != type){
				yyerror("function type redefined");
				return NULL;
			}
		}
	} else {
		if((strcmp(name, STR_MAIN) == 0) && (type != SEM_VOID)){
			yyerror("main must be void");
			return NULL;
		}
		s = sy_putSym(name, SYM_FUNC);
		free(name);
		if(s == NULL) fatal(STR_OUT_OF_MEMORY);
	}
	if(n == NULL){
		n = st_createEmptyNode(NODE_FUN);
		n->value.vint = (int)s; /* to find its name */
		s->vint = (int)n; /* save func node to sym table */
	}
	n->value.semType = type;
	fname = s;
	contextSemType = SEM_UNDEF;
	return n;
}


STNODE* createCallFunNode(char* name, STNODE* exprList){
	SYMREC *fname;
	STNODE *n;
	if(name == NULL) return NULL;
	fname = checkSym(name);
	if(fname == NULL) return NULL;
	n = st_node(NODE_OPR, O_CALL, 2, (STNODE*)fname->vint, exprList);
	n->value.semType = n->op[0]->value.semType;
	if(n->value.semType == SEM_VOID) n->value.semType = SEM_UNDEF;
	return n;
}

void isValidExpr(STNODE* e){
	if(e == NULL) return;
	if(e->value.semType == SEM_UNDEF){
		in_dumpSynTree(e, 0);
		yyerror("no val used as expr");
		return;
	}
}