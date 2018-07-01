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
	{"UPI", UPI}, {"WSJ", WSJ}, {"COMERCIO", COMERCIO}, {"BLOOMBERG", BLOOM}} ;

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
};

//Function Prototypes
static int countTitleWords(char *str);
static void getTitle(char *line);
static void cleanItem(char *buffer);
static void getContent(int dest_size, char *dest, char *starttag, char *endtag, char *text);
static int compare_pubDates(const void* a, const void* b);
static void cleanRssDateString(char *rssDateString);
static int getUrls(struct newsAgency *news_agency);
static void cleanDateString(char *rssDateString);
static void escapeQuotes(char *title);
static void cleanUrl(char *url);
static int calcDays(int month, int year);
static void getLatestItems();
static void getFeedItems(char *agency, int type, FILE *newsUrl);
static size_t parseUrlWithFlex(char *url, char **encoded, int flexStartState, char *imgUrl);
static char *base64_encode(char *data, size_t input_length,  size_t *output_length);
static void setOwnEncodedHtml(struct item *item, struct extra *extra);
static int fsize(const char *filename);
static void printTime(char *msg);

FILE *inputptr;
FILE *logptr;
int pfd[2]; // pipe file descriptor array
char *yybuf;
char *linda;
int agencyState;
int main(int argc, char *argv[]){
	linda = malloc(BUF_LEN*6);
	if (linda == NULL) {
		fprintf(logptr, "Could not allocate mem for parsing\n");
		exit(EXIT_FAILURE);
    	}
	logptr = fopen("bloom.log", "a");
	if(argc<2){
		fprintf(logptr, "usage: %s inputFile \n", argv[0]);
		exit(EXIT_FAILURE);
	}
	inputptr = fopen(argv[1], "r");
	if(inputptr == NULL){
		fprintf(logptr, "Could not open file %s for reading \n", argv[1]);
		exit(EXIT_FAILURE);
	}
	setbuf(stderr, NULL);
	setbuf(stdout, NULL);
	memset(&news_agency, 0, sizeof(struct newsAgency));
        getUrls(&news_agency);
	fclose(inputptr);

	getLatestItems();

	return 0;
}

static void getLatestItems(){
	int currentItemsCount = 0;

	if(pipe(pfd) == -1){
      		fprintf(logptr, "Error on opening pipe\n");
      		exit(EXIT_FAILURE);
  	}

	pid_t pid = fork();
	if(pid == -1){
                fprintf(logptr, "Error on forking a child for feed download:%s\n", strerror(errno));
                exit(EXIT_FAILURE);
	}
        if(pid == 0){ //Child code.
       	        close(pfd[0]);  // Child closes it's read end.
	        struct newsAgency *pp = &news_agency;
		do{
	        	if(strlen(pp->name)==0) continue;
		        char beth[1024];
		        strcpy(beth, "wget --timeout=180 -q -O - ");
		        strcat(beth, pp ->url);
		        char snum[5];
		        FILE *newsUrl = popen(beth, "r");

  		      	if (newsUrl == NULL) {
                		fprintf(logptr, "Error could not open news URL for parseUrl: %s\n", strerror(errno));
	  		      	continue;
		        //        exit(EXIT_FAILURE);
        		}
        	        getFeedItems(pp ->name, pp ->type, newsUrl);
			pclose(newsUrl);
		}while((pp = pp ->next) != NULL);
               	close(pfd[1]); // Child is done and closes write end. Parent will receive EOF.
                _exit(0);
       	}

	close(pfd[1]); // parent closes write end of pipe

 	while(read(pfd[0], itemArray + currentItemsCount, ITEM_SIZE) != 0){ //fills itemArray till it receives EOF
        	if(currentItemsCount < NUM_ITEMS-1) currentItemsCount++;
	}

	close(pfd[0]); // parent closes read end of pipe

	int i;

	for(i=0;i<currentItemsCount;i++){
		cleanRssDateString(itemArray[i].pubDate); // attempt to normalize all dates to MST time
	}

//	fprintf(logptr, "Item count:%d\n",currentItemsCount);
	qsort(itemArray, currentItemsCount, sizeof(struct item), compare_pubDates); //sort by pubDate descending

	char buff[BUF_LEN*8];
	int j,k;
	fprintf(stdout,"[");
	for(j=1;j<=NUM_CATEGORIES;j++){
		for(i=0,k=0;i<currentItemsCount;i++){
	     		if(j==itemArray[i].type) {
	     			k++;
		        	char *p;
		        	//sanity check
		        	if(itemArray[i].url == NULL) {
		        		fprintf(logptr, "NULL url:%d\n", j);
		        		continue;
		        	}
		        	if(itemArray[i].pubDate == NULL) {
		        		fprintf(logptr, "NULL pubdate:%d\n", j);
		        		continue;
		        	}
		        	if((p=strstr(itemArray[i].url, "http:")) != &(itemArray[i].url[0]) && p){
			        	long diff = p - itemArray[i].url;
			        	memmove(itemArray[i].url, p, URL_TITLE_LEN-diff );
	        	        }
	        	        buff[0] = '\0';
				struct extra extra;
				memset(&extra, 0, sizeof(struct extra));

				char rssdate[50];
				strncpy(rssdate, itemArray[i].pubDate, 50);
				rssdate[49] = '\0';
				cleanDateString(rssdate);
				strncpy(itemArray[i].pubDate, rssdate, 50);

				setOwnEncodedHtml(&itemArray[i], &extra); // uses flex to scrape html from url
				fprintf(stdout,"{\"html\":\"%s\",",extra.html);
				fprintf(stdout,	"\"pubdate\":\"%s\",",itemArray[i].pubDate);
				fprintf(stdout,	"\"url\":\"%s\",",itemArray[i].url);
				fprintf(stdout,	"\"logo\":\"%s\",","BLOOMBERG.png");
				fprintf(stdout,	"\"title\":\"%s\",",itemArray[i].title);
				if(extra.imgurl[0]){
					fprintf(stdout, "\"img\":\"");
					fprintf(stdout, extra.imgurl);
					fprintf(stdout, "\"}");
				}
				else
					fprintf(stdout, "\"img\":\"http://nllosa.com/images/BLOOMBERG.png\"}");
				if(i<currentItemsCount-1) fprintf(stdout,",");
			}
		}
	}
	fprintf(stdout,"]");
}


