#!/bin/bash
pass=$(perl /home1/nerillos/public_html/huffington.pl)
output=`/home1/nerillos/public_html/createHuff $pass`
echo $output > /home1/nerillos/public_html/huffington.json
