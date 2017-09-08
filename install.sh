#!/bin/bash

# This script install all the required packages for The NS3DockerEmulator  https://github.com/chepeftw/NS3DockerEmulator 
# Ns3.26 and Docker 17.06
# To running , open Terminal and execute  
# source install.sh

echo -e "\n\n Updating enviroment... \n" 
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade

echo -e "\n\n Installing git ... \n" 
sudo apt-get install git

echo -e "\n\n Installing Ns3 required packages ... \n" 
sudo apt-get install gcc g++ python python-dev mercurial bzr gdb valgrind gsl-bin libgsl2 libgsl2:i386 flex bison tcpdump sqlite sqlite3 libsqlite3-dev libxml2 libxml2-dev libgtk2.0-0 libgtk2.0-dev uncrustify doxygen graphviz imagemagick python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev python-pygccxml cmake autoconf libc6-dev libc6-dev-i386 g++-multilib texlive texlive-extra-utils texlive-latex-extra texlive-font-utils texlive-lang-portuguese dvipng python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev ipython libboost-signals-dev libboost-filesystem-dev openmpi-bin openmpi-common openmpi-doc libopenmpi-dev qt4-dev-tools libqt4-dev unzip p7zip-full unrar-free cvs 
sudo apt-get install python-pip
pip install pygccxml
pip install --upgrade pip
sudo -H pip install pygccxml --upgrade

echo -e "\n\n Setting Ns3 workspace ... \n" 
mkdir Ns3
cd Ns3
hg clone http://code.nsnam.org/bake
cd bake

 
echo "export BAKE_HOME=$PWD" >> ~/.bashrc
echo "export PATH=$PATH:$BAKE_HOME:$BAKE_HOME/build/bin" >> ~/.bashrc
echo "export PYTHONPATH=$PYTHONPATH:$BAKE_HOME:$BAKE_HOME/build/lib" >> ~/.bashrc
source ~/.bashrc

python $BAKE_HOME/bake.py check
python $BAKE_HOME/bake.py configure -e ns-3.26
python $BAKE_HOME/bake.py download
python $BAKE_HOME/bake.py build

echo -e "\n\n Verifying Ns3  ... \n" 
cd source/ns-3.26
./waf

echo -e "\n\n Recompoling NS3 in optimized mode  ... \n"
./waf distclean
./waf -d optimized configure --disable-examples --disable-tests --disable-python --enable-static --no-task-lines
./waf

echo -e "\n\n Running first Ns3 example  ... \n"
cp examples/tutorial/first.cc scratch/
./waf
./waf --run scratch/first
cd ~

echo -e "\n\n Installing Docker required packages  ... \n"

sudo apt-get install apt-transport-https ca-certificates curl software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo apt-key fingerprint 0EBFCD88
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"

service lxcfs stop
sudo apt-get remove lxc-common lxcfs lxd lxd-client

sudo apt-get update
sudo apt-get install docker-ce

echo -e "\n\n  Verifying  Docker  ... \n"
sudo docker run hello-world

echo -e "\n\n Installing Network Bridges  ... \n"

sudo apt install bridge-utils 
sudo apt install uml-utilities 

git clone https://github.com/chepeftw/NS3DockerEmulator.git
cd NS3DockerEmulator
