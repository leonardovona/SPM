#!/bin/bash

VIDEOS=( test_1_720 ) #test_2_270 test_2_360 test_2_720 test_2_1080 test_3_1080 test_4_720 test_5_1080 )

for VIDEO in "${VIDEOS[@]}"
do
	:
	for((I=0;I<10;I++))
	do
		:
		for((J=2;J<=32;J*=2))
		do
			:
			./pthread_av_tc ../video_tests/${VIDEO}.mp4 45 ${J} | awk '{print $2}' >> ./tests/pthread_av_tc_${VIDEO}_${J}.txt
		done
	done
done
