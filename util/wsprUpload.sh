#!/bin/bash

# http://wsprnet.org/automate.txt
while true;
do
    date
    curl -F allmept=@/cygdrive/c/Users/doug/AppData/Local/WSJT-X/ALL_WSPR.TXT -F call=KD2KDD -F grid=FN20xr http://wsprnet.org/meptspots.php;
    echo $?
    sleep 120;
done
