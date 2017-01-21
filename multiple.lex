%{
/*This flex program parses Washington Post, NY Times, ABC News, and Reuters Urls*/
#include <stdio.h>
#include <string.h>
extern int agencyState;
extern char *yybuf;
void writeText();
%}
%s WSH WSHSTORY REUTERS NYT ABC
%x WUMIA REUTSTORY NYTSTORY ABCSTORY
%option noyywrap
%%
	BEGIN(agencyState);

[^<]+   ;
"<"     ;

<ABC>"<"p.itemprop=[^<]+">"  {BEGIN(ABCSTORY);}
<ABC>"<h3>"  {writeText();BEGIN(ABCSTORY);}
<ABCSTORY>[^<]+ {yymore();}
<ABCSTORY>"<"   {yymore();}
<ABCSTORY>"</p>"  {yyless(yyleng-4);if(yyleng>10){writeText(); strcat(yybuf, "&#10;");} BEGIN(ABC);}
<ABCSTORY>"</h3>" {writeText();BEGIN(ABC);}

<NYT>"<"p.class=\"story.body.text[^>]+">"    {BEGIN(NYTSTORY);}
<NYTSTORY>[^<]+             {writeText();}
<NYTSTORY>"<"               {writeText();}
<NYTSTORY>"</p>"            {strcat(yybuf, "&#10;&#10;"); BEGIN(NYT);}

<REUTERS>"<"span.id=\"article-text\". {writeText();BEGIN(REUTSTORY);}
<REUTSTORY>[^<]+   {writeText();}
<REUTSTORY>"<"     {writeText();}
<REUTSTORY>"<"script.type=\"text.javascript\"     {BEGIN(REUTERS);}
<REUTSTORY>"<p></p>"	; /*ignore empty paragraphs*/
<REUTSTORY>"</p>"	{writeText();strcat(yybuf, "&#10;");}

<WSH>"<"span.class=\"pb-caption\"">"	{BEGIN(WSHSTORY);}
<WSHSTORY>"<p>"  {writeText();BEGIN(WUMIA);}
<WUMIA>[^<]+   {writeText();}
<WUMIA>"<"     {writeText();}
<WUMIA>"</p>"  {writeText();strcat(yybuf, "&#10;&#10;");}
<WUMIA>"<div"  {BEGIN(WSH);}

%%

void writeText(){
        if(yyleng + strlen(yybuf) < YY_BUF_SIZE){
                strcat(yybuf, yytext);
        }
}

