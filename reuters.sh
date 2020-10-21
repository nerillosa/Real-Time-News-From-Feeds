#!/bin/bash

actualsize=$(wc -c <"/home4/nerillos/public_html/reuters.log")
if [ $actualsize -ge 100000 ]; then
    echo "" > /home4/nerillos/public_html/reuters.log
fi
/home4/nerillos/public_html/createReuters 2>>/home4/nerillos/public_html/reuters.log

