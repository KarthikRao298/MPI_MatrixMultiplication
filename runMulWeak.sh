#!/bin/sh
# File Name       :runMulWeak.sh
# Description     :Script to execute the matrix multiplication with weak scaling
# Author          :Karthik Rao
# Date            :Dec 09 2017
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
    
mpirun ./matMul ${N} 2> ${RESULTDIR}/matMulWeak_${N}_${PROC}  >/dev/null



