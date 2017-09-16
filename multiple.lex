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
%s WSH WSHSTORY REUTERS NYT ABC USTODAY SIMPLE CNN FOX CNBC PERU21 GESTION GIMPLE PIMPLE WIMPLE WSJ UPI COMERCIO
%x WUMIA REUTSTORY NYTSTORY ABCSTORY SIMPLESTORY USTODAYSTORY CNNSTORY CNBCSTORY PERU21STORY GESTIONSTORY WSJSTORY UPISTORY COMSTORY RPP RPPSTORY
%option noyywrap
%%
	BEGIN(agencyState);

[^<]+   ;
"<"     ;

<RPP>[^"]+   ;
<RPP>\"      ;
<RPP>\"articleBody\":.\"    {BEGIN(RPPSTORY);}
<RPPSTORY>[^"]+             {writeText();}
<RPPSTORY>\"                {BEGIN(RPP);}

<ABC>"<"p.itemprop=[^<]+">"  {BEGIN(ABCSTORY);}
<ABC>"<h3>"  {writeText();BEGIN(ABCSTORY);}
<ABCSTORY>[^<]+ {yymore();}
<ABCSTORY>"<"   {yymore();}
<ABCSTORY>"</p>"  {yyless(yyleng-4);if(yyleng>10 && !strstr(yytext,"<strong>")){writeText(); writeString("&#10;");} BEGIN(ABC);}
<ABCSTORY>"</h3>" {writeText();BEGIN(ABC);}

<UPI>"<"article.itemprop  {BEGIN(PIMPLE);}
<PIMPLE>"<".article">"  {BEGIN(INITIAL);}
<PIMPLE>"<p>"   {BEGIN(UPISTORY);}
<PIMPLE>"</script>" {BEGIN(UPISTORY);}
<UPISTORY>"<script" {BEGIN(PIMPLE);}
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
<CNN>"<"p.class=\"zn-body..paragraph.speakable\"">"    {BEGIN(CNNSTORY);}
<CNN>"<"div.class=\"zn-body..paragraph.speakable\"">"  {BEGIN(CNNSTORY);}
<CNN>"<"p.class=\"speakable\"">"  {BEGIN(CNNSTORY);}
<CNN>"<"h2.class=\"speakable\"">" {BEGIN(CNNSTORY);}
<CNNSTORY>[^<]+    {writeText();}
<CNNSTORY>"<"      {writeText();}
<CNNSTORY>"<h3>"   {writeString("&lt;b&gt;");}
<CNNSTORY>"</h3>"  {writeString("&lt;/b&gt;");}
<CNNSTORY>"<cite class=".+"</cite>"    ;
<CNNSTORY>"</p>"   {writeString("&#10;&#10;"); BEGIN(CNN);}
<CNNSTORY>"</div>" {writeString("&#10;&#10;"); BEGIN(CNN);}
<CNNSTORY>"</h2>" {writeString("&#10;&#10;"); BEGIN(CNN);}

<FOX>"<"div.class=.article-body.">" {BEGIN(SIMPLE);}

<NYT>"<"p.class=\"story.body.text[^>]+"><em>"    ;
<NYT>"<"p.class=\"story.body.text[^>]+">"    {BEGIN(NYTSTORY);}
<NYTSTORY>[^<]+             {writeText();}
<NYTSTORY>"<"               {writeText();}
<NYTSTORY>"</p>"            {writeString("&#10;&#10;"); BEGIN(NYT);}

<COMERCIO>"<"p.class=\"parrafo.first.parrafo[^>]+">"      {BEGIN(COMSTORY);}
<COMSTORY>[^<]+             {writeText();}
<COMSTORY>"<"               {writeText();}
<COMSTORY>"</p>"            {writeString("&#10;&#10;"); BEGIN(COMERCIO);}

<REUTERS>"<"p.data.reactid=\"[0-9]{2}\"">" {writeText();BEGIN(REUTSTORY);}
<REUTSTORY>[^<]+   {writeText();}
<REUTSTORY>"<"     {writeText();}
<REUTSTORY>"<p></p>"	; /*ignore empty paragraphs*/
<REUTSTORY>"</p>"	{writeText();writeString("&#10;&#10;");BEGIN(REUTERS);}

<USTODAY>"<p>"USA  ;
<USTODAY>"<p>"We.re.sorry   ;
<USTODAY>"<p>"something.went.wrong   ;
<USTODAY>"<p>"[^a-zA-Z0-9]  ;
<USTODAY>"<"p.class=\"speakable-p-..p-text\"">"    {BEGIN(USTODAYSTORY);}
<USTODAY>"<"p.class=["']speakable-p-..p-text["']">"    {BEGIN(USTODAYSTORY);}

<USTODAY>"<"p.class=["']p-text["']"><strong>"  {writeString("<strong>");BEGIN(USTODAYSTORY);}
<USTODAY>"<"p.class=["']p-text["']"><b>"  {writeString("<b>");BEGIN(USTODAYSTORY);}
<USTODAY>"<"p.class=["']p-text["']"><span style=\"line-height:normal\">"  {writeString("<span>");BEGIN(USTODAYSTORY);}


<USTODAY>"<"p.class=["']p-text["']"><"    ;
<USTODAY>"<"p.class=["']p-text["']">"    {BEGIN(USTODAYSTORY);}
<USTODAY>"<"p.class=\"MsoNoSpacing\"">"    {BEGIN(USTODAYSTORY);}
<USTODAY>"<"h2.class=\"presto.h2\"">" {writeString("&lt;b&gt;");BEGIN(USTODAYSTORY);}
<USTODAY>"<"h3.class=\"presto.h3\"">" {writeString("&lt;b&gt;");BEGIN(USTODAYSTORY);}
<USTODAY>"<"h4.class=\"presto.h4\"">" {writeString("&lt;b&gt;");BEGIN(USTODAYSTORY);}
<USTODAY>"<p>"   {BEGIN(USTODAYSTORY);}
<USTODAYSTORY>[^<]+  {writeText();}
<USTODAYSTORY>"<"    {writeText();}
<USTODAYSTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(USTODAY);}
<USTODAYSTORY>"</h2>" {writeString("&lt;/b&gt;&#10;"); BEGIN(USTODAY);}
<USTODAYSTORY>"</h3>" {writeString("&lt;/b&gt;&#10;"); BEGIN(USTODAY);}
<USTODAYSTORY>"</h4>" {writeString("&lt;/b&gt;&#10;"); BEGIN(USTODAY);}

<WSH>"<"span.class=\"pb-caption\"">"	{BEGIN(WSHSTORY);}
<WSHSTORY>"<p"[ ]  ;
<WSHSTORY>"<p>"  {writeText();BEGIN(WUMIA);}
<WUMIA>[^<]+   {writeText();}
<WUMIA>"<"     {writeText();}
<WUMIA>"</p>"  {writeText();writeString("&#10;&#10;");BEGIN(WSHSTORY);}
<WUMIA>"<div"  {BEGIN(WSH);}

<SIMPLE>"<p>&nbsp;</p>"  ;
<SIMPLE>"<p><i></i></p>"  ;
<SIMPLE>"<p>This material may not be published" ;
<SIMPLE>"<"p.class=\"speakable\"">"    {BEGIN(SIMPLESTORY);/*This is for first paragraphs of FOX NEWS*/}
<SIMPLE>"<"p.class=\"story-body[^"]+\"">" {BEGIN(SIMPLESTORY); /*First line of BBC NEWS */}
<SIMPLE>"<p>"   {BEGIN(SIMPLESTORY);}
<SIMPLESTORY>[^<]+  {writeText();}
<SIMPLESTORY>"<"    {writeText();}
<SIMPLESTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(SIMPLE);}

<PERU21>"<p class=\"parrafo first-parrafo\""  {writeText(); BEGIN(PERU21STORY);}
<PERU21STORY>[^<]+  {writeText();}
<PERU21STORY>"<"    {writeText();}
<PERU21STORY>"</p>" {writeString("&#10;&#10;"); BEGIN(PERU21);}

<GESTION>"<div class="  {BEGIN(GIMPLE);}
<GIMPLE>"<p>"[^a-zA-Z0-9]  ;
<GIMPLE>"<p>"Env  {BEGIN(INITIAL);}
<GIMPLE>"<p>"   {BEGIN(GESTIONSTORY);}
<GESTIONSTORY>[^<]+  {writeText();}
<GESTIONSTORY>"<"    {writeText();}
<GESTIONSTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(GIMPLE);}

%%

void writeText(){
        if(yyleng + strlen(yybuf) < BUF_LEN-1){
                strcat(yybuf, yytext);
        }
}

void writeString(char *str){
        if(strlen(str) + strlen(yybuf) < BUF_LEN-1){
                strcat(yybuf, str);
        }
}
