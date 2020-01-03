#!/bin/bash

actualsize=$(wc -c <"/home1/nerillos/public_html/comercio.log")
if [ $actualsize -ge 100000 ]; then
    echo "" > /home1/nerillos/public_html/comercio.log
fi
/home1/nerillos/public_html/createComercio 2>>/home1/nerillos/public_html/comercio.log

