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
int hasRightArrow();
%}
%s TIME WSH HACK WSHEXAM POLITICO HUFF REUTERS NYT ABC ABCIMPLE USTODAY SIMPLE CNN FOX CNBC PERU21 PERU PEMPLE GIMPLE PIMPLE WIMPLE WSJ UPI
%x TIMESTORY HACKSTORY WSHEXAMSTORY WUMIA POLITICOSTORY HUFFSTORY WSHSTORY REUTSTORY NYTSTORY PERUSTORY ABCSTORY SSTORY SIMPLESTORY USTODAYSTORY CNNSTORY CNBCSTORY PERU21STORY GESTIONSTORY WSJSTORY UPISTORY COMSTORY RPP RPPSTORY
%option noyywrap
%%
	BEGIN(agencyState);

[^<]+   ;
"<"     ;

<POLITICO>"<p class=\" story-text__paragraph\">"   {BEGIN(POLITICOSTORY);}
<POLITICOSTORY>[^<]+             {writeText();}
<POLITICOSTORY>"<"               {writeText();}
<POLITICOSTORY>"</p>"            {strcat(yybuf, "&#10;&#10;"); BEGIN(POLITICO);}

<WSHEXAM>"<p>"  {BEGIN(WSHEXAMSTORY);}
<WSHEXAMSTORY>[^<]+  {writeText();}
<WSHEXAMSTORY>"<"    {writeText();}
<WSHEXAMSTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(WSHEXAM);}

<HACK>"<p>"   {BEGIN(HACKSTORY);}
<HACK>"<p>Sign up"  {BEGIN(INITIAL);}
<HACKSTORY>[^<]+  {writeText();}
<HACKSTORY>"<"    {writeText();}
<HACKSTORY>"</p>" {strcat(yybuf, "&#10;&#10;"); BEGIN(HACK);}

<TIME>"<p>"   {BEGIN(TIMESTORY);}
<TIMESTORY>[^<]+  {writeText();}
<TIMESTORY>"<"    {writeText();}
<TIMESTORY>"</p>" {strcat(yybuf, "&#10;&#10;"); BEGIN(TIME);}

<HUFF>"<div class=\"content-list-component yr-content-list-text"[^>]+"><h3>" ;
<HUFF>"<div class=\"content-list-component yr-content-list-text"[^>]+">"   {BEGIN(HUFFSTORY);}
<HUFFSTORY>[^<]+             {writeText();}
<HUFFSTORY>"<"               {writeText();}
<HUFFSTORY>"<div data"[^>]+">"[ \t\n]* ;
<HUFFSTORY>"</p>"      {writeString("&#10;&#10;");}
<HUFFSTORY>[ \t\n]*"</div>"      {BEGIN(HUFF);}

<PERU>"<div id=\"nota_body\""   {BEGIN(PEMPLE);}
<PEMPLE>"<p>"   {BEGIN(PERUSTORY);}
<PERUSTORY>[^<]+             {writeText();}
<PERUSTORY>"<"               {writeText();}
<PERUSTORY>"</p>" {writeString("&#10;"); BEGIN(PEMPLE);}

<RPP>[^"]+   ;
<RPP>\"      ;
<RPP>\"articleBody\":.\"    {BEGIN(RPPSTORY);}
<RPPSTORY>[^"]+             {writeText();}
<RPPSTORY>\"                {BEGIN(RPP);}

<UPI>"<"article.itemprop  {BEGIN(PIMPLE);}
<PIMPLE>"<div class=\"read-more-block lato\">" {BEGIN(INITIAL);}
<PIMPLE>"<p>"\t\t\t\t {BEGIN(INITIAL);}
<PIMPLE>"<p>"   {BEGIN(UPISTORY);}
<PIMPLE>"</p>" {BEGIN(UPISTORY);}
<PIMPLE>"<slide" {BEGIN(INITIAL);}
<UPISTORY>[^<]+  {writeText();}
<UPISTORY>"<"    {writeText();}
<UPISTORY>"<div class=\"ad_slot\">"  {BEGIN(PIMPLE);}
<UPISTORY>"<div class='story-offset ad_slot'>"  {BEGIN(PIMPLE);}
<UPISTORY>"</p></p>" {BEGIN(INITIAL);}
<UPISTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(PIMPLE);}
<UPISTORY>"<slide" {BEGIN(INITIAL);}
<UPISTORY>"<div class=\"montserrat bold\""  {BEGIN(INITIAL);}

<ABC>"<"section.class=\"Article..Content   {BEGIN(ABCIMPLE);}
<ABCIMPLE>"<p>"   {BEGIN(ABCSTORY);}
<ABCIMPLE>"<p id=\""[^"]+"\">"  {BEGIN(ABCSTORY);}
<ABCSTORY>[^<]+             {writeText();}
<ABCSTORY>"<"               {writeText();}
<ABCSTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(ABCIMPLE);}

<WSJ>"<"div.class=.wsj.snippet.body.">"  {BEGIN(WIMPLE);}
<WSJSTORY>"</div>"  {writeString("&#10;&#10;");BEGIN(INITIAL);}
<WIMPLE>"<p>"   {BEGIN(WSJSTORY);}
<WSJSTORY>[^<]+  {writeText();}
<WSJSTORY>"<"    {writeText();}
<WSJSTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(WIMPLE);}

