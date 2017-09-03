#!/bin/bash

ETH0=$(ifconfig -a | grep eth0 | wc -l)

while [ $ETH0 -eq 0 ]
do
  echo "waiting ... "
  sleep 2
  ETH0=$(ifconfig -a | grep eth0 | wc -l)
done

echo "starting ... "
echo "---------------------------------------------"

mkdir -p /var/log/golang
#touch /var/log/golang/treesip.log

#olsrd -i eth0 -f /data/olsrd.conf
#sleep 20
# babeld eth0 &

# echo "---------------------------------------------" >> /var/log/golang/treesip.log
# iptables -F
# iptables -A INPUT -p tcp -m tcp --dport 10002 -j ACCEPT
# iptables -A INPUT -p icmp -j ACCEPT
# iptables -P OUTPUT ACCEPT

# iptables -L -v >> /var/log/golang/treesip.log
# echo "---------------------------------------------" >> /var/log/golang/treesip.log

# sleep 20
# route >> /var/log/golang/treesip.log
# echo "----------------------" >> /var/log/golang/treesip.log
# ping -c 2 -R -i 1 10.12.0.9 >> /var/log/golang/treesip.log
# echo "----------------------" >> /var/log/golang/treesip.log
# nc -z -n -v 10.12.0.4 10002 >> /var/log/golang/treesip.log

$GOBIN/Gossip /treesip/conf.yml &
#$GOBIN/Treesip 10001 1 &
#$GOBIN/Treesip 10002 10 1

$GOBIN/Treesip /treesip/conf1.yml &
$GOBIN/Treesip /treesip/conf2.yml

# echo "------------------------------------------------------"

# ping -c 10 -R 10.12.0.1
# for i in {1..5}
# do
# 	echo "Welcome $i times"
# 	echo "ping 10.12.0.$i"
# 	# route
# 	ping -R -c 10 10.12.0.$i
# 	echo ""
# 	echo "------------------------------------------------------"
# 	echo ""
# done
