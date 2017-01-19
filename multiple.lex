%{
/*This flex program parses Washington Post and Reuters Urls*/
#include <stdio.h>
#include <string.h>
extern int agencyState;
extern char *yybuf;
void writeText();
%}
%s WSH WSHSTORY REUTERS
%x WUMIA REUTSTORY
%option noyywrap
%%
	BEGIN(agencyState);

[^<]+   ;
"<"     ;

<WSH>"<"span.class=\"pb-caption\"">"	{BEGIN(WSHSTORY);}
<WSHSTORY>"<p>"  {writeText();BEGIN(WUMIA);}
<WUMIA>[^<]+   {writeText();}
<WUMIA>"<"     {writeText();}
<WUMIA>"</p>"  {writeText();strcat(yybuf, "&#10;&#10");}
<WUMIA>"<div"  {BEGIN(WSH);}

<REUTERS>"<"span.id=\"article-text\". {writeText();BEGIN(REUTSTORY);}
<REUTSTORY>[^<]+   {writeText();}
<REUTSTORY>"<"     {writeText();}
<REUTSTORY>"<"script.type=\"text.javascript\"     {BEGIN(REUTERS);}
<REUTSTORY>"<p></p>"	; /*ignore empty paragraphs*/
<REUTSTORY>"</p>"	{writeText();strcat(yybuf, "&#10;");}

%%

void writeText(){
        if(yyleng + strlen(yybuf) < 16384){
                strcat(yybuf, yytext);
        }
}

