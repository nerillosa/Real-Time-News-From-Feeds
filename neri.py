#!/home1/nerillos/bin/python

import sys
import base64
from newspaper import Article

if len(sys.argv) < 2:
	print("Usage: %s url" % sys.argv[0])
	sys.exit()
first_article = Article(sys.argv[1])
first_article.download()
first_article.parse()
print(first_article.top_image)
a = str(base64.b64encode(bytearray(first_article.text, 'utf8')))
print(a[2:len(a)-1])
sys.stdout.flush()

#print(first_article.article_html)
#sys.stdout.buffer.write(first_article.top_image)
#first_article = Article(sys.argv[1], keep_article_html=True)
#str = first_article.article_html
#str = first_article.text
#tt = 0
#for line in str:
#	sys.stdout.write(line);
#	print(tt)
#	tt += 1
#print(first_article.text)
