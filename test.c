#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <curl/curl.h>
#include "flex.h"

#define BUF_LEN 16384

char *yybuf;

struct MemoryStruct {
  char *memory;
  size_t size;
};

struct MemoryStruct chunk;
CURL *curl_handle;

static void initCurl();
static void loadFeed(char *url);
static void curl_close();
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

void parseUrl(char *url);

int main(int argc, char **argv)
{
	initCurl();

	parseUrl("http://feeds.reuters.com/~r/Reuters/domesticNews/~3/oF2uBWrDrJ0/us-jpmorgan-lawsuit-idUSKBN1521TY");

	curl_close();
}

void parseUrl(char *url){
	chunk.memory = malloc(1);
	chunk.size = 0;
	loadFeed(url);
	chunk.memory[chunk.size-2] = chunk.memory[chunk.size-1] = '\0';
	char tumia[BUF_LEN];
	tumia[0]= '\0';
	tumia[BUF_LEN-1]= '\0';
	yybuf = &tumia[0];
        YY_BUFFER_STATE bs = yy_scan_buffer(chunk.memory, chunk.size);
        yy_switch_to_buffer(bs);
        yylex();
	yy_delete_buffer(bs);
	free(chunk.memory);
	printf("%s\n", tumia);
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

void loadFeed(char *url)
{
  CURLcode res;
  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  /* get it! */

  /* example.com is redirected, so we tell libcurl to follow redirection */
  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

  res = curl_easy_perform(curl_handle);
  /* check for errors */
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
}

void curl_close(){
  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);
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
 // fprintf(stdout, "realsize:%d\n", realsize);fflush(stdout);
  return realsize;
}

