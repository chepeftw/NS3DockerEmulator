---
layout: default
title: "Introduction to NS3 Docker Emulator"
---

Network Emulator based on NS3 and Docker, with the help of NS3 docs, Docker docs and some ideas from some research papers.

<img src="https://github.com/chepeftw/NS3DockerEmulator/raw/master/NS3DockerEmulatorSchema.jpg" alt="NS3 Docker Emulator Schema" style="width: 100%;">

Make sure you have everything installed, for detailed instructions refer to the [Preliminaries](/doc/preliminaries.html). You can also check the "install.sh" script provided by @ptrsen (Thank you very much (y)).

The existing documentation is intended for the latest version at the moment, which is [v1.0](https://github.com/chepeftw/NS3DockerEmulator/releases/tag/v1.0).

## NS3 Docker Emulator

This Emulator comes from different parts. In terms of "timeline", the first concept came with NS3, this article should be one of the firsts to dive into this:

https://www.nsnam.org/wiki/HOWTO_Use_Linux_Containers_to_set_up_virtual_networks

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

To run it you need to do one step.

<!-- ### Copy the NS3 file.

You can go to the net folder and run the update.sh script:

```bash
bash update.sh tap-wifi-virtual-machine.cc
```

This will only copy the file "tap-wifi-virtual-machine.cc" to the NS3 scratch folder and rename it as "tap-vm.cc".
If your path doesn't match the path inside this script, please update it. -->

### Run the main.py

For now it will run a Beacon demo (which is in another repo of mine https://github.com/chepeftw/Beacon), which basically starts dockers, configures them and start a Beacon GoLang program in each docker that it is a beacon as the name suggest hehe, and it will broadcast a HelloWorld message through json and it will be logged into the host fs into ./var/log. You can run the main.py like this:

```bash
python3 ./main.py -n 5 -t 100 full
```

- -n is for the number of nodes
- -t is for the time of the simulation (this currently does not works (not sure why))
- operation string, it can be "full", "create" or "destroy"
<!-- - --no-cache (optional) is for forcing the rebuild of the docker image file -->

This will create the N Docker containers, bridges and tap interfaces, and will configure everything.

After creating everything, it will run the NS3 simulation and when its done it will destroy everything.

<!-- ### Run the NS3 simulation

Then go to the NS3 folder and run the NS3 simulation.

```bash
sudo ./waf --run "scratch/tap-vm --NumNodes=5 --TotalTime=100 --TapBaseName=emu"
```

The number of nodes needs to match of course. -->

### Next?

You can connect to the instances and start doing tasks like ping and watch the behavior. You can also go to the NS3 file and change the mobility patterns, size of the scenario, node density and any other parameter.

There are more updates coming for the future, if you have an idea or a contribution I'll be glad and happy to receive it or talk it over.
