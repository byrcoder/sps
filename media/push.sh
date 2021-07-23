#:!/bin/bash

i=0
while [ $i -lt 3 ]
do
 ffmpeg -re -i hy.flv -vcodec copy -acodec copy -f flv "rtmp://127.0.0.1/live/test?xx=wt&xxy=d"
 i=`expr $i + 1`
done
