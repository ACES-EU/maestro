
# the network:
# client{icnalsp3s1}[em3] 192.168.3.5 -- 192.168.3.2 [em3]nat{icnalsp3s2}
# nat{icnalsp3s2}[em2] 192.168.2.2 -- 192.168.2.10 [em2]server{icnalsp3s3}
# server{icnalsp3s3}[em3] 192.168.0.10 -- 192.168.0.5 [em2]client{icnalsp3s1}

CLIENT_HOST=icnalsp3s1.epfl.ch
NAT_HOST=icnalsp3s2.epfl.ch
SERVER_HOST=icnalsp3s3.epfl.ch

CLIENT_MAC=00:1e:67:92:2a:bd
SERVER_MAC=00:1e:67:92:2a:29

NAT_INTERNAL_MAC=00:1e:67:92:29:6d
NAT_EXTERNAL_MAC=00:1e:67:92:29:6c

CLIENT_IP_DIRECT=192.168.0.5
SERVER_IP_DIRECT=192.168.0.10

CLIENT_IP_INDIRECT=192.168.3.5
NAT_IP_INTERNAL=192.168.3.2
EXTERNAL_SUBNET=192.168.2.0
NAT_IP_EXTERNAL=192.168.2.2
CLIENT_IP_FOR_STUB=192.168.2.5
SERVER_IP_INDIRECT=192.168.2.10

CLIENT_DEVICE_DIRECT=em2
CLIENT_DEVICE_INDIRECT=em3

SERVER_DEVICE_DIRECT=em3
SERVER_DEVICE_INDIRECT=em2

NAT_DEVICE_INTERNAL=em3
NAT_DEVICE_EXTERNAL=em2

NAT_PCI_INTERNAL='0000:02:00.1'
NAT_PCI_EXTERNAL='0000:02:00.2'

export RTE_SDK=/home/necto/dpdk
export RTE_TARGET=x86_64-native-linuxapp-gcc

NAT_SRC_PATH=/home/necto/vnds/nat
STUB_SRC_PATH=/home/necto/vnb-nat
