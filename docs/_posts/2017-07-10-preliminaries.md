---
layout: page
title: "Preliminaries"
category: doc
date: 2017-07-10 17:31:35
order: 10
---

<!-- # Preliminaries -->

You should install NS3, Docker and some other utilities for the emulator to work.
You can also check the "install.sh" script provided by @ptrsen (Thank you very much 👍).

## NS3
Reference ... [https://www.nsnam.org/docs/tutorial/html/getting-started.html](https://www.nsnam.org/docs/tutorial/html/getting-started.html)

The steps to install NS3 in Ubuntu 16.04 are:

This will install all the required packages.

```bash
sudo apt-get update
sudo apt-get install gcc g++ python python-dev mercurial bzr gdb valgrind gsl-bin libgsl0-dev libgsl0ldbl flex bison tcpdump sqlite sqlite3 libsqlite3-dev libxml2 libxml2-dev libgtk2.0-0 libgtk2.0-dev uncrustify doxygen graphviz imagemagick python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev python-pygccxml cmake autoconf libc6-dev libc6-dev-i386 g++-multilib texlive texlive-extra-utils texlive-latex-extra texlive-font-utils texlive-lang-portuguese dvipng git python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev ipython libboost-signals-dev libboost-filesystem-dev openmpi-bin openmpi-common openmpi-doc libopenmpi-dev qt4-dev-tools libqt4-dev unzip p7zip-full unrar-free
sudo apt-get install python-pip
pip install pygccxml
sudo pip install pygccxml —upgrade
```

Now we will create a workspace directory and clone bake (a tool from nsnam)

```bash
cd
mkdir workspace
cd workspace
hg clone http://code.nsnam.org/bake
```

Now add the bake path to the bashrc. Update your path accordingly.

```bash
echo "export BAKE_HOME=/home/ubuntu/workspace/bake" >> ~/.bashrc
echo "export PATH=$PATH:$BAKE_HOME:$BAKE_HOME/build/bin" >> ~/.bashrc
echo "export PYTHONPATH=$PYTHONPATH:$BAKE_HOME:$BAKE_HOME/build/lib" >> ~/.bashrc
source ~/.bashrc
```

Now we can start by checking the system, downloading ns3 and building it.

```bash
python $BAKE_HOME/bake.py check
python $BAKE_HOME/bake.py configure -e ns-3.26
python $BAKE_HOME/bake.py download
python $BAKE_HOME/bake.py build
```

The last command might take a while.
Then you can go to the ns3 folder, should be something like:

```bash
cd source/ns-3.26
```

To test that everything is working you can try:

```bash
./waf
```

Then you can copy some examples to the scratch folder to test and play with:

```bash
cp examples/tutorial/first.cc scratch/
./waf
./waf --run scratch/first
```
Please read the docs and check the examples. There are multiple examples in examples/tutorial or in src/<someFolder>/examples

## NS3 Optimized

If you dig into NS3 documentation and based on experience, you will notice that when you try to simulate more nodes it gets slower.
This is understandable and you can find some explanations to this behavior.
The bottomline is that NS3 tries to keep each node in sync, therefore if you have 100 nodes and if you have mobility for example, NS3 will try to keep up.
This of course can imply that it would decrease the time for the nodes in order to keep everything in sync.
It might sound kind of weird but it makes sense.

So let's look for an example for it to make more sense:
Let's say you have 100 nodes, with random mobility at some random speed, so every second, each node will move in different directions with different speeds.
If we add some packages to the equation, it needs to keep up by making sure that nodes are in range, the signal and many other stuff that I could be ignoring.
So if you set the simulation to 60 seconds, this could take 300 seconds to complete.
Because in an attempt to keep everything in sync it would decrease the internal time.
Therefore this will lead you to unrealistic simulations and of course bad results.
And this sometimes it can become noticeable even on small number of nodes as 50.
It depends on what you are trying to do.

In this simulator specific case, it gets even trickier because the Docker machines in the outside are operating at normal speed, meanwhile the NS3 simulation could be running in "slow motion".
Therefore we need to optimize as much as we can the NS3 behavior in order to get the desired results.

So this being said, and if you dig in into the compile parameters of NS3, there are some stuff you can to enhance this.
It requires to recompile NS3 and can take you some time but it is worth it.

Running NS3 optimized:
```bash
# first we clean all the compile options
./waf distclean
# we configure the compile options as optimized, disable examples, tests, python integration and static.
./waf -d optimized configure --disable-examples --disable-tests --disable-python --enable-static --no-task-lines
# we recompile (this might take some time)
./waf
```

## Docker

Reference ... [Get Docker CE for Ubuntu](https://docs.docker.com/engine/installation/linux/ubuntulinux/)

To install Docker in Ubuntu 16.04 the steps are:

```bash
sudo apt install apt-transport-https ca-certificates
sudo apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
echo "deb https://apt.dockerproject.org/repo ubuntu-xenial main" > /etc/apt/sources.list.d/docker.list
sudo apt update
sudo apt install docker-ce
```

To start the service you can run:

```bash
sudo service docker start
```

You can run a Hello world and an example like this:

```bash
sudo docker run hello-world
sudo docker run -it ubuntu bash
```

You can "remove" the fact that you need to use "sudo" every time you call docker, you can find this info in the reference link.

## Network Bridges

You need to install the following packages for easy bridge management:

```bash
sudo apt install bridge-utils # for  brctl
sudo apt install uml-utilities # for tunctl
```
