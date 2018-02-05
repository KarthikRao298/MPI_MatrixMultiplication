#!/bin/sh
# File Name       :runMulStrong.sh
# Description     :Script to execute the matrix multiplication with strong scaling
# Author          :Karthik Rao
# Date            :Dec 06 2017
# Version         :0.1

RESULTDIR=result/
h=`hostname`

if [ "$h" = "mba-i1.uncc.edu"  ];
then
    echo Do not run this on the headnode of the cluster, use qsub!
    exit 1
fi

if [ ! -d ${RESULTDIR} ];
then
    mkdir ${RESULTDIR}
fi
    
mpirun ./matMul ${N} 2> ${RESULTDIR}/mulStrong_${N}_${PROC}  >/dev/null


