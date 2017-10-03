/* (c) Vasian CEPA 2002 */
%{

#include "symtab.h"
#include "syntree.h"
#include "ana.tab.h"

int lineCount = 0;

%}

integer [0-9]+
real [0-9]+(\.[0-9]+)?((e|E)((\+)?|\-)[0-9]+)?

%%

"//".*\n			{ lineCount++; } /* skip */

[ \t]+				/* skip */

\n				{ lineCount++; } /* skip */

\"[^\"]*[\"]			{
					int len = strlen(yytext);
					yylval.string = (char*)malloc(len - 1);
					if(yylval.string != NULL){
						*(yytext + len - 1) = '\0';
						strcpy(yylval.string, yytext + 1);
					}
					return STRING;
				}

{integer}			{ yylval.vint = atoi(yytext); return CINTEGER; }

{real}				{ sscanf(yytext, "%lf", &yylval.vdouble); return CREAL; }

[_[:alpha:]][_[:alnum:]]*		{ 
					SYMREC* sym = sy_getSym(yytext);
					if(sym != (SYMREC*)0){
						switch(sym->type){
							case SYM_KEYWD:
								return sym->vint;
							case SYM_IFUN:
								yylval.vint = sym->vint;
								return yylval.vint;
						}
					}
					yylval.string = (char*)malloc(strlen(yytext) + 1);
					if(yylval.string != NULL)
						strcpy(yylval.string, yytext);
					return ID;
				}
				
"<="				return LE;

">="				return GE;

"=="				return EQ;

"!="				return NE;

.				{ return (int)(yytext[0]); } /* return all other chars to parser */

<<EOF>>				{ yyterminate(); }


%%

int yywrap(){
	return 1;
}
