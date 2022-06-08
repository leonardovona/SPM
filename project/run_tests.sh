#!/bin/bash

#MISSING: AWK

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
				for((J=2;J<=32;J*=2))
				do
					:
					./${EXECUTABLE}_av ./opencv_tests/video_tests/${VIDEO}.mp4 45 ${J} | grep computed >> ./tests/${EXECUTABLE}_${VIDEO}_${J}.txt #| awk '{print $6}'
				done
			fi
		done
	done
done

#for video in "${videos[@]}"
#do
#	:
#	for((j=0;i<10;j++))
#   	do
#   		:
#   		./sequential_av ../video_tests/$video.mp4 45; done | grep computed | awk '{print $6}' > test_results/sequential_test_$video.txt
#	for((i=2;i<=32;i*=2))
#	do
#		:
#   		for((j=0;i<10;j++))
#   			do
#   			:
#   			./pthread_av ../video_tests/$video.mp4 45 $i; done | grep computed > test_results/pthread_test_$video_$i.txt  
#   			./ff_av ../video_tests/$video.mp4 45 $i; done | grep computed > test_results/ff_test_$video_$i.txt 
#		done
#	done
#done
