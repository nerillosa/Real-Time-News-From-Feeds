#!/bin/bash

actualsize=$(wc -c <"/home4/nerillos/public_html/larepublica.log")
if [ $actualsize -ge 100000 ]; then
    echo "" > /home4/nerillos/public_html/larepublica.log
fi
/home4/nerillos/public_html/createRepublica 2>>/home4/nerillos/public_html/larepublica.log

