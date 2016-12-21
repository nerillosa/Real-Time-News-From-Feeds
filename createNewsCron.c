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

#define OUT 0
#define IN  1
#define MAX_TITLES 100
#define BUFFER_SIZE 8192
#define BUF_LEN 32768
#define ITEM_SIZE 400
#define NUM_TITLES 20

struct item{
	char title[512];
	char agency[10];
	char pubDate[50];
	char url[256];
} itemArray[ITEM_SIZE];

struct news {
	char agency[10];
	char items[ITEM_SIZE][512];
	int size;
} newsItems;

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
static void download_feed(FILE *dst, const char *src);
static int countTitleWords(char *str);
static void getTitle(char *line);
static int fsize(const char *filename);
static void cleanItem(char *buffer);
static void fillItems(struct item *items);
static void getContent(int dest_size, char *dest, char *starttag, char *endtag, char *text);
static int refreshFeed(struct newsAgency newsAgency);
static int compare_pubDates(const void* a, const void* b);
static void cleanRssDateString(char *rssDateString);
static int getUrls(struct newsAgency *news_agency);
static void getInsertString(struct item *item, char *json, int type);
static void cleanDateString(char *rssDateString);
static void escapeQuotes(char *title);
static void cleanUrl(char *url);
static void cleanTitle(int size, char *buff);
static int calcDays(int month, int year);
static void getLatestItems(int type);
static void downloadFeeds(int type);
static void getparticulars(char *url, struct extra *extra);
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
static void fillStruct(char *url, struct extra *beth);
static void cleanHtml(char *code, char value, char * const line);

FILE *inputptr;
FILE *logptr;

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
	time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char s[64];
        strftime(s, sizeof(s), "%c", tm);
        fprintf(logptr, "start: %s\n", s);


        int size = getUrls(&news_agency);
	fclose(inputptr);

	int i;
	for(i=1;i<=11;i++)
		getLatestItems(i);

    	t = time(NULL);
    	tm = localtime(&t);
	strftime(s, sizeof(s), "%c", tm);
    	fprintf(logptr, "end: %s\n\n", s);

	fclose(logptr);
}

static void getLatestItems(int type){
	int i,j;
	int currentItemsCount = 0;

	downloadFeeds(type);

        struct newsAgency *pp = &news_agency;

	do{
		if(pp -> type != type) continue;
		refreshFeed(*pp);
		fillItems(&itemArray[0] + currentItemsCount);
		currentItemsCount += newsItems.size;
	}while((pp = pp ->next) != NULL);

	for(i=0;i<currentItemsCount;i++){
		cleanRssDateString(itemArray[i].pubDate); // attempt to normalize all dates to MST time
	}

	//fprintf(logptr, "Items:%d, %s -->",currentItemsCount, itemArray[0].pubDate);
	qsort(itemArray, currentItemsCount, sizeof(struct item), compare_pubDates); //sort by pubDate descending

	//fprintf(logptr, "%s\n", type==1 ? "Politics" : type==2 ? "Science" : type==3 ? "World" : type==4 ? "Sports" : type==5 ? 
	//"Entertainment" : type==6 ? "Health" : type==7 ? "USA" : type==8 ? "Actualidad" : type==9 ? "Deporte" : type==10 ? "Economia" : 
	//"Entretenimiento");

	char buff[BUF_LEN];

	MYSQL *con = mysql_init(NULL);

	if (con == NULL){
		fprintf(logptr, "ERROR:mysql_init() failed\n");
		return;
	}
	if (mysql_real_connect(con, "localhost", "nerillos_neri", "carpa1", "nerillos_neri", 0, NULL, 0) == NULL){
		fprintf(logptr, "ERROR:%s\n", mysql_error(con));
		mysql_close(con);
		return;
	}
	for(i=0;i<NUM_TITLES;i++){
		getInsertString(&itemArray[i], buff, type);
		if (mysql_query(con, buff)){
			fprintf(logptr, "ERROR:%s\n", mysql_error(con));
		}
	}

	//Leave at least 80 records for each category
	strcpy(buff, "DELETE FROM news WHERE pubdate < (select pubdate from (select * from news)a where news_type=");
        char beth[5];
        sprintf(beth, "%d", type);
	strcat(buff, beth);
	strcat(buff, " order by pubdate desc limit 79,1) and news_type=");
	strcat(buff, beth);
	if (mysql_query(con, buff)){
		fprintf(logptr, "ERROR:%s\n", mysql_error(con));
	}

	mysql_close(con);
}

int getUrls(struct newsAgency *news_agency){
        char * line = NULL;
        size_t len = 0;
        ssize_t read;

        struct newsAgency *pp = news_agency;

        int ii = 0;
        while ((read = getline(&line, &len, inputptr)) != -1) {
                sscanf( line, "%d,%10[^,],%s", &pp->type, pp->name, pp->url );
                pp->next = malloc(sizeof(struct newsAgency));
                pp = pp->next;
                ii++;
        }
	pp = NULL; // a little memory leak. program will exit quick anyways
        return ii;
}

void fillItems(struct item *items){
	int i;
	for(i=0;i<newsItems.size;i++){
		char *text = newsItems.items[i];
		getContent(50, items[i].pubDate, "<pubDate>", "</pubDate>", text);
		getContent(512, items[i].title, "<title>", "</title>", text);
		getContent(256, items[i].url, "<link>", "</link>", text);
		cleanUrl(items[i].url);
		getTitle(items[i].title);
		strcpy(items[i].agency, newsItems.agency);
	}
}

