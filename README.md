Linux高精度的批量ping

实测150个IP，在vps上精确度为±1ms。（在WSL上精确度或许不足）

- 配置文件

  **config.ini、iplist.txt手动拷贝到bin文件同目录**

```ini

#uint 读取列表时间间隔（分钟）  默认5，范围[5,10]
UpdateListTimeMinutes = 5

#string ip列表文件
UpdateListFile = iplist.txt

#uint 一大轮测速间隔（秒） 默认60，范围[30,300]
PingIntervalSecond = 10

#uint (一小轮)每个IP测数次数（次）默认2，范围[1,50]
PingIPCount = 10

#uing (一小轮)每个IP测速间隔(毫秒)  默认10，范围[10,100]
PingIPIntervalMs = 10

#uint 测速超时时间（毫秒）默认1000，范围[100,2000]
PingTimeOutMs = 1000

```

- 原理

  假设每个IP测10次，那么就启动10个线程来recv，再启动10个线程send，并计算好每个线程的ping时间，用std::this_thread::sleep_until(tp_begin);。

  使用异步sendto，假设sendto不耗时，由此来精确控制ping间隔。

- 测速示例 共35个IP，每个IP测10次，每次间隔10ms

```base
[**** Debug]# ./FastPing.out 
net.ipv4.ping_group_range = 0 0
UpdateListFile            = iplist.txt
UpdateListTimeMinutes     = 5
PingIntervalSecond        = 60
PingIPCount               = 10
PingIPIntervalMs          = 10
PingTimeOutMs             = 1000

set_ip_list_cache size: 35
开始测速 次数: 10, 间隔10ms, 超时: 1000ms, 理论耗时(次数*间隔+超时+1秒): 2100ms
addr: 18.166.83.241  , s/r:  10/10  (  0%), avg:   2.166ms
addr: 18.182.229.53  , s/r:  10/10  (  0%), avg:  53.315ms
addr: 18.184.144.251 , s/r:  10/10  (  0%), avg: 212.830ms
addr: 23.50.192.1    , s/r:  10/10  (  0%), avg: 278.36 ms
addr: 23.58.222.1    , s/r:  10/10  (  0%), avg: 346.581ms
addr: 23.97.97.8     , s/r:  10/10  (  0%), avg: 350.696ms
addr: 23.98.144.52   , s/r:  10/10  (  0%), avg: 238.644ms
addr: 23.194.192.1   , s/r:  10/10  (  0%), avg: 113.829ms
addr: 27.102.118.1   , s/r:  10/10  (  0%), avg:  40.194ms
addr: 27.155.119.78  , s/r:  10/10  (  0%), avg:  33.164ms
addr: 31.41.56.1     , s/r:  10/10  (  0%), avg: 372.640ms
addr: 34.72.161.220  , s/r:  10/10  (  0%), avg: 169.516ms
addr: 34.141.68.38   , s/r:  10/10  (  0%), avg: 213.179ms
addr: 35.156.184.121 , s/r:  10/10  (  0%), avg: 197.226ms
addr: 169.56.83.75   , s/r:  10/10  (  0%), avg:  47.492ms   <-这个用于实际ping
addr: 172.65.223.1   , s/r:  10/10  (  0%), avg:   2.700ms
addr: 172.65.230.1   , s/r:  10/10  (  0%), avg:   2.89 ms
addr: 172.98.85.8    , s/r:  10/10  (  0%), avg: 214.976ms
addr: 172.107.218.1  , s/r:  10/10  (  0%), avg: 141.356ms
addr: 175.6.6.109    , s/r:   9/10  ( 10%), avg:  17.731ms
addr: 182.201.242.76 , s/r:  10/10  (  0%), avg:  58.142ms
addr: 183.2.157.1    , s/r:  10/10  (  0%), avg:  11.454ms
addr: 185.101.137.134, s/r:  10/10  (  0%), avg:  77.821ms
addr: 185.175.47.29  , s/r:  10/10  (  0%), avg: 276.876ms
addr: 185.179.202.101, s/r:  10/10  (  0%), avg: 440.633ms
addr: 185.179.203.164, s/r:  10/10  (  0%), avg: 445.341ms
addr: 185.225.208.1  , s/r:  10/10  (  0%), avg: 308.387ms
addr: 192.144.236.171, s/r:  10/10  (  0%), avg:  48.407ms
addr: 200.187.199.1  , s/r:  10/10  (  0%), avg: 345.537ms
addr: 202.136.162.1  , s/r:  10/10  (  0%), avg:  40.476ms
addr: 202.136.162.2  , s/r:  10/10  (  0%), avg:  39.56 ms
addr: 203.116.132.193, s/r:  10/10  (  0%), avg:  35.947ms
addr: 209.58.169.122 , s/r:  10/10  (  0%), avg:  35.760ms
addr: 217.147.89.1   , s/r:  10/10  (  0%), avg: 294.303ms
addr: 217.182.200.7  , s/r:  10/10  (  0%), avg: 312.317ms
测速结束,总耗时: 2100ms
^C
[**** Debug]# 
```

- 实际ping结果

  与ping程序测速相差47.574 - 47.492= 0.082ms

```
[***** Debug]# ping 169.56.83.75
PING 169.56.83.75 (169.56.83.75) 56(84) bytes of data.
64 bytes from 169.56.83.75: icmp_seq=1 ttl=53 time=47.4 ms
64 bytes from 169.56.83.75: icmp_seq=2 ttl=53 time=47.3 ms
64 bytes from 169.56.83.75: icmp_seq=3 ttl=53 time=47.6 ms
64 bytes from 169.56.83.75: icmp_seq=4 ttl=53 time=47.4 ms
64 bytes from 169.56.83.75: icmp_seq=5 ttl=53 time=47.7 ms
64 bytes from 169.56.83.75: icmp_seq=6 ttl=53 time=47.5 ms
64 bytes from 169.56.83.75: icmp_seq=7 ttl=53 time=47.5 ms
64 bytes from 169.56.83.75: icmp_seq=8 ttl=53 time=47.6 ms
64 bytes from 169.56.83.75: icmp_seq=9 ttl=53 time=47.6 ms
64 bytes from 169.56.83.75: icmp_seq=10 ttl=53 time=47.7 ms
64 bytes from 169.56.83.75: icmp_seq=11 ttl=53 time=47.3 ms
64 bytes from 169.56.83.75: icmp_seq=12 ttl=53 time=47.1 ms
64 bytes from 169.56.83.75: icmp_seq=13 ttl=53 time=47.5 ms
64 bytes from 169.56.83.75: icmp_seq=14 ttl=53 time=47.8 ms
64 bytes from 169.56.83.75: icmp_seq=15 ttl=53 time=47.9 ms
64 bytes from 169.56.83.75: icmp_seq=16 ttl=53 time=47.4 ms
64 bytes from 169.56.83.75: icmp_seq=17 ttl=53 time=47.6 ms
64 bytes from 169.56.83.75: icmp_seq=18 ttl=53 time=47.7 ms
64 bytes from 169.56.83.75: icmp_seq=19 ttl=53 time=47.5 ms
^C
--- 169.56.83.75 ping statistics ---
19 packets transmitted, 19 received, 0% packet loss, time 18025ms
rtt min/avg/max/mdev = 47.149/47.574/47.920/0.235 ms
```

