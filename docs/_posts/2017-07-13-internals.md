---
layout: page
title: "Internals"
category: doc
date: 2017-07-13 17:26:57
order: 20
---

The project has 3 main folders, each one for a different purpose.

## docker

The folder "docker", as you might expect, refers to the Docker images.
For the simple case, the idea is that the container pulls a repo from github.
Then builds the code, runs it and that's it.

Thanks to the latest additions of Docker, we will use multi stage builds.
You can find plenty of docs regarding this like:
- [Use multi-stage builds by Docker](https://docs.docker.com/engine/userguide/eng-image/multistage-build/#use-multi-stage-builds)
- [Builder pattern vs. Multi-stage builds in Docker](https://blog.alexellis.io/mutli-stage-docker-builds/)
So since we are using a GoLang application, we will use the Go Container.
From this we will build the app, then copy it to a clean container and that is it.
Keep in mind that if your app needs more stuff, always consider using an Alpine container.

In this folder, there is a script I use for multiple services called "wrapper.sh", which I run from the Dockerfile.
This script is in charge of making sure the container has an IP before running the main code.
On the process it can do multiple stuff like starting other useful services or logging some useful information.
For our specific Beacon example, is not so useful since we just have one thing running, but for more complex applications comes really handy to keep track of multiple apps.
This idea I took it from the recommendation by Docker for [Multi Service Container](https://docs.docker.com/engine/admin/multi-service_container/).

## ns3

The main two files from this folder is "tap-wifi-virtual-machine.cc" and "update.sh".

#### tap-wifi-virtual-machine.cc

This file contains the main code for NS3 to model the network.
It contains a lot of NS3 concepts and uses way beyond this documentation, therefore I highly recommend for you to read the NS3 documentation.
Keep in mind that all the "NS_LOG_UNCOND()" calls are commented to optimize the running time for the NS3 simulation.

In a brief summary, this file intends to model a mobile ad-hoc network A.K.A. MANET.
The main areas are from line ~105 to line ~134, its variable initialization and parsing arguments.
The idea is to have as much arguments as possible to control everything from the outside.
Then from line ~145 to ~194 the initialization of the simulated network.
Basically the configuration of the physical layer.
Then from line ~196 to ~224 is the initial position allocation, it could be a grid or a random position.
Both options are present in the file but one is commented and can be changed according as necessary.
Then from line ~227 to ~265 is mobility configuration.
You can set a model of constant mobility or another one based on acceleration and timeout.
This can also be changed according as necessary.
Then from line ~267 to ~287 is an important file, where all nodes in the simulation are given a name and a tap name.
This will be used to link the nodes in the simulation with the corresponding linux bridges which finally will be connected to a container.
And from line ~290 is just simulation lines.


NS3 has the ability to create an animation file, it generates a animation.xml file which as far as I know for the time being it does not work correctly with tap bridges, therefore it does not work :(.


In general in this file you can change data mode, the model of packet loss, MAC protocol, checksum of packets, position, mobility, anything.
And that is it, this can changed as you wish, make sure you understand how NS3 works before doing so.

#### update.sh

This file is a simple script used by main.new.py to copy the main NS3 file, which is expected as a parameter into the corresponding NS3 scratch source folder.
This script is only one line but I did it like this so it could be totally apart from the main script, therefore if something else was needed it would be the job of this script, luckily it never happened such thing therefore the script remained as 1 line.
Maybe in future versions we could incorporate it into the main script.


## net

This folder is kind of interesting and it has the core connecting stuff between NS3 and Docker.

Before I continue, this information can be found in this references ... NONETHELESS ... when I built the emulator, this docs were not available so I had to scavenge the whole internet to find some useful information and then after testing, testing, debugging and errors I was able to complete this section:
* [Docker Reference Architecture: Designing Scalable, Portable Docker Container Networks](https://success.docker.com/Architecture/Docker_Reference_Architecture%3A_Designing_Scalable%2C_Portable_Docker_Container_Networks)
* [Docker Networking With Plugin](https://medium.com/@tugba/docker-networking-with-plugin-8fc3ce97444)

The files "setup.sh" and "destroy.sh" are from initial experiments, therefore they do not do anything from the main.new.py perspective.

#### singleSetup.sh

This file creates a bridge with the tool brctl (ethernet bridge administration) and a TAP interface with the tool tunctl (create and manage persistent TUN/TAP interfaces).
Then the tap interface is configured in promisc mode, added to the bridge and started.

The whole purpose of this script is to create the end of the NS3 node.
So the NS3 nodes will try to connect to the tap-$NAME device, since this is connected to the bridge, and a docker container will be connected to the same bridge via other mechanism ... that will make the docker container to be able to communicate via the NS3 simulation.

In this script we rely on NS3 to find the corresponding tap device based on the name, remember lines ~220 to ~242 from tap-wifi-virtual-machine.cc?
This lines tell NS3 which tap device to look, therefore it is important that both use the same names.

#### singleEndSetup.sh

This file ensures that the bridges-nf-* have a 0, therefore that they are turned off.
The bridges-nf-* can be:
* bridge-nf-call-arptables
* bridge-nf-call-ip6tables
* bridge-nf-call-iptables

And the fact that they have 0 it means that the traffic going through the bridges should not go to the arptables, ip6tables and iptables of the host.
This is done just one time after the singleSetup is ran for each node of NS3.

#### container.sh

This script receives the name and a index. The name is intended to be the same name for the docker and for the tap interface for NS3.
The name does not needs to be the same as the other, but it makes everything easier and more "debugabble".
So this script will create a folder under /var/run/netns which it refers to NETwork NameSpace (netns).
In there will create a simlink to the corresponding network of the corresponding container.
Then it will create two veth interfaces, one will go to the docker container and the other to the corresponding bridge.
The side A will connect to the bridge. The side B will connected to the container network namespace.
Then it will assign a random mac address to the side B, and a corresponding IP address based on the index.
The idea of the index is to avoid clashes between IP addresses.
It is worth mentioning that side B is configured as the eth0 of the container.
Therefore inside the container we will know that everything is ok when we are able to see the eth0 up.

#### singleDestroy.sh

Simply destroys the tap interface and the intermediary bridge. The internal and external bridges are destroyed by docker when the corresponding docker container is destroyed. The namespace created by this scripts is removed by main.new.py.

-----

## Known issues

One big problem that this simulator has been dragging is that due to this manual network configuration of the container, the container itself will never recognize its own IP. For example, if you run the simulator, and through a script you want to get the ipconfig of eth0, it will show you the ip and everything. But if you try the same from the host computer and do a docker inspect, the container will not have an IP, since it was started as a net=none and then manually assigned the interface. This could be solved either by doing some research engineering for the current network interfaces, to see how they work or to develop a custom plugin for network for this simulator.
This could provide some light into the plugin direction [Docker Networking With Plugin](https://medium.com/@tugba/docker-networking-with-plugin-8fc3ce97444).

-----

Second issue (*FIXED*), when dealing with a lot of containers, Docker ran into caching problems with the OS. I ran into problems with 40 to 60 nodes already, because the host was creating and deleting so much containers in so little time. For this refer to this [Running 1,000 Containers in Docker Swarm](https://blog.codeship.com/running-1000-containers-in-docker-swarm/)

*EDIT:* This was fixed as part of changing the flow of the emulation.

-----

So making a big summary.

<img src="http://d2r9k1wfjzxupg.cloudfront.net/NS3DockerEmulatorSchema-min.png" alt="NS3 Docker Emulator Schema" style="width:100%;padding-top:30px;padding-bottom:30px">

The singleSetup.sh takes care of all the tap interfaces and intermediary bridges. The singleEndSetup.sh it makes sure this bridges does not forward their traffic to the arptables and iptables of the host, we would not want this, is better if it goes directly from the container to NS3. Then we have container.sh which builds the internal and external bridge for every container, connects to the corresponding intermediary bridge and assigns MAC and IP address. And finally singleDestroy.sh will destroy everything.
