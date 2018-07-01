/*
* C program that gets the latest rss feeds from bloomberg rss
* Uses libcurl open source library to download feed.xml
* Uses flex lexical analyzer to parse html and top image from the harvested urls.
* The program files flex.h and lex.yy.c are created with the following command: flex --header-file=flex.h multiple.lex
* where multiple.lex contains the rules for extracting the html data.
* Compile with : gcc bloombergNews.c lex.yy.c -o createBloomberg
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

#define OUT 0
#define IN  1
#define MAX_TITLES 100
#define BUFFER_SIZE 8192
#define BUF_LEN 32768
#define NUM_ITEMS 3000
#define NUM_CATEGORIES 13
#define URL_TITLE_LEN 512

#define YY_HEADER_EXPORT_START_CONDITIONS
#include "flex.h"

struct item{
	int type;
	char title[URL_TITLE_LEN];
	char agency[10];
	char pubDate[50];
	char url[URL_TITLE_LEN];
} itemArray[NUM_ITEMS];

#define ITEM_SIZE sizeof(struct item)

struct agencyParse{
	char* agency;
	int parseType;
} agencyParseArray[] = {{"CNN", CNN}, {"FOX NEWS", FOX}, {"NY TIMES", NYT},{"ABC NEWS", ABC},{"GESTION", GESTION},
	{"PERU21", PERU21}, {"CNBC", CNBC}, {"REUTERS", REUTERS}, {"US TODAY", USTODAY}, {"WSH POST", WSH},
	{"UPI", UPI}, {"WSJ", WSJ}, {"COMERCIO", COMERCIO}, {"BLOOMBERG", BLOOM}, {"HUFFINGTON", HUFF}} ;

static int AGENCY_PARSE_SIZE = sizeof(agencyParseArray)/sizeof(agencyParseArray[0]);

struct newsAgency {
        int type;
        char name[10];
        char url[URL_TITLE_LEN];
	struct newsAgency *next;
} news_agency;

struct extra {
  char imgurl[URL_TITLE_LEN];
  char html[BUF_LEN * 4];
  char pubDate[50];
  char title[URL_TITLE_LEN];
};

//Function Prototypes
static int compare_pubDates(const void* a, const void* b);
static void cleanRssDateString(char *rssDateString);
static int getUrls(struct newsAgency *news_agency);
static void cleanDateString(char *rssDateString);
static int calcDays(int month, int year);
static size_t parseUrlWithFlex(char *url, char **encoded, int flexStartState, struct extra *extra);
static char *base64_encode(char *data, size_t input_length,  size_t *output_length);
static int fsize(const char *filename);
static void printTime(char *msg);

FILE *inputptr;
FILE *logptr;
int pfd[2]; // pipe file descriptor array
char *yybuf;
int agencyState;
int main(int argc, char *argv[]){
        if(argc<2){
                fprintf(logptr, "usage: %s url-args \n", argv[0]);
                exit(EXIT_FAILURE);
        }

	logptr = fopen("huff.log", "a");
	printTime("Start");
	setbuf(stdout, NULL);
        int i = 1;
        fprintf(stdout,"[");

        for(;i<argc;i++){
		char *encoded = NULL;
		struct extra extra;
		memset(&extra, 0, sizeof(struct extra));
		char url2[4096];
		strcpy(url2, "https://www.huffingtonpost.com");
		strcat(url2, argv[i]);
		size_t out_len = parseUrlWithFlex(url2, &encoded, HUFF, &extra);

		if(out_len>20 && out_len<BUF_LEN*4){
			strncpy(extra.html, encoded, out_len );
			extra.html[out_len] = '\0'; //terminate

                        fprintf(stdout,"{\"html\":\"%s\",",extra.html);
                        fprintf(stdout, "\"pubdate\":\"%s\",",extra.pubDate);
                        fprintf(stdout, "\"url\":\"%s\",",url2);
                        fprintf(stdout, "\"logo\":\"%s\",","HUFF.png");
                        fprintf(stdout, "\"title\":\"%s\",",extra.title);
			if(extra.imgurl[0]){
				fprintf(stdout, "\"img\":\"");
				fprintf(stdout, extra.imgurl);
				fprintf(stdout, "\"}");
			}
			else
				fprintf(stdout, "\"img\":\"http://nllosa.com/images/HUFF.png\"}");
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

int getUrls(struct newsAgency *news_agency){
        char * line = NULL;
        size_t len = 0;
        ssize_t read;

        struct newsAgency *pp = news_agency;

        while ((read = getline(&line, &len, inputptr)) != -1) {
		if(line[0] == '#') continue;  //skip comments
                pp->next = calloc(1, sizeof(struct newsAgency));
                sscanf( line, "%d,%10[^,],%s", &pp->type, pp->name, pp->url );
                pp = pp->next;
        }
	free(line);
        return 0;
}

// utility compare function to be used in qsort
int compare_pubDates(const void* a, const void* b) {
	struct item *itemA = (struct item *)a;
	struct item *itemB = (struct item *)b;
	struct tm tmA,tmB;

	memset(&tmA, 0, sizeof(struct tm));
	memset(&tmB, 0, sizeof(struct tm));

	if(!(itemA -> pubDate)[0]) return 1;
	if(!(itemB -> pubDate)[0]) return -1;

	strptime(itemA -> pubDate,"%a, %d %b %Y %H:%M:%S %Z", &tmA);
	strptime(itemB -> pubDate,"%a, %d %b %Y %H:%M:%S %Z", &tmB);

	if(tmA.tm_year > tmB.tm_year) return -1;
	else if (tmA.tm_year < tmB.tm_year) return 1;
	else if (tmA.tm_mon > tmB.tm_mon) return -1;
	else if (tmA.tm_mon < tmB.tm_mon) return 1;
	else if (tmA.tm_mday > tmB.tm_mday) return -1;
	else if (tmA.tm_mday < tmB.tm_mday) return 1;
	else if (tmA.tm_hour > tmB.tm_hour) return -1;
	else if (tmA.tm_hour < tmB.tm_hour) return 1;
	else if (tmA.tm_min > tmB.tm_min) return -1;
	else if (tmA.tm_min < tmB.tm_min) return 1;
	else if (tmA.tm_sec > tmB.tm_sec) return -1;
	else if (tmA.tm_sec < tmB.tm_sec) return 1;
	else return 0;
}

void cleanRssDateString(char *rssDateString){
	if(!rssDateString[0]) return;

	int i=0,k=0;
	while(rssDateString[i]){
		if(rssDateString[i++] == ':') k++;
	}
	if(!k) return;
	if(k==1){ // its missing the seconds count
		int sz = strlen(rssDateString);
		for(i=0;i<5;i++){
			rssDateString[sz + 3 -i] = rssDateString[sz-i];
		}
		rssDateString[sz-4] = ':';
		rssDateString[sz-3] = '0';
		rssDateString[sz-2] = '0';
	}

        char *t;
        if((t = strstr(rssDateString, "EST"))){
           strcat (t, "-0500");
        }

	struct tm tmA;
	memset(&tmA, 0, sizeof(struct tm));
	strptime(rssDateString,"%a, %d %b %Y %H:%M:%S %Z", &tmA);

	char *p = rssDateString;
	p += strlen(p)-5;
	int diff = 0;//7; // MST = UTC -7
	int delta= 0;
	if(*p == '-' || *p == '+'){
		delta = atoi(p)/100;
	}
	diff += delta;
	if(diff){
		tmA.tm_hour -= diff;
                if(tmA.tm_hour > 23){
                        tmA.tm_hour -= 24;
                        tmA.tm_mday += 1;
                }
		if(tmA.tm_hour < 0) {
			tmA.tm_hour += 24;
			if(tmA.tm_mday > 1){
				tmA.tm_mday -= 1;
			}else{
				tmA.tm_mon -= 1;
				if(tmA.tm_mon < 0){ // its January 1st!
					tmA.tm_mon = 11;
					tmA.tm_year -= 1;
				}else
					tmA.tm_mday = calcDays(tmA.tm_mon, 1900 + tmA.tm_year);
			}
		}
		strftime(rssDateString, 50, "%a, %d %b %Y %H:%M:%S GMT", &tmA);
	}
}

int calcDays(int month, int year)// calculates number of days in a given month
{
	int Days;
	if (month == 3 || month == 5 || month == 8 || month == 10) Days = 30;
	else if (month == 1) {
		int isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
		if (isLeapYear) Days = 29;
		else Days = 28;
	}
	else Days = 31;
	return Days;
}

void cleanDateString(char *rssDateString){
        struct tm tmA;
        memset(&tmA, 0, sizeof(struct tm));
        strptime(rssDateString,"%a, %d %b %Y %H:%M:%S %Z", &tmA);
        strftime(rssDateString, 50, "%Y-%m-%d %H:%M:%S", &tmA);
}

size_t parseUrlWithFlex(char *url, char **encoded, int flexStartState, struct extra *extra){
        char beth[1024];
	strcpy(beth, "wget --timeout=180 -q -O - ");
        strcat(beth, url);

	char linda[BUF_LEN*12];
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
	int countFilled = 0;
	int found = 0;
	int foundd = 0;
	int found2 = 0;

	FILE *ppp = popen(beth, "r");
	if (ppp == NULL) {
		fprintf(logptr, "Error could not open news URL for parseUrl: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	while ((read = getline(&line, &len, ppp)) != -1) {
		if((countFilled + read) >= BUF_LEN*12 ){
			fprintf(logptr,"Buffer Size Exceeded!!!!:%d for %s\n", countFilled + read, url);
			fflush(logptr);
			return 0;
		}
		if(!found){
			if(strstr(line,"property=\"og:image\"")){
		        	char *p = strstr(line, "http");
	        		if(p){
	        			char *m = strstr(p, "\"");
        				if(m){
        					strncpy(extra-> imgurl, p, m-p);
        					found = 1;
        				}
				}
			}
		}
		if(!found2){
			if(strstr(line,"property=\"og:title\"")){
		        	char *p = strstr(line, "content=\"");
	        		if(p){
	        			char *m = strstr(p+10, "\"");
        				if(m){
        					strncpy(extra-> title, p+9, m-p-9);
        					found2 = 1;
        				}
				}
			}
		}
		if(!foundd){
			if(strstr(line,"property=\"article:modified_time\"")){
		        	char *p = strstr(line, "content=\"");
	        		if(p){
	        			char *m = strstr(p+10, "\"");
        				if(m){
        					strncpy(extra-> pubDate, p+9, m-p-9-6); //-6 takes away -0400
//        					cleanRssDateString(pubDate);
//        					cleanDateString(pubDate);
        					foundd = 1;
        				}
				}
			}
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

int fsize(const char *filename) {
        struct stat st;

        if (stat(filename, &st) == 0)
                return (st.st_size + 0);

        return -1;
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
