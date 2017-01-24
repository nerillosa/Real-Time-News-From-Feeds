%{
/*This flex program parses Washington Post, NY Times, ABC News, and Reuters Urls*/
#include <stdio.h>
#include <string.h>
extern int agencyState;
extern char *yybuf;
void writeText();
%}
%s WSH WSHSTORY REUTERS NYT ABC USTODAY SIMPLE CNN FOX CNBC PERU21
%x WUMIA REUTSTORY NYTSTORY ABCSTORY SIMPLESTORY USTODAYSTORY CNNSTORY CNBCSTORY PERU21STORY
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

<CNBC>"<p>"[^a-zA-Z0-9" ]  ;
<CNBC>"<p>"   {BEGIN(CNBCSTORY);}
<CNBCSTORY>[^<]+  {writeText();}
<CNBCSTORY>"<"    {writeText();}
<CNBCSTORY>"</p>" {strcat(yybuf, "&#10;&#10;"); BEGIN(CNBC);}

<CNN>"<"p.class=\"zn-body..paragraph\"">"    {BEGIN(CNNSTORY);}
<CNN>"<"div.class=\"zn-body..paragraph\"">"  {BEGIN(CNNSTORY);}
<CNNSTORY>[^<]+    {writeText();}
<CNNSTORY>"<"      {writeText();}
<CNNSTORY>"</p>"   {strcat(yybuf, "&#10;&#10;"); BEGIN(CNN);}
<CNNSTORY>"</div>" {strcat(yybuf, "&#10;&#10;"); BEGIN(CNN);}

<FOX>"<"div.class=.article-text.">" {BEGIN(SIMPLE);}

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

<USTODAY>"<p>"USA  ;
<USTODAY>"<p>"We.re.sorry   ;
<USTODAY>"<p>"something.went.wrong   ;
<USTODAY>"<p>"[^a-zA-Z0-9]  ;
<USTODAY>"<p>"   {BEGIN(USTODAYSTORY);}
<USTODAYSTORY>[^<]+  {writeText();}
<USTODAYSTORY>"<"    {writeText();}
<USTODAYSTORY>"</p>" {strcat(yybuf, "&#10;&#10;"); BEGIN(USTODAY);}

<WSH>"<"span.class=\"pb-caption\"">"	{BEGIN(WSHSTORY);}
<WSHSTORY>"<p>"  {writeText();BEGIN(WUMIA);}
<WUMIA>[^<]+   {writeText();}
<WUMIA>"<"     {writeText();}
<WUMIA>"</p>"  {writeText();strcat(yybuf, "&#10;&#10;");}
<WUMIA>"<div"  {BEGIN(WSH);}

<SIMPLE>"<p>"[^a-zA-Z0-9]  ;
<SIMPLE>"<p>"   {BEGIN(SIMPLESTORY);}
<SIMPLESTORY>[^<]+  {writeText();}
<SIMPLESTORY>"<"    {writeText();}
<SIMPLESTORY>"</p>" {strcat(yybuf, "&#10;&#10;"); BEGIN(SIMPLE);}

<PERU21>"<p>"[^a-zA-Z0-9]  ;
<PERU21>"<p>"       {BEGIN(PERU21STORY);}
<PERU21STORY>[^<]+  {yymore();}
<PERU21STORY>"<"    {yymore();}
<PERU21STORY>"</p>" {yyless(yyleng-4);if(yyleng>75){writeText(); strcat(yybuf, "&#10;&#10;");} BEGIN(PERU21);}

%%

void writeText(){
        if(yyleng + strlen(yybuf) < YY_BUF_SIZE){
                strcat(yybuf, yytext);
        }
}

