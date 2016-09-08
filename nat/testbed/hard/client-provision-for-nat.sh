. ./config.sh

ifconfig $CLIENT_DEVICE_DIRECT up
ip addr flush dev $CLIENT_DEVICE_DIRECT
ip addr add $CLIENT_IP_DIRECT/24 dev $CLIENT_DEVICE_DIRECT
ifconfig $CLIENT_DEVICE_INDIRECT up
ip addr flush dev $CLIENT_DEVICE_INDIRECT
ip addr add $CLIENT_IP_INDIRECT/24 dev $CLIENT_DEVICE_INDIRECT
ip route flush $EXTERNAL_SUBNET
ip route add $EXTERNAL_SUBNET/24 via $NAT_IP_INTERNAL dev $CLIENT_DEVICE_INDIRECT
ip route flush cache
arp -s $NAT_IP_INTERNAL $NAT_INTERNAL_MAC
#hping3 192.168.33.10 -k -p 47882 -s 0
