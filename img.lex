%{
/* This flex program parses the top image from a news url. It will need the library -lfl when compiled
** because it will act as a standalone (using the trivial main() program provided by -lfl) and 
** compiled into the executable img.out which is invoked by img.sh
** To compile: first do "flex img.lex" (this creates lex.yy.c) 
** and then do: "gcc -o img.out lex.yy.c -L/home4/nerillos/opt/flex-2-5-39/lib -lfl"
** img.sh is in turn invoked by createNewsCron.c using the popen system call:
** popen("bash img.sh '$url'", "r");  -- where $url gets substituted by the url to be parsed.
*/
#include <stdio.h>
%}
%x STORY
%s STRT
%option noyywrap
%%
	BEGIN(STRT);
[^=]+   ;
"="     ;

<STRT>=\"image_src\".href=\" {BEGIN(STORY); /* CNN  */}
<STRT>=\"og:image\"[ ]+content=["'] {BEGIN(STORY); /* ABC, NYT, US TODAY, WSH POST, RPP */}
<STRT>=\"og:image\".item[pP]rop=\"image\".content=\" {BEGIN(STORY); /* CNBC  */}
<STORY>[^"']+   {ECHO;}
<STORY>["']     {printf("\n");BEGIN(INITIAL);}

%%
