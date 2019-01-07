# router

For exp3, use the following command to generate rip executable file:

```bash
g++ rip.cpp -orip -O2 -lpthread -DRIP_MAIN

# RUN:
./rip
```

The rip output log may seem like this:

```
get 192.168.  4.  1	iface enp7s0
get 172. 16.  9.  1	iface vmnet1
get 172. 16.181.  1	iface vmnet8
get 192.168.  3.  2	iface enx000ec6aa5f42
-----------------------------------------------------
   |        Network       |   Nexthop Addr  | Metric 
-----------------------------------------------------
 C | 192.168.  4.  1 / 24 |   0.  0.  0.  0 |    1
 C |   3.  0.  0.  0 / 24 |   0.  0.  0.  0 |   16
 C | 172. 16.  9.  1 / 24 |   0.  0.  0.  0 |    1
 C | 172. 16.181.  1 / 24 |   0.  0.  0.  0 |    1
 C | 192.168.  3.  2 / 24 |   0.  0.  0.  0 |    1
-----------------------------------------------------
send to 192.168.  4.  1 succeed! packet len: 64
send to 172. 16.  9.  1 succeed! packet len: 64
send to 172. 16.181.  1 succeed! packet len: 64
send to 192.168.  3.  2 succeed! packet len: 64
receive len: 64	cmd: 2	ver: 2	from 192.168.3.1
get 192.168.  4.  1	iface enp7s0
get 172. 16.  9.  1	iface vmnet1
get 172. 16.181.  1	iface vmnet8
get 192.168.  3.  2	iface enx000ec6aa5f42
-----------------------------------------------------
   |        Network       |   Nexthop Addr  | Metric 
-----------------------------------------------------
 C | 192.168.  4.  1 / 24 |   0.  0.  0.  0 |    1
 C |   3.  0.  0.  0 / 24 |   0.  0.  0.  0 |   16
 C | 172. 16.  9.  1 / 24 |   0.  0.  0.  0 |    1
 C | 172. 16.181.  1 / 24 |   0.  0.  0.  0 |    1
 C | 192.168.  3.  2 / 24 |   0.  0.  0.  0 |    1
 R | 192.168.  1.  0 / 24 | 192.168.  3.  1 |    2
 R | 192.168.198.  0 / 24 | 192.168.  3.  1 |    3
 R | 192.168. 10.  0 / 24 | 192.168.  3.  1 |    3
-----------------------------------------------------
--- 192.168.  1.  0/24 192.168.  3.  1
--- 192.168.198.  0/24 192.168.  3.  1
--- 192.168. 10.  0/24 192.168.  3.  1
send to 192.168.  4.  1 succeed! packet len: 124
send to 172. 16.  9.  1 succeed! packet len: 124
send to 172. 16.181.  1 succeed! packet len: 124
send to 192.168.  3.  2 succeed! packet len: 64
--- 192.168.  1.  0/24 192.168.  3.  1
--- 192.168.198.  0/24 192.168.  3.  1
--- 192.168. 10.  0/24 192.168.  3.  1
receive len: 24	cmd: 1	ver: 2	from 192.168.4.2
send to 192.168.  4.  1 succeed! packet len: 144
--- 192.168.  1.  0/24 192.168.  3.  1
--- 192.168.198.  0/24 192.168.  3.  1
--- 192.168. 10.  0/24 192.168.  3.  1
get 192.168.  4.  1	iface enp7s0
get 172. 16.  9.  1	iface vmnet1
get 172. 16.181.  1	iface vmnet8
get 192.168.  3.  2	iface enx000ec6aa5f42
```

For exp4, just type `mv lookuproute_simple.cpp.bak lookuproute.cpp; make -j` and run `sudo ./main`.

由于为了后续性能，把输出的部分给注释掉了。

## Additional exp

编译：直接 `make -j`

就写了个哈希边表，每次从/32查到/1，看看有没有对应的路由表项

可以理解为我开了32个数组，但是存的时候存得紧凑了点

该算法代码位于 `lookuproute.cpp` 中，不到30行（后面常数优化了一波，循环展开，做到差不多和4位trie一样快了，代码翻了一倍）

事实上是/32到/13存hash，/12到/1直接数组存下来
