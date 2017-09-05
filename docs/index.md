---
layout: default
title: "Introduction to NS3 Docker Emulator"
---

Network Emulator based on NS3 and Docker, with the help of NS3 docs, Docker docs and some ideas from some research papers.

<img src="https://github.com/chepeftw/NS3DockerEmulator/raw/master/NS3DockerEmulatorSchema.jpg" alt="NS3 Docker Emulator Schema" style="width: 100%;">

Make sure you have everything installed, for detailed instructions refer to the [Preliminaries](/NS3DockerEmulator/doc/preliminaries.html). You can also check the "install.sh" script provided by @ptrsen (Thank you very much 👍).

The existing documentation is intended for the latest version at the moment, which is [v2.0](https://github.com/chepeftw/NS3DockerEmulator/releases/tag/v2.0).

## NS3 Docker Emulator

This Emulator comes from different parts. In terms of "timeline", the first concept came with NS3, this article should be one of the firsts to dive into this:

[How to use Linux Containers to set up virtual networks](https://www.nsnam.org/wiki/HOWTO_Use_Linux_Containers_to_set_up_virtual_networks)

It dates back from 2010 and latest updates where in 2013.
But this guide uses LXC which is other tool for linux containers.
Then there has been multiple publications related to enhancing NS3 simulations with linux containers.

- Handigol, Nikhil, et al. "Reproducible network experiments using container-based emulation." Proceedings of the 8th international conference on Emerging networking experiments and technologies. ACM, 2012.
- Calarco, Giorgio, and Maurizio Casoni. "On the effectiveness of Linux containers for network virtualization." Simulation Modelling Practice and Theory 31 (2013): 169-185.
- Chan, Min-Cheng, et al. "Opennet: A simulator for software-defined wireless local area network." 2014 IEEE Wireless Communications and Networking Conference (WCNC). IEEE, 2014.
- Bustos-Jiménez, Javier, et al. "Boxing experience: Measuring QoS and QoE of multimedia streaming using NS3, LXC and VLC." Local Computer Networks Workshops (LCN Workshops), 2014 IEEE 39th Conference on. IEEE, 2014.
- To, Marco Antonio, Marcos Cano, and Preng Biba. "DOCKEMU--A Network Emulation Tool." Advanced Information Networking and Applications Workshops (WAINA), 2015 IEEE 29th International Conference on. IEEE, 2015.

The final publication was my inspiration and it appears to be the first paper in consider the connection between NS3 and Docker.

### Running it

<img src="/NS3DockerEmulator/diagrams/version2flow.png" alt="Version 2 Flow" style="width:100%;padding-top:30px;padding-bottom:30px">

For now it will run a [Beacon](https://github.com/chepeftw/Beacon) demo, which basically is a GoLang program in each docker that it is a beacon as the name suggest hehe, and it will broadcast a HelloWorld message through json and it will be logged into the host filesystem into ./var/log.

To run it you have 3 stages which will allow you to run highly scalable emulations.
The following parameters are used among the 3 stages:
- -n is for the number of nodes
- -s is for the size of the network
- operation string, it can be "create", "ns3", "simulation" or "destroy"

#### Create

```bash
python3 main.new.py -n 20 -s 300 create # Creates containers and bridges
python3 main.new.py -n 20 -s 300 ns3 # Starts a NS3 process
```

This will create the N Docker containers, bridges and tap interfaces, and will configure everything.
Then it will start the NS3 process.

#### Iterate

```bash
python3 main.new.py -n 20 -s 300 simulation
```

This is the highly scalable part of the emulation.
So what this does, is that it restarts the containers and makes sure everything is in place.
By doing this, it restarts the app you are running inside the containers, allowing you to run a test.
Then without destroying everything you just reinvoke the same command to start again.

#### Destroy

```bash
python3 main.new.py -n 20 -s 300 destroy
```

This destroys everything.

#### Difference with version 1

[Version 1](/NS3DockerEmulator/legv1/homelegacy.html) had a more stream line process, it was create, emulate, destroy, repeat.
But then I tried to run emulations for 20, 30, 40, 60, 80 and 100 nodes, each one running parallel in an isolated AWS EC2 instance, each one of them running 200 cycles. My idea was to generate a massive amount of data to analyze and all hell broke loose. Sometimes emulations ran correctly and sometimes it just stopped working because Docker engine said "no  baby" and it failed in cascade. After a lot of debugging and googling I ran into this article:

[Running 1,000 containers in Docker swarm](https://blog.codeship.com/running-1000-containers-in-docker-swarm/)

So this provide a solution and an idea of what was happening, but I thought that it was a waste of time to do the whole version 1 flow.
Therefore this "restart containers" loop was introduced.
So with this new version 2 flow, the emulator will not run into Docker cache problems and it will save time in the process.

### Next?

You can connect to the instances and start doing tasks like ping and watch the behavior. You can also go to the NS3 file and change the mobility patterns, size of the scenario, node density and any other parameter.

There are more updates coming for the future, if you have an idea or a contribution I'll be glad and happy to receive it or talk it over.
