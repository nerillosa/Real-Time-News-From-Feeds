/*
* C program that gets the latest news articles from Politico
* Uses flex lexical analyzer to parse html from the harvested urls.
* Program receives all news urls as arguments when invoked by script or directly.
* The program files flex.h and lex.yy.c are created with the following command: flex --header-file=flex.h multiple.lex
* where multiple.lex contains the rules for extracting the html data.
* Compile with : gcc politicoNews.c lex.yy.c -o createPolitico
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define BUFFER_SIZE 4096
#define BUF_LEN 32768
#define URL_TITLE_LEN 512

#define YY_HEADER_EXPORT_START_CONDITIONS
#include "flex.h"

struct extra {
  char imgurl[URL_TITLE_LEN];
  char html[BUF_LEN * 4];
  char pubDate[50];
  char title[URL_TITLE_LEN];
};

//Function Prototypes
static size_t parseUrlWithFlex(char *url, char **encoded, int flexStartState, struct extra *extra);
static char *base64_encode(char *data, size_t input_length,  size_t *output_length);
static void printTime(char *msg);
static void AddDays(char * msg, size_t maxsize, int daysToAdd);

FILE *inputptr;
FILE *logptr;
char *yybuf;
int agencyState;
int main(int argc, char *argv[]){
        if(argc<2){
                fprintf(stdout, "usage: %s url-args \n", argv[0]);
                exit(EXIT_FAILURE);
        }

	logptr = fopen("politico.log", "w");
	printTime("Start");
	setbuf(stdout, NULL);
        int i = 1;
        fprintf(stdout,"[");
        char today[50];
        char yesterday[50];
        AddDays(today, 49, 0); // 2018/07/08
        AddDays(yesterday, 49, -1); // 2018/07/07

        for(;i<argc;i++){
                if(!strstr(argv[i],today) && !strstr(argv[i],yesterday)){ // only news from today or yesterday
                        continue;
                }
		char *encoded = NULL;
		struct extra extra;
		memset(&extra, 0, sizeof(struct extra));
		char url2[BUFFER_SIZE];
		strcpy(url2, argv[i]);
		size_t out_len = parseUrlWithFlex(url2, &encoded, POLITICO, &extra);

		if(out_len>20 && out_len<BUF_LEN*4){
			strncpy(extra.html, encoded, out_len );
			extra.html[out_len] = '\0'; //terminate

                        fprintf(stdout,"{\"html\":\"%s\",",extra.html);
                        fprintf(stdout, "\"pubdate\":\"%s\",",extra.pubDate);
                        fprintf(stdout, "\"url\":\"%s\",",url2);
                        fprintf(stdout, "\"logo\":\"%s\",","POLITICO.png");
                        fprintf(stdout, "\"title\":\"%s\",",extra.title);
			if(extra.imgurl[0]){
				fprintf(stdout, "\"img\":\"");
				fprintf(stdout, extra.imgurl);
				fprintf(stdout, "\"}");
			}
			else
				fprintf(stdout, "\"img\":\"http://nllosa.com/images/POLITICO.png\"}");
			if(i<argc-1) fprintf(stdout,",");

		}else{
        		fprintf(logptr, "Error Carajo for %s\n", url2);
		}
		free(encoded);
        }
        fprintf(stdout,"]");

	printTime("End");
	fprintf(logptr, "\n");
	return 0;
}

size_t parseUrlWithFlex(char *url, char **encoded, int flexStartState, struct extra *extra){
        char beth[1024];
	strcpy(beth, "wget --timeout=180 -q -O - ");
        strcat(beth, url);

	char linda[BUF_LEN*10];
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
	int countFilled = 0;
	int found = 0;
	int found2 = 0;
	int foundd=0;
	int start = 0;
	char *p;

	FILE *ppp = popen(beth, "r");
	if (ppp == NULL) {
		fprintf(logptr, "Error could not open news URL for parseUrl: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	while ((read = getline(&line, &len, ppp)) != -1) {
		if(!found){
			if(strstr(line,"property=\"og:image\"")){
		        	p = strstr(line, "http");
	        		if(p){
	        			char *m = strstr(p, "\"");
        				if(m){
        					strncpy(extra-> imgurl, p, m-p);
        					found = 1;
        				}
				}
			}
		}
		if(!foundd){
			if(p=strstr(line,"datetime=")){
				strncpy(extra-> pubDate, p+10, 19); //2018-01-01 13:00:00
       				foundd = 1;
			}
		}
		if(!found2){
			if(strstr(line,"property=\"og:title\"")){
		        	p = strstr(line, "content=\"");
	        		if(p){
	        			char *m = strstr(p+10, "\"");
        				if(m){
        					strncpy(extra-> title, p+9, m-p-9);
        					found2 = 1;
        				}
				}
			}
		}

		if(!start && found && found2 && foundd){ //start flex scanning from here
			start = 1;
		}
		else if(!start){
			continue;
		}
		if((countFilled + read) >= BUF_LEN*10 ){
			fprintf(logptr,"Buffer Size Exceeded!!!!:%d for %s\n", countFilled + read, url);
			fflush(logptr);
			return 0;
		}

		memcpy(linda + countFilled, line, read);
		countFilled = countFilled + read;
	}
	free(line);
	fprintf(logptr,"%d, %s\n", countFilled, url);
	fflush(logptr);
	pclose(ppp);
	linda[countFilled-read] = '\0';
	FILE *pp = fmemopen(linda, countFilled-read, "r");

        yyin = pp;
        char tumia[BUF_LEN];
	memset(tumia, 0x00, BUF_LEN);

        agencyState = flexStartState; //global variable that flex uses to define start state
        yybuf = &tumia[0]; //flex will write to yybuf

        YY_BUFFER_STATE bp = yy_create_buffer(yyin, BUF_LEN);
        yy_switch_to_buffer(bp);

        yylex();
        tumia[BUF_LEN-1]= '\0';
	size_t out_len = 0;
        *encoded = base64_encode(tumia, strlen(tumia), &out_len);

        yy_delete_buffer(bp);
        fclose(pp);
/*
	//https://www.politico.com/story/2018/06/27/black-womens-congressional-alliance-women-rule-678719
	char *p = strstr(url, "/20");
	if(p){
		strncpy(beth, p+1, 10);// 2018/06/27
		beth[10] = '\0';
                p = &beth[0];
                while(*p++){
                        if(*p=='/') *p='-';
                }
		strcat(beth, " 13:00:00");
	}else{
		strcpy(beth,"2018-01-01 13:00:00");
	}
	strcpy(extra-> pubDate, beth);
*/
	return out_len;
}

