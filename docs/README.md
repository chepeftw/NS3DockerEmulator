# NS3DockerEmulator
Network Emulator based on NS3 and Docker, with the help of NS3 docs, Docker docs and some ideas from some research papers.

![NS3 Docker Emulator Schema](https://github.com/chepeftw/NS3DockerEmulator/blob/master/NS3DockerEmulatorSchema.jpg)

## Preliminaries

This assumes that you have NS3.25 installed. It has been tested in Ubuntu 16.04 in an Amazon EC2 instance.

## NS3
Reference ... https://www.nsnam.org/docs/tutorial/html/getting-started.html

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
python $BAKE_HOME/bake.py configure -e ns-3.25
python $BAKE_HOME/bake.py download
python $BAKE_HOME/bake.py build
```

The last command might take a while.
Then you can go to the ns3 folder, should be something like:

```bash
cd source/ns-3.25
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

## Docker

Reference ... https://docs.docker.com/engine/installation/linux/ubuntulinux/

To install Docker in Ubuntu 16.04 the steps are:

```bash
sudo apt install apt-transport-https ca-certificates
sudo apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
echo "deb https://apt.dockerproject.org/repo ubuntu-xenial main" > /etc/apt/sources.list.d/docker.list
sudo apt update
sudo apt install docker-engine
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

You can "remove" the fact that you need to use "sudo" everytime you call docker, you can find this info in the reference link.

## Network Bridges

You need to install the following packages for easy bridge managment:

```bash
sudo apt install bridge-utils # for  brctl
sudo apt install uml-utilities # for tunctl
```

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

The final publication was my inspiration and it appears to be the first paper in consider the connection between NS3 and Docker, since first of all I know the authors but I couldn't find the source code, so I decided to write my own and publish it.

### Running it

To run it you need to do three steps.

### Copy the NS3 file.

You can go to the net folder and run the update.sh script:

```bash
bash update.sh tap-wifi-virtual-machine.cc
```

This will only copy the file "tap-wifi-virtual-machine.cc" to the NS3 scratch folder and rename it as "tap-vm.cc".
If your path doesn't match the path inside this script, please update it.

### Run the main.py

For now it will run a Beacon demo (which is in another repo of mine(https://github.com/chepeftw/Beacon)), which basically starts dockers, configures them and start a Beacon GoLang program in each docker that it is a beacon as the name suggest hehe, and it will broadcast a HelloWorld message through json and it will be logged into the host fs into ./var/log. You can run the main.py like this:

```bash
python ./main.py -n 5 -t 100 -o create
```

- -n is for the number of nodes
- -t is for the time of the simulation (this currently does not works (not sure why))
- -o operation, it ca be "create" or "destroy"
- --no-cache (optional) is for forcing the rebuild of the docker image file

This will create the N Docker containers, bridges and tap interfaces, and will configure everything.

Once you are done run the same command but with the "-o destroy" option.

### Run the NS3 simulation

Then go to the NS3 folder and run the NS3 simulation.

```bash
sudo ./waf --run "scratch/tap-vm --NumNodes=5 --TotalTime=100 --TapBaseName=emu"
```

The number of nodes needs to match of course.

### Next?

You can connect to the instances and start doing tasks like ping and watch the bahaviour. You can also go to the NS3 file and change the mobility patterns, size of the scenario, node density and any other parameter.

There are more updates coming for the future, if you have an idea or a contribution I'll be glad and happy to receive it or talk it over.
