/*
* Compile with : gcc comercioNews.c lex.yy.c -o createComercio `mysql_config --cflags --libs`
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <my_global.h>
#include <mysql.h>
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

struct item{
	int type;
	char title[URL_TITLE_LEN];
	char agency[10];
	char pubDate[50];
	char url[URL_TITLE_LEN];
};

struct section{
	char* name;
	int news_type;
} sectionArray[] = {{"lima", 9}, {"politica", 11}, {"economia", 13}} ;

static int SECTION_ARRAY_SIZE = sizeof(sectionArray)/sizeof(sectionArray[0]);


//Function Prototypes
static size_t parseUrlWithFlex(char *url, char **encoded, struct extra *extra);
static char *base64_encode(char *data, size_t input_length,  size_t *output_length);
static void getInsertString(struct item *item, char *json, struct extra *extra);
static void deleteOldRecords(int type);
static void createNewsSection(struct section *section);
static void printTime(char *msg);
static void initMysql();

char *yybuf;  // flex writes to this global variable
int agencyState = COMERCIO; //global variable that flex uses to define start state
MYSQL *con ;

int main(int argc, char *argv[]){
	printTime("\nstart");

	initMysql();
	setbuf(stderr, NULL);
	int i;
	for(i=0;i<SECTION_ARRAY_SIZE;i++){
		struct section section = sectionArray[i];
		createNewsSection(&section);
	}
	mysql_close(con);

	printTime("end");
}

void createNewsSection(struct section *section){
	char tumia[BUFFER_SIZE];
	char *pch;

	strcpy(tumia, "perl /home1/nerillos/public_html/comercio.pl ");
	strcat(tumia, section ->name);

	FILE *pp = popen(tumia, "r");

	if (pp != NULL) {
  		if(fgets(tumia, sizeof(tumia), pp) != NULL) {
			tumia[BUFFER_SIZE - 1] = '\0';
			pch = strtok (tumia, " ");
			int isErr = 0;
  			while (pch != NULL){
				char *encoded = NULL;
				struct extra extra;
				memset(&extra, 0, sizeof(struct extra));
				size_t out_len = parseUrlWithFlex(pch, &encoded, &extra);

				if(out_len>20 && out_len<BUF_LEN*4){
					strncpy(extra.html, encoded, out_len );
					extra.html[out_len] = '\0'; //terminate
					if(!extra.imgurl[0]){
						strcpy(extra.imgurl, "http://nllosa.com/images/COMERCIO.png");
					}
					char buff[BUF_LEN*8];
					buff[0] = '\0';
					struct item item;
					memset(&item, 0, sizeof(struct item));
					item.type = section ->news_type;
					strcpy(item.agency, "COMERCIO");
					strcpy(item.url, "https://elcomercio.pe");
					strcat(item.url, pch);
					getInsertString(&item, buff, &extra);
					if (buff[0] && mysql_query(con, buff)){
						fprintf(stderr, "ERROR ADDING RECORD for %s: %s\n", pch, mysql_error(con));
						isErr = 1;
					}
					//fprintf(stdout, "Insert String:%s\n", buff);
				}else{
        				fprintf(stderr, "Error for Url: %s\n", pch);
				}
				free(encoded);

   				pch = strtok (NULL, " ");
  			}

			if(!isErr){
				deleteOldRecords(section ->news_type);
			}

  		}
           pclose(pp);
	}
	else{
		fprintf(stderr, "Error could not open pipe: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

size_t parseUrlWithFlex(char *url, char **encoded, struct extra *extra){
        char beth[1024];
	strcpy(beth, "wget --timeout=180 -q -O - ");
	strcat(beth, "https://elcomercio.pe");
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
		fprintf(stderr, "Error could not open news URL for parseUrl: %s\n", strerror(errno));
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
			if(p=strstr(line,"dateTime=")){
				strncpy(extra-> pubDate, p+10, 19); //2018-01-01T13:00:00
				p = extra-> pubDate;
				while(*p++ != 'T'){} //replace the T with a space
				*(p-1) = ' ';
				foundd = 1;
			}
		}
		if(!found2){
			if(strstr(line,"property=\"og:title\"")){
		        	p = strstr(strstr(line,"property=\"og:title\""), "content=\"");
	        		if(p){
	        			char *m = strstr(p+10, "\"");
        				if(m){
        					strncpy(extra-> title, p+9, m-p-9);
        					found2 = 1;
        				}
				}
			}
		}

		if(!start && strstr(line,"dateTime=")){ //start flex scanning from here
			start = 1;
		}
		else if(!start){
			continue;
		}
		if((countFilled + read) >= BUF_LEN*10 ){
			fprintf(stderr,"Buffer Size Exceeded!!!!:%d for %s\n", countFilled + read, url);
			return 0;
		}

		memcpy(linda + countFilled, line, read);
		countFilled = countFilled + read;
	}
	free(line);
//	fprintf(stdout,"%d, %s\n", countFilled, url);
	pclose(ppp);
	linda[countFilled-read] = '\0';
	FILE *pp = fmemopen(linda, countFilled-read, "r");

        yyin = pp;
        char tumia[BUF_LEN];
	memset(tumia, 0x00, BUF_LEN);

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

void getInsertString(struct item *item, char *buff, struct extra *extra){

	if(extra -> html[0]){ // there is html
			char beth[5];
			sprintf(beth, "%d", item -> type);

			strcpy(buff, "REPLACE INTO news (url,html,img,news_type,title,pubdate,agency,create_date) values ('");
			strcat(buff, item ->url);
			strcat(buff, "','");
			strcat(buff, extra ->html);
			strcat(buff, "','");
        		strcat(buff, extra ->imgurl);
			strcat(buff, "',");
			strcat(buff, beth);
			strcat(buff, ",'");
			strcat(buff, extra ->title);
			strcat(buff, "',STR_TO_DATE('");
			strcat(buff, extra -> pubDate);
			strcat(buff, "','%Y-%m-%d %H:%i:%S'),'");
			strcat(buff, item ->agency);
			strcat(buff, "',now())");
	}
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

void initMysql(){
        con = mysql_init(NULL);
        if (con == NULL){
                fprintf(stderr, "ERROR:mysql_init() failed\n");
                exit(EXIT_FAILURE);
        }
	if (mysql_real_connect(con, "localhost", "nerillos_neri", "carpa1", "nerillos_neri", 0, NULL, 0) == NULL){
                fprintf(stderr, "ERROR:%s\n", mysql_error(con));
                mysql_close(con);
                exit(EXIT_FAILURE);
        }
}

void deleteOldRecords(int type){

	char buff[URL_TITLE_LEN];
	strcpy(buff, "DELETE FROM news WHERE agency = 'COMERCIO' AND news_type=");
        char beth[5];
       	sprintf(beth, "%d", type);
	strcat(buff, beth);
	strcat(buff, " AND create_date < now() - INTERVAL 1 hour");
	if (mysql_query(con, buff)){
		fprintf(stderr, "ERROR ON DELETE:%s\n", mysql_error(con));
	}

}

void printTime(char *msg){
	time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char s[URL_TITLE_LEN];
        strcpy(s, msg);
        strcat(s, ": ");
        strftime(s+strlen(msg)+2, URL_TITLE_LEN-strlen(msg)-2, "%c", tm);
        fprintf(stderr, "%s\n", s);
}

