---
layout: page
title: "Home"
category: legv1
date: 2017-09-05 15:56:49
order: 10
---

*This documentation is intended for [legacy version 1.0](https://github.com/chepeftw/NS3DockerEmulator/releases/tag/v1.0).*

**For the latest documentation for the latest version please refer to the [Home page](/NS3DockerEmulator/).**

-----


Network Emulator based on NS3 and Docker, with the help of NS3 docs, Docker docs and some ideas from some research papers.

<img src="https://github.com/chepeftw/NS3DockerEmulator/raw/master/NS3DockerEmulatorSchema.jpg" alt="NS3 Docker Emulator Schema" style="width: 100%;">

Make sure you have everything installed, for detailed instructions refer to the [Preliminaries](/NS3DockerEmulator/legv1/preliminarieslegacy.html). You can also check the "install.sh" script provided by @ptrsen (Thank you very much 👍).

The existing documentation is intended for version [v1.0](https://github.com/chepeftw/NS3DockerEmulator/releases/tag/v1.0).

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
<!-- , since first of all I know the authors but I couldn't find the source code, so I decided to write my own and publish it. -->

### Running it

<img src="/NS3DockerEmulator/images/diagrams/version1flow.png" alt="Version 1 Flow" style="width:100%;padding-top:30px;padding-bottom:30px">

For now it will run a [Beacon](https://github.com/chepeftw/Beacon) demo, which basically starts dockers, configures them and start a Beacon GoLang program in each docker that it is a beacon as the name suggest hehe, and it will broadcast a HelloWorld message through json and it will be logged into the host filesystem into ./var/log.

To run it you need to do one step, you need to run the main.py:

```bash
python3 ./main.py -n 5 -t 100 full
```

- -n is for the number of nodes
- -t is for the time of the simulation (this currently does not works (not sure why))
- operation string, it can be "full", "create" or "destroy"

This will create the N Docker containers, bridges and tap interfaces, and will configure everything.

After creating everything, it will run the NS3 simulation and when its done it will destroy everything.

### Next?

You can connect to the instances and start doing tasks like ping and watch the behavior. You can also go to the NS3 file and change the mobility patterns, size of the scenario, node density and any other parameter.

There are more updates coming for the future, if you have an idea or a contribution I'll be glad and happy to receive it or talk it over.
