#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "flex.h"

char *yybuf;

int main (int argc, char **argv) {
	if(argc<2) {
		printf("Usage: %s url\n", argv[0]);
		return 1;
	}
	char beth[256];
	strcpy(beth, "wget -S -O - ");
	strcat(beth, argv[1]);
//	char *beth = "wget -S -O - 'http://abcnews.go.com/Politics/house-speaker-ryan-hopes-white-house-tempering-agent/story?id=44877257'";
        FILE *pp = popen(beth, "r");

        if (pp != NULL) {
		yyin = pp;
		char tumia[YY_BUF_SIZE];
        	tumia[0]= '\0';
	        tumia[YY_BUF_SIZE-1]= '\0';
		yybuf = &tumia[0]; //flex will write to yybuf
		YY_BUFFER_STATE bp = yy_create_buffer(yyin, YY_BUF_SIZE);
		yy_switch_to_buffer(bp);
               	yylex();
		fprintf(stdout, "%s\n", tumia);fflush(stdout);
		pclose(pp);
        }
	else{
		fprintf(stdout, "Error could not open pipe: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

