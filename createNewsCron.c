/*
* C program that gets the latest rss feeds from various news agencies and categories and saves them to a database
* Uses libcurl open source library to download feed.xml
* The program accounts that the rss feeds downloaded are not neccesarily "proper"
* Compile with : gcc createNewsCron.c -o createNewsCron -lcurl `mysql_config --cflags --libs`
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <my_global.h>
#include <mysql.h>
#include <errno.h>

#define OUT 0
#define IN  1
#define MAX_TITLES 100
#define BUFFER_SIZE 8192
#define BUF_LEN 32768
#define NUM_ITEMS 3000
#define NUM_TITLES 20

struct item{
	int type;
	char title[512];
	char agency[10];
	char pubDate[50];
	char url[256];
} itemArray[NUM_ITEMS];

#define ITEM_SIZE sizeof(struct item)

struct newsAgency {
        int type;
        char name[10];
        char url[100];
	struct newsAgency *next;
} news_agency;

struct MemoryStruct {
  char *memory;
  size_t size;
};

struct extra {
  char imgurl[512];
  char html[BUF_LEN];
};

//Function Prototypes
static int countTitleWords(char *str);
static void getTitle(char *line);
static void cleanItem(char *buffer);
static void getContent(int dest_size, char *dest, char *starttag, char *endtag, char *text);
static int compare_pubDates(const void* a, const void* b);
static void cleanRssDateString(char *rssDateString);
static int getUrls(struct newsAgency *news_agency);
static void getInsertString(struct item *item, char *json, int type);
static void cleanDateString(char *rssDateString);
static void escapeQuotes(char *title);
static void cleanUrl(char *url);
static int calcDays(int month, int year);
static void getLatestItems();
static void fillStruct(char *url, struct extra *beth);
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
static void loadFeed(char *url);
static void getFeedItems(char *agency, int type, char *buff);
static void initCurl();
static void initMysql();
static void cleanCurl();
static void deleteExtraRecords(int type);

MYSQL *con ;
FILE *inputptr;
FILE *logptr;
int pfd[2]; // pipe file descriptor array
struct MemoryStruct chunk;
CURL *curl_handle;

int main(int argc, char *argv[]){
	if(argc<3){
		fprintf(stdout, "usage: %s inputFile logFile\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	inputptr = fopen(argv[1], "r");
	if(inputptr == NULL){
		fprintf(stdout, "Could not open file %s for reading \n", argv[1]);
		exit(EXIT_FAILURE);
	}
	logptr = fopen(argv[2], "a");
	if (logptr == NULL){
		fprintf(stdout, "Could not open file %s for writing logs\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	setbuf(stdout, NULL);
	initMysql();
	initCurl();
	time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char s[64];
        strftime(s, sizeof(s), "%c", tm);
        fprintf(logptr, "start: %s\n", s);

	memset(&news_agency, 0, sizeof(struct newsAgency));
        getUrls(&news_agency);
	fclose(inputptr);

	getLatestItems();

    	t = time(NULL);
    	tm = localtime(&t);
	strftime(s, sizeof(s), "%c", tm);
    	fprintf(logptr, "end: %s\n\n", s);
	fclose(logptr);
	mysql_close(con);
	cleanCurl();
	return 0;
}

static void getLatestItems(){
	int currentItemsCount = 0;

	if(pipe(pfd) == -1){
      		fprintf(stdout, "Error on opening pipe\n");
      		exit(EXIT_FAILURE);
  	}

	pid_t pid = fork();
	if(pid == -1){
                fprintf(logptr, "Error on forking a child for feed download:%s\n", strerror(errno));
                fflush(logptr);
                exit(EXIT_FAILURE);
	}
        if(pid == 0){ //Child code.
       	        close(pfd[0]);  // Child closes it's read end.
	        struct newsAgency *pp = &news_agency;
		do{
	        	if(strlen(pp->name)==0) continue;
                	chunk.memory = malloc(1);
        	        chunk.size = 0;
       	        	loadFeed(pp ->url); // Loads feed into memory.
        	        getFeedItems(pp ->name, pp ->type, chunk.memory);
       	        	free(chunk.memory);
		}while((pp = pp ->next) != NULL);
               	close(pfd[1]); // Child is done and closes write end. Parent will receive EOF.
                _exit(0);
       	}

	close(pfd[1]); // parent closes write end of pipe

 	while(read(pfd[0], itemArray + currentItemsCount, ITEM_SIZE) != 0){ //fills itemArray till it receives EOF
        	if(currentItemsCount < NUM_ITEMS-1) currentItemsCount++;
	        //fprintf(stdout, "url:%s,agency:%s,pubDate:%s,title:%s\n",item.url,item.agency,item.pubDate,item.title);
	}

	close(pfd[0]); // parent closes read end of pipe

	int i;
	for(i=0;i<currentItemsCount;i++){
		cleanRssDateString(itemArray[i].pubDate); // attempt to normalize all dates to MST time
	}

	fprintf(logptr, "Item count:%d\n",currentItemsCount);
	qsort(itemArray, currentItemsCount, sizeof(struct item), compare_pubDates); //sort by pubDate descending

	char buff[BUF_LEN];
	int j,k;
	for(j=1;j<=7;j++){
		for(i=0,k=0;i<currentItemsCount;i++){
			if(k==NUM_TITLES) break;
	     		if(j==itemArray[i].type) {
	     			k++;
				getInsertString(&itemArray[i], buff, itemArray[i].type);
				if (mysql_query(con, buff)){
					fprintf(logptr, "ERROR:%s\n", mysql_error(con));
				}
			}
		}
		deleteExtraRecords(j);
	}
}

//Leave at least 80 records for each category
void deleteExtraRecords(int type){
	char buff[512];
	strcpy(buff, "DELETE FROM news WHERE pubdate < (select pubdate from (select * from news)a where news_type=");
        char beth[5];
       	sprintf(beth, "%d", type);
	strcat(buff, beth);
	strcat(buff, " order by pubdate desc limit 79,1) and news_type=");
	strcat(buff, beth);
	//fprintf(logptr, "deletestring:%s\n",buff);
	if (mysql_query(con, buff)){
		fprintf(logptr, "ERROR:%s\n", mysql_error(con));
	}
}

void getFeedItems(char *agency, int type, char *buff)
{
        int i=0,k = 0;
        int state=OUT,pos=0;
        char a = 0;
        char buffer[BUFFER_SIZE];
        char identifier[12];
        memset(identifier, 0, 12);
        do { //this do loop will capture all the texts in between item tags
                a = buff[i];
                if(!a) break;
                for(k=1;k<11;k++){ //shift left
                        identifier[k-1] = identifier[k];
                }
                identifier[k-1]=a; // add new char at the right end
                if(state==OUT){
                        if(strstr(identifier, "<item>") != NULL){
                                state=IN;
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
                                        getContent(512, item.title, "<title>", "</title>", buffer);
                                        getContent(256, item.url, "<link>", "</link>", buffer);
                                        cleanUrl(item.url);
                                        getTitle(item.title);
                                        strcpy(item.agency, agency);
					item.type = type;
                                        write(pfd[1], &item, ITEM_SIZE);
	                                //fprintf(stdout, "url:%s,agency:%s,pubDate:%s,title:%s\n",item.url,item.agency,item.pubDate,item.title);
                                }
                        }
                }
          i++;
        } while (1);
}

int getUrls(struct newsAgency *news_agency){
        char * line = NULL;
        size_t len = 0;
        ssize_t read;

        struct newsAgency *pp = news_agency;
       	read = getline(&line, &len, inputptr);
	if(read == -1) return 1;
        sscanf( line, "%d,%10[^,],%s", &pp->type, pp->name, pp->url );

        while ((read = getline(&line, &len, inputptr)) != -1) {
                pp->next = calloc(1, sizeof(struct newsAgency));
                pp = pp->next;
                sscanf( line, "%d,%10[^,],%s", &pp->type, pp->name, pp->url );
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
//	static char *suffix = "</TITLE>";
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
	escapeQuotes(p1);
	strcpy(line, p1);
}

void cleanUrl(char *url){
	static char *cdata = "<![CDATA[";

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

void getInsertString(struct item *item, char *json, int type){
	char beth[5];
	char rssdate[50];
	strcpy(rssdate, item ->pubDate);
	cleanDateString(rssdate);
	struct extra extra;
	memset(&extra, 0, sizeof(struct extra));
//	fprintf(logptr, "url:%s\n", item ->url); fflush(logptr);

	fillStruct(item ->url, &extra); // uses newspaper python module to scrape html and top image from url

//	fprintf(logptr, "html:%s, img:%s\n", extra.html, extra.imgurl); fflush(logptr);

	sprintf(beth, "%d", type);

	if(strstr(extra.imgurl, "You must") != NULL) extra.imgurl[0]='\0';
	strcpy(json, "REPLACE INTO news (url,html,img,news_type,title,pubdate,agency) values ('");
	strcat(json, item ->url);
	strcat(json, "','");
	strcat(json, extra.html);
	strcat(json, "','");
        strcat(json, extra.imgurl);
	strcat(json, "',");
	strcat(json, beth);
	strcat(json, ",'");
	strcat(json, item ->title);
	strcat(json, "',STR_TO_DATE('");
	strcat(json, rssdate);
	strcat(json, "','%Y-%m-%d %H:%i:%S'),'");
	strcat(json, item ->agency);
	strcat(json, "')");
//	fprintf(logptr, "insertString:%s\n\n", json); fflush(logptr);
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

void fillStruct(char *url, struct extra *beth){
        char tumia[512];
	char *cc;
        strcpy(tumia, "./neri.py '");
        strcat(tumia, url);
        strcat(tumia, "'");
        char gigi[BUF_LEN];
	memset(gigi, 0, BUF_LEN);
        FILE *pp = popen(tumia, "r");

        if (pp != NULL) {
  		if(fgets(tumia, sizeof(tumia), pp) != NULL) {
			strcpy(beth->imgurl, tumia);
			if((cc = strstr(beth->imgurl, "\n")))
				*cc = '\0';
  		}

  		if(fgets(gigi, sizeof(gigi), pp) != NULL) {
			strcpy(beth->html, gigi);
		}
  		if(fgets(gigi, sizeof(gigi), pp) != NULL) {//twice max
			strcat(beth->html, gigi);
		}

		size_t sz = strlen(beth->html);
		if(beth->html[sz-1] == '\n'){
			beth->html[sz-1] = '\0';
			//fprintf(stdout, "html_len:%zu, url:%s\n", sz, url);fflush(stdout);
		}

                pclose(pp);
        }
	else{
		fprintf(logptr, "Error could not open pipe: %s\n", strerror(errno));
		fflush(logptr);
		exit(EXIT_FAILURE);
	}
}

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

void initCurl(){
  curl_global_init(CURL_GLOBAL_ALL);
  /* init the curl session */
  curl_handle = curl_easy_init();
  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
}

void cleanCurl(){
  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);
  /* we're done with libcurl, so clean it up */
  curl_global_cleanup();
}

void loadFeed(char *url)
{
//  fprintf(stdout, "!!%s\n", url);
  CURLcode res;
  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  /* get it! */
  res = curl_easy_perform(curl_handle);
  /* check for errors */
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
}

void initMysql(){
        con = mysql_init(NULL);
        if (con == NULL){
                fprintf(logptr, "ERROR:mysql_init() failed\n");
                exit(EXIT_FAILURE);
        }
        if (mysql_real_connect(con, "localhost", "nerillos_neri", "carpa1", "nerillos_neri", 0, NULL, 0) == NULL){
                fprintf(logptr, "ERROR:%s\n", mysql_error(con));
                mysql_close(con);
                exit(EXIT_FAILURE);
        }
}
