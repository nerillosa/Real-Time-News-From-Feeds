#!/bin/bash

echo `wget --timeout=180 -S -O - $1 2>/dev/null` | ./img.out

