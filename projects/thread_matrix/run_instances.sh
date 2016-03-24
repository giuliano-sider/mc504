#!/bin/bash

# script untested on mac and cygwin
# to do list: learn Python

PASTA=instances_$(date | sed "s/ /_/g; s/:/-/g; s/[^-_A-Za-z0-9]/-/g") #unique folder, timestamped
mkdir $PASTA

echo -e "Profile generated on \n$(date)\n$(uname -a)\n" >> $PASTA/profile.txt

#if [ -n "$(uname -a | grep Darwin)" ]; then
#	# Mac OS X platform        
#
#elif [ -n "$(uname -a | grep Linux)" ]; then
#	# Linux platform
#
#elif [ -n "$(uname -a | grep Cygwin)" ]; then
#	# Cygwin platform
#
#fi

matsizes=( 10 20 50 100 200 500 1000 )
threadnums=( 1 2 3 4 5 10 15 50 100 200 500 1000 )

for i in "${matsizes[@]}" ; do
	echo -e "Running instance on ${i}x${i} matrices\n"
	./random_matrix ${i} ${i} --interval -1000 1000 > $PASTA/instances${i}a_in
	./random_matrix ${i} ${i} --interval -1000 1000 > $PASTA/instances${i}b_in

	for j in "${threadnums[@]}" ; do
		
		if (( i >= j )) ; then
			echo "Testing ${i}x${i} matrix multiplication with ${j} threads"
			echo -e "Testing ${i}x${i} matrix multiplication with ${j} threads:\n\n" >> $PASTA/profile.txt
			cat $PASTA/instances${i}a_in $PASTA/instances${i}b_in | ./threaded_matrix_multiply --profile --threaded $j > $PASTA/instances${i}_thread${j}_out 2>> $PASTA/profile.txt
			echo -e -n '\n'
			echo -e '\n\n' >> $PASTA/profile.txt
		fi
		
	done
	echo -e '\n'
	echo -e '\n\n' >> $PASTA/profile.txt

done

echo -e "Finished! check the $PASTA folder for the results, with profile.txt having the timing profiles for each executed instance."
