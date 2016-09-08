. ./config.sh

ifconfig $SERVER_DEVICE_INDIRECT up
ip addr flush dev $SERVER_DEVICE_INDIRECT
ip addr add $SERVER_IP_INDIRECT/24 dev $SERVER_DEVICE_INDIRECT
ifconfig $SERVER_DEVICE_DIRECT up
ip addr flush dev $SERVER_DEVICE_DIRECT
ip addr add $SERVER_IP_DIRECT/24 dev $SERVER_DEVICE_DIRECT
arp -s $CLIENT_IP_FOR_STUB $NAT_EXTERNAL_MAC
#hping3 192.168.33.10 -k -p 47882 -s 0