int refreshFeed(struct newsAgency newsAgency)
{
	int k = 0;
	FILE *fptr;

	int fileSize = fsize(newsAgency.name);
	if (fileSize == -1){
		fprintf(logptr, "ERROR:%s:Could not stat size of feed file\n", newsAgency.name);
		return 1;
	}
	/*  open for reading */
	fptr = fopen(newsAgency.name, "r");
	if (fptr == NULL){
		fprintf(logptr, "ERROR:Could not open file feed.xml for reading \n");
		return 1;
	}
	int nlines = 0;

	int state=OUT,i=0,j=0,pos=0;
	char a;
	char buffer[BUFFER_SIZE];
	char identifier[12];
	memset(identifier, 0, 12);

	do { //this do loop will capture all the texts in between item tags
		a = fgetc(fptr);
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
				if(nlines<MAX_TITLES && countTitleWords(buffer)){
					strncpy(newsItems.items[nlines], buffer, 512);
					newsItems.items[nlines++][511] = '\0';
				}
			}
		}

	} while (a != EOF && ++j<fileSize);// some files don't have EOF

	newsItems.size = nlines;
	strcpy (newsItems.agency, newsAgency.name);
	fclose(fptr);
	return 0;
}

void download_feed(FILE *dst, const char *src){
	CURL *handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, src);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, dst);
	curl_easy_perform(handle);
	curl_easy_cleanup(handle); //without this program will leak memory and eventually crash...learned it the hard way
}

int fsize(const char *filename) {
	struct stat st;
	if (stat(filename, &st) == 0)
		return (st.st_size + 0);

	return -1;
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
	static char *suffix = "</TITLE>";
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
	int size = BUFFER_SIZE - (p1 - buff);
	//cleanTitle(size, p1);
	escapeQuotes(p1);
	strcpy(line, p1);
}

//Removes 3 consecutive chars with MSB (utf-8) set to 1 and replaces them with a '\''
void cleanTitle(int size, char *buff){
	int i;
	for(i=0;i<size;i++){
		if(buff[i]>>7){
			strncpy( buff+i, buff+i+2, size-i-2);
			buff[i] = '\'';
			cleanTitle(size, buff); //recursive in case there are more than one ocurrences of weird quotes
			break;
		}
	}
}


void cleanUrl(char *url){
	static char *cdata = "<![CDATA[";

	if(strstr(url, cdata) != &url[0]) return;
	int cdataSize = strlen(cdata);
	int last = strlen(url) - cdataSize - 3;
	memcpy(url, &url[cdataSize], strlen(url) - cdataSize);
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

	//strcat(json, rhonda);
	strcat(json, extra.html);
	strcat(json, "','");
        strcat(json, extra.imgurl);
        //strcat(json, "extra.imgurl");
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
        if(p = strstr(title, "'")){
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

void downloadFeeds(int type){
	struct newsAgency *pp = &news_agency;
	do{
		if (pp ->type==type && fork() == 0 ){
			FILE *fptr;

			fptr = fopen(pp ->name, "w");
			if (fptr == NULL){
				fprintf(logptr, "ERROR:%s:Could not open file for writing \n", pp ->name);
				_exit(1);
			}

			download_feed(fptr, pp ->url);
			fclose(fptr);
			_exit(0);
		}
	}while((pp = pp ->next) != NULL);
	//parent code
	pid_t pid;
	// wait for ALL children to terminate
	do {
		pid = wait(NULL);
	} while (pid != -1);
}

void getparticulars(char *url, struct extra *extra)
{
  CURL *curl_handle;
  CURLcode res;
  char *heroku = "http://newspaper-demo.herokuapp.com/articles/show?url_to_clean=";
  struct MemoryStruct chunk;
  char cartera[512];
  strcpy(cartera, heroku);
  strcat(cartera, url);


  chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, cartera);

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* get it! */
  res = curl_easy_perform(curl_handle);

  /* check for errors */
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
  else {
    //    printf("%lu bytes retrieved\n", (long)chunk.size);
    char *marker = "<img src=\"";
    char *pstart = strstr(chunk.memory, marker);

    if(pstart){
      char *pend = strstr(pstart, "\"/>");
      if(pend){
	char buffer[256];
	pstart += strlen(marker);
	size_t size = pend - pstart;
	strncpy(buffer, pstart, size);
	buffer[size] = '\0';
	//	puts(buffer);
	strcpy(extra -> imgurl, buffer);
      }
    }

    puts(chunk.memory);
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  //  free(chunk.memory);

  /* we're done with libcurl, so clean it up */
  curl_global_cleanup();


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

void fillStruct(char *url, struct extra *beth){
        char tumia[512];
	char *cc;
        strcpy(tumia, "./neri.py '");
        strcat(tumia, url);
        strcat(tumia, "'");
        char gigi[16384];
	memset(gigi, 0, 16384);
        FILE *pp = popen(tumia, "r");

        if (pp != NULL) {
		int i = 0;
  		if(fgets(tumia, sizeof(tumia), pp) != NULL) {
			strcpy(beth->imgurl, tumia);
			if((cc = strstr(beth->imgurl, "\n")))
				*cc = '\0';
  		}

  		if(fgets(gigi, sizeof(gigi), pp) != NULL) {
			strcpy(beth->html, gigi);
		}
  		if(fgets(gigi, sizeof(gigi), pp) != NULL) {//twice just in case
			strcat(beth->html, gigi);
		}

		size_t sz = strlen(beth->html);
		if(beth->html[sz-1] == '\n'){
			beth->html[sz-1] = '\0';
			fprintf(stdout, "html_len:%d, url:%s\n", sz, url);fflush(stdout);
		}

                pclose(pp);
        }
	else
                puts("could not open pipe");


}
