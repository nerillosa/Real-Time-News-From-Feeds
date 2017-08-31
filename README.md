# Real-Time-News-From-Feeds
This web project shows news in many categories from several news agencies. News is harvested from news feeds and is updated every 10 minutes.
## Synopsis
This project uses php as the web backend and mysql as the database to save news records. 
The heart of the project is a C program, createNewsCron.c, that is run every 15 minutes using a cron job. This program takes in a text file as a command line argument.
This text file (news_urls.txt) contains lines with the following comma separated values:<br> News Type (an integer), News Agency (short name), and the url of the news feed. Some of the news agencies we scrap news from are CNN, FOX, REUTERS, BBC, USA Today, etc.
There are currently seven news types (defined in the news_type table): Politics, Science, World, Sports, Entertainment, Health, USA, Business, and TRUMP. The default type which comes up when you initially visit the web site is TRUMP news.
## Code
The standalone C program, reads each line of news_urls.txt by type. Once it gets all the feeds for the url type, it sorts them all by descending date and gets the 20 most recent for each type and saves them to the mysql news table.
The program also takes care of only keeping the 80 most recent news for each type, so any given time the news table will hold at the most 560 records (7 news types * 80 records). The TRUMP category gets selected news containing the word "Trump" from all the others. 
## Flex Scanner
The project utilizes the free FLEX lexical analyzer (https://www.gnu.org/software/flex/) for scraping off the news from the news urls. It is also used to extract the main image for each article. FLEX speeds up tremendously the processing of each url. The program can process almost 2000 news urls in less than 2 minutes. The file multiple.lex defines the patterns that FLEX will use to extract the news information for <i>each</i> news agency and the file img.lex defines the patterns to look for to extract the main image. The FLEX compiler is used to produce a C source file called lex.yy.c that is compiled together with createNewsCron.c :
## Compiling the executable
To compile the code into an executable (createNews) that can be called by the Cron job you do the following:<br><br>
gcc createNewsCron.c lex.yy.c -o createNews -lcurl `mysql_config --cflags --libs`

<br><br>
<b> Demo:  http://nllosa.com/news.html </b>