// Encodes string to base64. Returned pointer must be freed after use.
char *base64_encode(char *data, size_t input_length,  size_t *output_length) {
	static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

	static int mod_table[] = {0, 2, 1};

	*output_length = 4 * ((input_length + 2) / 3);

	char *encoded_data = malloc(*output_length);
	if (encoded_data == NULL) return NULL;
  	int i,j;
  	for (i = 0, j = 0; i < input_length;) {

    		uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
  	  	uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
    		uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

	    	uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    		encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
	    	encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
    		encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
	    	encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
  	}

  	for (i = 0; i < mod_table[input_length % 3]; i++)
    		encoded_data[*output_length - 1 - i] = '=';

  	return encoded_data;
}

void printTime(char *msg){
	time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char s[URL_TITLE_LEN];
        strcpy(s, msg);
        strcat(s, ": ");
        strftime(s+strlen(msg)+2, URL_TITLE_LEN-strlen(msg)-2, "%c", tm);
        fprintf(logptr, "%s\n", s);
}

void AddDays(char * msg, size_t maxsize, int daysToAdd)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    time_t oneDay = 24 * 60 * 60;
     // Seconds since start of epoch --> 01/01/1970 at 00:00hrs
    time_t date_seconds = mktime(tm) + (daysToAdd * oneDay);

    *tm = *localtime(&date_seconds);

    strftime(msg, maxsize, "%Y/%m/%d", tm);
}

