#!/bin/bash

VIDEOS=( test_2_270 test_2_360 test_2_720 )
EXECUTABLES=( pthread )

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
				./${EXECUTABLE}_motion_detection ../video_tests/${VIDEO}.mp4 45 | grep computed | awk '{print $6}' >> ./tests/${EXECUTABLE}_${VIDEO}.txt
			else
				for((J=2;J<=32;J*=2))
				do
					:
					./${EXECUTABLE}_motion_detection ../video_tests/${VIDEO}.mp4 45 ${J} | grep computed | awk '{print $6}' >> ./tests/${EXECUTABLE}_${VIDEO}_${J}.txt
				done
			fi
		done
	done
done
