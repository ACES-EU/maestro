#!/bin/bash

. ./config.sh

mkdir -p latency-results

. ./sync-scripts.sh

bash ./provision-rr-nat.sh
bash dpdk-setup-middlebox-rr.sh

# NAT
echo "[TL]testing VigNAT"
(./run-nat-rr.sh $PWD 10 61000 0<&- &>nat.log) &

#sleep 5
sleep 20

echo "[TL]perform the testing"
ssh $TESTER_HOST "bash ~/scripts/latency-mf.sh $HOME/lat-res-vig-nat.txt"
scp $TESTER_HOST:lat-res-vig-nat.txt ./latency-results/vig-nat.txt

sudo pkill -9 nat

# NAT
echo "[TL]testing optimized VigNAT"
(./run-opt-nat-rr.sh $PWD 10 61000 0<&- &>nat-opt.log) &

#sleep 5
sleep 20

echo "[TL]perform the testing"
ssh $TESTER_HOST "bash ~/scripts/latency-mf.sh $HOME/lat-res-vig-nat-opt.txt"
scp $TESTER_HOST:lat-res-vig-nat-opt.txt ./latency-results/vig-nat-opt.txt

if false
then

sudo pkill -9 nat

#sleep 2
sleep 10

echo "[TL]testing NetFilter"
# NetFilter
sudo ./nf-nat-rr.sh 0<&- &>nfn.log

#sleep 10
sleep 30

echo "[TL]perform the testing"
ssh $TESTER_HOST "bash ~/scripts/latency-mf.sh $HOME/lat-res-nf-nat.txt"
scp $TESTER_HOST:lat-res-nf-nat.txt ./latency-results/nf-nat.txt

fi
