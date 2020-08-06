#!/bin/bash

actualsize=$(wc -c <"/home1/nerillos/public_html/larepublica.log")
if [ $actualsize -ge 100000 ]; then
    echo "" > /home1/nerillos/public_html/larepublica.log
fi
/home1/nerillos/public_html/createRepublica 2>>/home1/nerillos/public_html/larepublica.log