<CNBC>"<p>"[^a-zA-Z0-9&" <]  ;
<CNBC>"<p><input type=\"checkbox"   ;
<CNBC>"<p></p>"  ;
<CNBC>"<p>"   {BEGIN(CNBCSTORY);}
<CNBC>"<p>YOUR BROWSER IS NOT SUPPORTED.</p>"  {BEGIN(INITIAL);}
<CNBC>"<p><em><strong>Like this story"   {BEGIN(INITIAL);}
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
<CNNSTORY>"</p>"[ \t\n]+"<p>"  {writeString("&#10;&#10;");}
<CNNSTORY>"</p>"[ \t\n]+"<"div[ \t\n]+id=\"[a-zA-Z0-9_ ]+\""></div>"[ \t\n]+"<p>"  {writeString("&#10;&#10;");} 
<CNNSTORY>"</p>"   {writeString("&#10;&#10;"); BEGIN(CNN);}
<CNNSTORY>"</div>" {writeString("&#10;&#10;"); BEGIN(CNN);}
<CNNSTORY>"</h2>" {writeString("&#10;&#10;"); BEGIN(CNN);}

<FOX>"<"div.class=.article-body.">" {BEGIN(SIMPLE);}

<NYT>"<"p.class=\"css[^>]+">"   {BEGIN(NYTSTORY);}
<NYTSTORY>[^<]+             {yymore();}
<NYTSTORY>"<"               {yymore();}
<NYTSTORY>"</p>"            {if(strncmp(yytext, "By", 2)){writeText(); strcat(yybuf, "&#10;&#10;");} BEGIN(NYT);/*remove <p>s starting with By...*/}

<REUTERS>"<p class=\"Paragraph"[^>]+">"    {BEGIN(REUTSTORY);writeString("&#10;&#10;");}
<REUTERS>"<p><span>" ;
<REUTERS>"<p><a href" ;
<REUTSTORY>"<"span[^>]+">"   ;
<REUTSTORY>"</span>"   ;
<REUTSTORY>"<a href="[^>]+">"   ;
<REUTSTORY>"</a>"   ;
<REUTSTORY>"<h3>"   {writeString("&lt;b&gt;");}
<REUTSTORY>"</h3>"  {writeString("&lt;/b&gt;");}
<REUTSTORY>"</p>"   {BEGIN(REUTERS);}
<REUTSTORY>[^<]+    {writeText();}

<USTODAY>"<p class=gnt"[^>]+>  {BEGIN(USTODAYSTORY);}
<USTODAYSTORY>[^<]+  {writeText();}
<USTODAYSTORY>"<"    {writeText();}
<USTODAYSTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(USTODAY);}

<WSH>"<p class=\"font"[^>]+><b>How.we.got.here   {BEGIN(FOX);}
<WSH>"<p class=\"font"[^>]+><b>Read.more:   {BEGIN(FOX);}
<WSH>"<p class=\"font"[^>]+>   {BEGIN(WSHSTORY);}
<WSHSTORY>[^<]+             {writeText();}
<WSHSTORY>"<"               {writeText();}
<WSHSTORY>"</p>"            {writeString("&#10;&#10;"); BEGIN(WSH);}

<SIMPLE>"<p><i></i></p>"  ;
<SIMPLE>"<p>"[^b-ln-oq-z]+"</p>"  ;
<SIMPLE>"<p>&nbsp;</p>"  ;
<SIMPLE>"<p>"[^a-zA-Z0-9&<] ;
<SIMPLE>"<p>This material may not be published" ;
<SIMPLE>"<p><strong>"   {BEGIN(SIMPLESTORY);}
<SIMPLE>"<"p.class=\"speakable\"">"    {BEGIN(SIMPLESTORY);/*This is for first paragraphs of FOX NEWS*/}
<SIMPLE>"<"p.class=\"story-body[^"]+\"">" {BEGIN(SIMPLESTORY); /*First line of BBC NEWS */}
<SIMPLE>"<p>"   {BEGIN(SIMPLESTORY);}
<SIMPLESTORY>[^<]+  {writeText();}
<SIMPLESTORY>"<"    {writeText();}
<SIMPLESTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(SIMPLE);}
<SIMPLESTORY>"<style" {BEGIN(SSTORY);}
<SSTORY>[^<]+  ;
<SSTORY>"<"    ;
<SSTORY>"</style>" {BEGIN(SIMPLESTORY);}

<PERU21>"<"p.itemProp=\"description\".class=\"story-content[^>]+">"    {BEGIN(PERU21STORY);}
<PERU21STORY>[^<]+  {writeText();}
<PERU21STORY>"<"    {writeText();}
<PERU21STORY>"</p>" {writeString("&#10;&#10;"); BEGIN(PERU21);}

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

int hasRightArrow(){
	int n;
	char linda[20];
	for(n=0; n<6; n++)
		sprintf(linda+2*n, "%02x",(unsigned char)yytext[n]);
	*(linda+2*n) = '\0';
	if(!strcmp(linda, "3c623ee296ba")|| !strncmp(linda, "e296ba",6)){ /* <b>E296BA or E296BA */
		return 1;
	}
	return 0;
}

