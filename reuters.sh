#!/bin/bash

actualsize=$(wc -c <"/home1/nerillos/public_html/reuters.log")
if [ $actualsize -ge 100000 ]; then
    echo "" > /home1/nerillos/public_html/reuters.log
fi
/home1/nerillos/public_html/createReuters 2>>/home1/nerillos/public_html/reuters.log

