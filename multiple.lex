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
%s WSH POLITICO POLIT HUFF REUTERS ABC USTODAY SIMPLE CNN FOX CNBC GIMPLE PIMPLE WIMPLE PERU UPI PEMPLE REPUBLICA
%x POLITICOSTORY HUFFSTORY WSHSTORY REUTSTORY ABCSTORY SIMPLESTORY USTODAYSTORY CNNSTORY CNBCSTORY PERUSTORY UPISTORY REPUBLICASTORY
%option noyywrap
%%
	BEGIN(agencyState);

[^<]+   ;
"<"     ;

<POLITICO>"</figcaption>"   {BEGIN(POLIT);}
<POLIT>"<p class=\" story-text__paragraph\">"   {BEGIN(POLITICOSTORY);}
<POLITICOSTORY>[^<]+             {writeText();}
<POLITICOSTORY>"<"               {writeText();}
<POLITICOSTORY>"</p>"            {strcat(yybuf, "&#10;&#10;"); BEGIN(POLIT);}

<HUFF>"<div class=\"content-list-component yr-content-list-text"[^>]+"><h3>" ;
<HUFF>"<div class=\"content-list-component yr-content-list-text"[^>]+">"   {BEGIN(HUFFSTORY);}
<HUFFSTORY>[^<]+             {writeText();}
<HUFFSTORY>"<"               {writeText();}
<HUFFSTORY>"<div data"[^>]+">"[ \t\n]* ;
<HUFFSTORY>"</p>"      {writeString("&#10;&#10;");}
<HUFFSTORY>[ \t\n]*"</div>"      {BEGIN(HUFF);}

<ABC>"<"article.class=\"Article..Content   {BEGIN(WIMPLE);}
<WIMPLE>"<p>"   {BEGIN(ABCSTORY);}
<ABCSTORY>[^<]+             {writeText();}
<ABCSTORY>"<"               {writeText();}
<ABCSTORY>"</p>" {writeString("&#10;"); BEGIN(WIMPLE);}

<UPI>"<"article.itemprop  {BEGIN(PIMPLE);}
<PIMPLE>"<".article">"  {BEGIN(INITIAL);}
<PIMPLE>"<p>"   {BEGIN(UPISTORY);}
<PIMPLE>"</script>" {BEGIN(UPISTORY);}
<UPISTORY>"<script" {BEGIN(PIMPLE);}
<UPISTORY>[^<]+  {writeText();}
<UPISTORY>"<"    {writeText();}
<UPISTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(PIMPLE);}

<CNBC>"<p>"[^a-zA-Z0-9" <]  ;
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

<REUTERS>"<p class=\"\">"    {BEGIN(REUTSTORY);writeString("&#10;&#10;");}
<REUTERS>"<p><span>" ;
<REUTERS>"<p><a href" ;
<REUTERS>"<p>"    {BEGIN(REUTSTORY);writeString("&#10;&#10;");}
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

<FOX>"<"div.class=.article-body.">" {BEGIN(SIMPLE);}
<SIMPLE>"<p><i></i></p>"  ;
<SIMPLE>"<p>"[^b-ln-oq-z]+"</p>"  ;
<SIMPLE>"<p>&nbsp;</p>"  ;
<SIMPLE>"<p>"[^a-zA-Z0-9&] ;
<SIMPLE>"<p>This material may not be published" ;
<SIMPLE>"<p><strong>"   {BEGIN(SIMPLESTORY);}
<SIMPLE>"<"p.class=\"speakable\"">"    {BEGIN(SIMPLESTORY);/*This is for first paragraphs of FOX NEWS*/}
<SIMPLE>"<"p.class=\"story-body[^"]+\"">" {BEGIN(SIMPLESTORY); /*First line of BBC NEWS */}
<SIMPLE>"<p>"   {BEGIN(SIMPLESTORY);}
<SIMPLESTORY>[^<]+  {writeText();}
<SIMPLESTORY>"<"    {writeText();}
<SIMPLESTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(SIMPLE);}

<PERU>"<div id=\"nota_body\""   {BEGIN(PEMPLE);}
<PEMPLE>"<p>"   {BEGIN(PERUSTORY);}
<PERUSTORY>[^<]+             {writeText();}
<PERUSTORY>"<"               {writeText();}
<PERUSTORY>"</p>" {writeString("&#10;"); BEGIN(PEMPLE);}

<REPUBLICA>"<p class=\"\">"   {BEGIN(REPUBLICASTORY);}
<REPUBLICASTORY>[^<]+             {writeText();}
<REPUBLICASTORY>"<"               {writeText();}
<REPUBLICASTORY>"</p>" {writeString("&#10;&#10;"); BEGIN(REPUBLICA);}

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