void getFeedItems(char *agency, int type, FILE *newsUrl)
{
        int c = 0, k = 0, check = 0;
        int state=OUT,pos=0;
        char a = 0;
        char buffer[BUFFER_SIZE];
        char identifier[12];
        memset(identifier, 0, 12);

        do { //this do loop will capture all the texts in between item tags
		c = getc(newsUrl);
                a = c;
                if(!a || c == EOF) {
			if(!check) //never changed state -- no items
                              fprintf(logptr, "NO RESULTS FOR %s type:%d\n\n", agency,type);
                	break;
                }
                for(k=1;k<11;k++){ //shift left
                        identifier[k-1] = identifier[k];
                }
                identifier[k-1]=a; // add new char at the right end
                if(state==OUT){
                        if(strstr(identifier, "<item>") != NULL){
                                state=IN;
                                check=1;
                                pos=0;
                        }
                }
                else if(state ==IN){
                        if(pos<BUFFER_SIZE-1){
                                buffer[pos++] = a;
                        }
                        if(strstr(identifier, "</item>") != NULL){
                                state=OUT;
                                buffer[pos] = '\0';
                                cleanItem(buffer);
                                if(countTitleWords(buffer)){
                                        struct item item;
					memset(&item, 0, sizeof(struct item));
                                        getContent(50, item.pubDate, "<pubDate>", "</pubDate>", buffer);
                                        getContent(URL_TITLE_LEN, item.title, "<title>", "</title>", buffer);
                                        getContent(URL_TITLE_LEN, item.url, "<link>", "</link>", buffer);
                                        cleanUrl(item.url);
                                        getTitle(item.title);
                                        strcpy(item.agency, agency);
					item.type = type;
                                        write(pfd[1], &item, ITEM_SIZE);
                                }
                        }
                }
        } while (1);
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

void cleanItem(char *buffer){
	char temp[1024];
	memset(temp, 0, 1024);

	char *p = strstr(buffer, "<title>");
	char *p1 = strstr(buffer, "</title>");

	if(!p || !p1) return;
	strncpy(temp, p, p1-p + strlen("</title>"));

	p = strstr(buffer, "<link>");
	p1 = strstr(buffer, "</link>");

	if(!p || !p1) return;
	strncat(temp, p, p1-p + strlen("</link>"));

	p = strstr(buffer, "<pubDate>");
	p1 = strstr(buffer, "</pubDate>");

	if(p && p1){
		strncat(temp, p, p1-p + strlen("</pubDate>"));
	}

	strcpy(buffer, temp);
}

void getTitle(char *line){
	static char *cdata = "<![CDATA[";
	static char *watch = "WATCH:";
	static char buff[BUFFER_SIZE];

	int i =0;
	while(line[i] && i<BUFFER_SIZE-1){
		buff[i] = line[i];
		i++;
	}
	buff[i] = '\0';//terminate string

	char *p1 = &buff[0];

	if(strstr(p1, cdata) == p1){ // starts with cdata
		p1 += strlen(cdata);
	}
	if(strstr(p1, watch) == p1){ // starts with watch
		p1 += strlen(watch);
	}
	int k=0;
	while(isspace(p1[k])) k++; //get rid of leading whitespace
	p1 += k;

	if(strstr(p1, "]]>") == p1+strlen(p1)-3){ // ends with ]]>
		p1[strlen(p1)-3] = '\0';
	}

	if(strstr(p1, "[Video]") == p1+strlen(p1)-7){ // ends with Video
		p1[strlen(p1)-7] = '\0';
	}

	if(strstr(p1, "[VIDEO]") == p1+strlen(p1)-7){ // ends with VIDEO
		p1[strlen(p1)-7] = '\0';
	}


//	escapeQuotes(p1);
	strcpy(line, p1);

}

void cleanUrl(char *url){
	static char *cdata = "<![CDATA[";
	//make sure it starts with http
	if(strstr(url, "http") && strstr(url, "http") != &url[0]){
	  int pos = strstr(url, "http") - &url[0];
	  int length = strlen(url) - pos;
	  if(strstr(url, cdata)){ // take care of "]]>"
	  	length -= 3;
	  }
	  memmove(url, url + pos, length);
	  url[length] = '\0';
	  return;
	}
	//remove CDATA if there
	if(strstr(url, cdata) != &url[0]) return;
	int cdataSize = strlen(cdata);
	int last = strlen(url) - cdataSize - 3;
	memmove(url, &url[cdataSize], strlen(url) - cdataSize);
	url[last] = '\0';
}

// returns true if number of words in title > 4
int countTitleWords(char *str)
{
	int state = OUT;
	int wc = 0;  // word count
	char *p = strstr(str, "</title>");
	if(!p) return 0;
	// Scan all characters one by one
	while (str != p)
	{
		// If next character is a separator, set the
		// state as OUT
		if (*str == ' ' || *str == '\n' || *str == '\t')
			state = OUT;

		else if (state == OUT)
		{
			state = IN;
			++wc;
		}

		++str;
	}
	return wc > 4 ? 1 : 0;
}

void getContent(int dest_size, char *dest, char *starttag, char *endtag, char *text){
	if(!text){
		dest[0] = '\0';
		return;
	}
	char *p1 = strstr(text, starttag);
	char *p2 = strstr(text, endtag);

	if(!p1 || !p2 || p1>p2){
		dest[0] = '\0';
		return;
	}

	size_t size = p2 - p1 - strlen(starttag);
	strncpy(dest, p1+strlen(starttag), dest_size);
	dest[size] = '\0';
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

void setOwnEncodedHtml(struct item *item, struct extra *extra){
	int i, parseType = SIMPLE;
	char *encoded = NULL;

	for(i=0;i<AGENCY_PARSE_SIZE;i++){
		struct agencyParse agencyParse = agencyParseArray[i];
		if(!strcmp(item->agency, agencyParse.agency)){
			parseType = agencyParse.parseType;
			break;
		}
	}

	size_t out_len = parseUrlWithFlex(item->url, &encoded, parseType, extra->imgurl);

	if(out_len>20 && out_len<BUF_LEN*4){
		strncpy(extra->html, encoded, out_len );
		extra->html[out_len] = '\0'; //terminate
	}else{
                if(!strstr(item->url, "video")){
	        	fprintf(logptr, "Error:Type:%d:%s\n", item->type, item->url);
                }
	}
	free(encoded);

}

void cleanDateString(char *rssDateString){
        struct tm tmA;
        memset(&tmA, 0, sizeof(struct tm));
        strptime(rssDateString,"%a, %d %b %Y %H:%M:%S %Z", &tmA);
        strftime(rssDateString, 50, "%Y-%m-%d %H:%M:%S", &tmA);
}

void escapeQuotes(char *title){//add another quote to a quote: ''
        char *p;
        int i;
        if((p = strstr(title, "'"))){
                size_t sz = strlen(title);
                size_t rr = p - &title[0];
                for(i=0;i<sz-rr;i++){
                        title[sz+1-i]=title[sz-i];
                }
                *(p+1) = '\'';
                title[sz+1] = '\0';
                if(strlen(p+2))
                        escapeQuotes(p+2);
        }
}

size_t parseUrlWithFlex(char *url, char **encoded, int flexStartState, char *imgUrl){
        char beth[1024];
	strcpy(beth, "wget --timeout=180 -q -O - ");
        strcat(beth, url);

        char * line = NULL;
        size_t len = 0;
        ssize_t read;
	int countFilled = 0;
	int found = 0;

	FILE *ppp = popen(beth, "r");
        if (ppp == NULL) {
                fprintf(logptr, "Error could not open news URL for parseUrl: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
	}

        while ((read = getline(&line, &len, ppp)) != -1) {
		if((countFilled + read) >= BUF_LEN*6 ){
			fprintf(logptr,"Buffer Size Exceeded!!!!:%d\n", countFilled + read);
			fflush(logptr);
			return 0;
		}
		if(!found){
			if(strstr(line,"property=\"og:image\"")){
		        	char *p = strstr(line, "http");
	        		if(p){
	        			char *m = strstr(p, "\"");
        				if(m){
        					strncpy(imgUrl, p, m-p);
        					found = 1;
        				}
				}
			}
		}
		memcpy(linda + countFilled, line, read);
		countFilled = countFilled + read;
	}
	free(line);
//	fprintf(logptr,"(%.*s)\n", countFilled-1, linda);
	fprintf(logptr,"--%d\n", countFilled);
	fflush(logptr);

	pclose(ppp);

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
