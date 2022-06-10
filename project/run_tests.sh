#!/bin/bash

VIDEOS=( test_2_270 test_2_360 test_2_720 test_2_1080 )
EXECUTABLES=( sequential pthread ff )

for EXECUTABLE in "${EXECUTABLES[@]}"
do
	:
	for VIDEO in "${VIDEOS[@]}"
	do
		:
		for((I=0;I<10;I++))
		do
			:
			if [[ "${EXECUTABLE}" == "sequential" ]]; then
				./${EXECUTABLE}_av ./opencv_tests/video_tests/${VIDEO}.mp4 45 | grep computed >> ./tests/${EXECUTABLE}_${VIDEO}.txt #| awk '{print $6}'
			else
				for((J=2;J<=4;J*=2))
				do
					:
					./${EXECUTABLE}_av ./opencv_tests/video_tests/${VIDEO}.mp4 45 ${J} | grep computed >> ./tests/${EXECUTABLE}_${VIDEO}_${J}.txt #| awk '{print $6}'
				done
			fi
		done
	done
done
