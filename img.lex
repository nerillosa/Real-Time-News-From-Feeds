%{
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
<STRT>=\"og:image\".content=["'] {BEGIN(STORY); /* ABC, NYT, US TODAY, WSH POST */}
<STRT>=\"og:image\".itemprop=\"image\".content=\" {BEGIN(STORY); /* CNBC  */}
<STORY>[^"']+   {ECHO;}
<STORY>["']     {printf("\n");BEGIN(INITIAL);}

%%

