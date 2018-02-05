#!/bin/sh
# File Name       :plotMulStrong.sh
# Description     :Script to plot the speedup of matrix multiplication with strong scaling
# Author          :Karthik Rao
# Date            :Dec 06 2017
# Version         :0.1


NS="12000 51000 102000"

PROCS="4 9 16 25"

RESULTDIR=result/

if [ ! -d ${RESULTDIR} ];
then
    mkdir ${RESULTDIR}
fi

#strong scaling

    for N in ${NS};
    do
	FILE=${RESULTDIR}/seqMul_${N}
	if [ ! -f ${FILE} ]
	then
	    echo missing sequential result file "${FILE}". Did you run qSeqMul and wait for completion?
	fi

	seqtime=$(cat ${RESULTDIR}/seqMul_${N})
	
	for PROC in ${PROCS}
	do
	
	    FILE=${RESULTDIR}/mulStrong_${N}_${PROC}
	    
	    if [ ! -f ${FILE} ]
	    then
		echo missing heatEqn result file "${FILE}". Did you run qMulStrong and wait for completion?
	    fi

	    partime=$(cat ${RESULTDIR}/mulStrong_${N}_${PROC})
	    
	    echo ${PROC} ${seqtime} ${partime}
	done > ${RESULTDIR}/speedupMulStrong_${N}


	GNUPLOTSTRONG="${GNUPLOTSTRONG} set title 'matrix multiplication strong scaling. n=${N} '; plot '${RESULTDIR}/speedupMulStrong_${N}' u 1:(\$2/\$3);"
    done


gnuplot <<EOF
set terminal pdf
set output 'mul_strong_plots.pdf'

set style data linespoints

set key top left

set xlabel 'Proc'
set ylabel 'Speedup'

${GNUPLOTSTRONG}


EOF
