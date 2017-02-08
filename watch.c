#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]){
	if(argc<2){
     		fprintf(stdout, "usage: %s num_times\n", argv[0]);
              	exit(EXIT_FAILURE);
      	}

	int num = atoi(argv[1]);
	while(num>0){
		num--;
		int tt = system("./createNews news_urls.txt createNewsCron.log");
		if(tt == -1) {printf("error on system call\n"); break;}
		fprintf(stderr, "!!!%d left ", num);
		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		fprintf(stderr, "%s\n", asctime(tm));
		if(num) sleep(600);
	}
	return 0;
}
