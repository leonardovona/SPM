#!/bin/bash

for((k=0;k<=1;k++))
do
	for((l=0;l<10;l++))
	do
		./assignment_2_seq 123 10000 $k
	done
	echo
	for((j=0;j<=1;j++))
	do
		for((i=1;i<=16;i*=2))
		do
			for((l=0;l<10;l++))
			do
				./assignment_2_par_async 123 10000 $k $j $i
			done
			echo
		done
		echo
	done
	echo
done

for((k=0;k<=1;k++))
do
	for((l=0;l<10;l++))
	do
		./assignment_2_seq 123 10000 $k
	done
	echo
	for((j=0;j<=1;j++))
	do
		for((i=1;i<=16;i*=2))
		do
			for((l=0;l<10;l++))
			do
				./assignment_2_par_async 123 10000 $k $j $i
			done
			echo
		done
		echo
	done
	echo
done
