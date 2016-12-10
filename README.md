# Real-Time-News-From-Feeds
This web project shows news in many categories from several news agencies. News is harvested from news feeds and is updated every 5 minutes.
This project uses php as the web backend and mysql as the database to save news records. 
The heart of the project is a C program, createNewsCron.c, that is run every 5 minutes using a cron job. This program takes in a text file as a command line argument.
This text file (news_urls.txt) contains lines with the following comma separated values : News Type (an integer), News Agency (short name), and the url of the news feed.
There are currently seven news types (defined in the news_type table): Politics, Science, World, Sports, Entertainment, Health, and USA. The default type which comes up when you initially visit the web site is USA news.
The standalone C program, reads each line of news_urls.txt by type. Once it gets all the feeds for the url type, it sorts them all by descending date and gets the 20 most recent and saves them to the mysql news table.
The program also takes care of only keeping the 80 most recent news for each category, so any given time the news table will hold at the most 560 records (7 news types * 80 records).

