#!/bin/bash
. ./config.sh

# Runs a DPDK app for the specified scenario

# Parameters:
# $1: Scenario, can be "loopback" or "rr"
# $2: Folder of the app; the app must be named "nat"
#     and take the usual arguments e.g. "--extip"
# $3: Additional arguments for the app

if [ $1 != "loopback" -a $1 != "rr" ]; then
    echo "[bench] Unknown scenario specified in run-dpdk" 1>&2
    exit 1
fi

if [ -z $2 ]; then
    echo "[bench] No app folder specified in run-dpdk" 1>&2
    exit 2
fi

pushd $2 >> /dev/null

  echo "[bench] Building $2..."
  sudo rm build -rf
  make clean
  make

  echo "[bench] Running $2..."
  if [ $1 = "loopback" ]; then
      sudo taskset -c 8 ./build/nat -n 2 -- --wan 0 --lan-dev 1 \
          $3 \
          --extip $MB_IP_EXTERNAL \
          --eth-dest 0,$TESTER_MAC_EXTERNAL --eth-dest 1,$TESTER_MAC_INTERNAL
  else
      echo "[bench] ERROR: non-loopback is not supported" 1>&2
      exit 1
  fi

popd >> /dev/null
