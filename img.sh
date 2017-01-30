#!/bin/bash
# The following command downloads the input url and then extracts the top image using the flex program img.out
# It also creates the temp file DOWNLOAD for the main program to use to scrape the html.
wget --timeout=180 -O - $1 2>/dev/null | tee DOWNLOAD | ./img.out

