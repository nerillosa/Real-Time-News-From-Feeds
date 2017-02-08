%{
/*
	This flex program parses various news agencies web pages.
	If you modify this file you must do:
	
	flex --header-file=flex.h multiple.lex (creates new lex.yy.c)
	cp lex.yy.c .. (copy to the folder where your main program resides)
	cp flex.h ..  (optional if you added new start states)
	cd ..  (go to the folder where your main program resides)
	gcc createNewsCron.c lex.yy.c -o createNews -lcurl `mysql_config --cflags --libs`
*/

#include <stdio.h>
#include <string.h>
#define BUF_LEN 32768
extern int agencyState;
extern char *yybuf;
void writeText();
void writeString(char *str);
%}
%s WSH WSHSTORY REUTERS NYT ABC USTODAY SIMPLE CNN FOX CNBC PERU21 GESTION GIMPLE PIMPLE WIMPLE WSJ UPI
%x WUMIA REUTSTORY NYTSTORY ABCSTORY SIMPLESTORY USTODAYSTORY CNNSTORY CNBCSTORY PERU21STORY GESTIONSTORY WSJSTORY UPISTORY
%option noyywrap
%%
	BEGIN(agencyState);

[^<]+   ;
"<"     ;

<ABC>"<"p.itemprop=[^<]+">"  {BEGIN(ABCSTORY);}
<ABC>"<h3>"  {writeText();BEGIN(ABCSTORY);}
<ABCSTORY>[^<]+ {yymore();}
<ABCSTORY>"<"   {yymore();}
<ABCSTORY>"</p>"  {yyless(yyleng-4);if(yyleng>10 && !strstr(yytext,"<strong>")){writeText(); strcat(yybuf,"&#10;");} BEGIN(ABC);}
<ABCSTORY>"</h3>" {writeText();BEGIN(ABC);}

<UPI>"<"article.itemprop  {BEGIN(PIMPLE);}
<PIMPLE>"<".article">"  {BEGIN(INITIAL);}
<PIMPLE>"<p>"   {BEGIN(UPISTORY);}
<UPISTORY>[^<]+  {writeText();}
<UPISTORY>"<"    {writeText();}
<UPISTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(PIMPLE);}

<WSJ>"<"div.class=.wsj.snippet.body.">"  {BEGIN(WIMPLE);}
<WSJSTORY>"</div>"  {writeString("&#10;&#10;");BEGIN(INITIAL);}
<WIMPLE>"<p>"   {BEGIN(WSJSTORY);}
<WSJSTORY>[^<]+  {writeText();}
<WSJSTORY>"<"    {writeText();}
<WSJSTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(WIMPLE);}

<CNBC>"<p>"[^a-zA-Z0-9" ]  ;
<CNBC>"<p>"   {BEGIN(CNBCSTORY);}
<CNBC>"<p>YOUR BROWSER IS NOT SUPPORTED.</p>"  {BEGIN(INITIAL);}
<CNBCSTORY>[^<]+  {writeText();}
<CNBCSTORY>"<"    {writeText();}
<CNBCSTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(CNBC);}

<CNN>"<"p.class=\"zn-body..paragraph\"">"    {BEGIN(CNNSTORY);}
<CNN>"<"div.class=\"zn-body..paragraph\"">"  {BEGIN(CNNSTORY);}
<CNNSTORY>[^<]+    {writeText();}
<CNNSTORY>"<"      {writeText();}
<CNNSTORY>"</p>"   {writeString("&#10;&#10;"); BEGIN(CNN);}
<CNNSTORY>"</div>" {writeString("&#10;&#10;"); BEGIN(CNN);}

<FOX>"<"div.class=.article-text.">" {BEGIN(SIMPLE);}

<NYT>"<"p.class=\"story.body.text[^>]+">"    {BEGIN(NYTSTORY);}
<NYTSTORY>[^<]+             {writeText();}
<NYTSTORY>"<"               {writeText();}
<NYTSTORY>"</p>"            {writeString("&#10;&#10;"); BEGIN(NYT);}

<REUTERS>"<"span.id=\"article-text\". {writeText();BEGIN(REUTSTORY);}
<REUTSTORY>[^<]+   {writeText();}
<REUTSTORY>"<"     {writeText();}
<REUTSTORY>"<"script.type=\"text.javascript\"     {BEGIN(REUTERS);}
<REUTSTORY>"<p></p>"	; /*ignore empty paragraphs*/
<REUTSTORY>"</p>"	{writeText();writeString("&#10;");}

<USTODAY>"<p>"USA  ;
<USTODAY>"<p>"We.re.sorry   ;
<USTODAY>"<p>"something.went.wrong   ;
<USTODAY>"<p>"[^a-zA-Z0-9]  ;
<USTODAY>"<p>"   {BEGIN(USTODAYSTORY);}
<USTODAYSTORY>[^<]+  {writeText();}
<USTODAYSTORY>"<"    {writeText();}
<USTODAYSTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(USTODAY);}

<WSH>"<"span.class=\"pb-caption\"">"	{BEGIN(WSHSTORY);}
<WSHSTORY>"<p>"  {writeText();BEGIN(WUMIA);}
<WUMIA>[^<]+   {writeText();}
<WUMIA>"<"     {writeText();}
<WUMIA>"</p>"  {writeText();writeString("&#10;&#10;");}
<WUMIA>"<div"  {BEGIN(WSH);}

<SIMPLE>"<p>"[^a-zA-Z0-9]  ;
<SIMPLE>"<p>"   {BEGIN(SIMPLESTORY);}
<SIMPLESTORY>[^<]+  {writeText();}
<SIMPLESTORY>"<"    {writeText();}
<SIMPLESTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(SIMPLE);}

<PERU21>"<p>"[^a-zA-Z0-9]  ;
<PERU21>"<p>"       {BEGIN(PERU21STORY);}
<PERU21STORY>[^<]+  {yymore();}
<PERU21STORY>"<"    {yymore();}
<PERU21STORY>"</p>" {yyless(yyleng-4);if(yyleng>75){writeText(); writeString("&#10;&#10;");} BEGIN(PERU21);}

<GESTION>"<div class="  {BEGIN(GIMPLE);}
<GIMPLE>"<p>"[^a-zA-Z0-9]  ;
<GIMPLE>"<p>"Env  {BEGIN(INITIAL);}
<GIMPLE>"<p>"   {BEGIN(GESTIONSTORY);}
<GESTIONSTORY>[^<]+  {writeText();}
<GESTIONSTORY>"<"    {writeText();}
<GESTIONSTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(GIMPLE);}

%%

void writeText(){
        if(yyleng + strlen(yybuf) < BUF_LEN-512){
                strcat(yybuf, yytext);
        }
}

void writeString(char *str){
        if(strlen(str) + strlen(yybuf) < BUF_LEN-512){
                strcat(yybuf, str);
        }
}
