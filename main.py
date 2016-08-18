#!/usr/bin/python
__author__ = 'chepeftw'
import sys, getopt
import itertools
import subprocess
import os

from os import listdir
from os.path import isfile, join

numberOfNodesStr='200'
emulationTimeStr='600'
operationStr='none'
noBuildCacheDocker=''

baseContainerName='myubuntu'

pidsDirectory = "./var/pid/"
logsDirectory = "./var/log/"
 
###############################
# n == number of nodes
# t == simulation time in seconds
###############################
# Cache an error with try..except 
# Note: options is the string of option letters that the script wants to recognize, with 
# options that require an argument followed by a colon (':') i.e. -i fileName
#
try:
    myopts, args = getopt.getopt(sys.argv[1:],"hn:o:t:",["number=","operation=","time=","no-cache"])
except getopt.GetoptError as e:
    print (str(e))
    print("Usage: %s -o <create|destroy> -n numberOfNodes -t emulationTime" % sys.argv[0])
    sys.exit(2)
 
for opt, arg in myopts:
    if opt == '-h':
     print("Usage: %s -o <create|destroy> -n numberOfNodes -t emulationTime" % sys.argv[0])
     sys.exit()
    elif opt in ("-n", "--number"):
        numberOfNodesStr=arg
    elif opt in ("-t", "--time"):
        emulationTimeStr=arg
    elif opt in ("-o", "--operation"):
        operationStr=arg
    elif opt in ("--no-cache"):
        noBuildCacheDocker='--no-cache'

# Display input and output file name passed as the args
print ("Number of nodes : %s and emulation time : %s and operation : %s" % (numberOfNodesStr,emulationTimeStr,operationStr) )



################################################################################
################################################################################
########### create ()
################################################################################
################################################################################
def create():
    print "Creating ..."

    #############################
    ## First we make sure we are running the latest version of our Ubuntu container
    ## This Ubuntu has tools like ping and ifconfig available.
    ## docker build -t myubuntu docker/.
    r_code = subprocess.call("docker build %s -t %s docker/." % (noBuildCacheDocker, baseContainerName), shell=True)
    if r_code != 0:
        print "Error building base container %s" %(baseContainerName)
        sys.exit(2)
    else:
        print "Docker build successful"



    #############################
    ## Second, we run the numberOfNodes of containers.
    ## https://docs.docker.com/engine/reference/run/
    ## They have to run as privileged (don't remember why, need to clarify but I read it in stackoverflow)
    ## (Found it, it is to have access to all host devices, might be unsafe, will check later)
    ## By default, Docker containers are "unprivileged" and cannot, for example, run a Docker daemon inside a Docker container. 
    ## This is because by default a container is not allowed to access any devices, but a "privileged" container is given access to all devices.
    ## -dit ... -d run as daemon, -i Keep STDIN open even if not attached, -t Allocate a pseudo-tty
    ## --name the name of the container, using emuX
    ## Finally the name of our own Ubuntu image.
    if not os.path.exists(logsDirectory):
        os.makedirs(logsDirectory)

    dir_path = os.path.dirname(os.path.realpath(__file__))

    acc_status=0
    for x in range(0, numberOfNodes):
        if not os.path.exists(logsDirectory+nameList[x]):
            os.makedirs(logsDirectory+nameList[x])

        logHostPath = dir_path + logsDirectory[1:] + nameList[x] ## "." are not allowed in the -v of docker and it just work with absolute paths

        r_code = subprocess.call("docker run --privileged -dit --net=none -v %s:/var/log/golang --name %s %s" % (logHostPath, nameList[x], baseContainerName), shell=True)
        if r_code != 0:
            acc_status+=r_code
            print "Error run docker container %s" %(nameList[x])
        else:
            print "Container running %s" % (nameList[x])

    ## If something went wrong running the docker containers, we panic and exit
    if acc_status > 0:
        sys.exit(2)



    #############################
    ## Third, we create the bridges and the tap interfaces for NS3
    ## Based on NS3 scripts ... https://www.nsnam.org/docs/release/3.25/doxygen/tap-wifi-virtual-machine_8cc.html
    ## But in the source you can find more examples in the same dir.
    acc_status=0
    for x in range(0, numberOfNodes):
        r_code = subprocess.call("sudo bash net/singleSetup.sh %s" % (nameList[x]), shell=True)
        if r_code != 0:
            acc_status+=r_code
            print "Error creating bridge br-%s" %(nameList[x])
        else:
            print "Created bridge br-%s and tap interface tap-%s" % (nameList[x],nameList[x])

    r_code = subprocess.call("sudo bash net/singleEndSetup.sh", shell=True)
    if r_code != 0:
        acc_status+=r_code
        print "Error finalizing bridges and tap interfaces"
    else:
        print "Finishing creation of bridges and tap interfaces"

    ## If something went wrong creating the bridges and tap interfaces, we panic and exit
    if acc_status > 0:
        sys.exit(2)


    if not os.path.exists(pidsDirectory):
        os.makedirs(pidsDirectory)

    #############################
    ## Fourth, we create the bridges for the docker containers
    ## https://docs.docker.com/v1.7/articles/networking/
    acc_status=0
    for x in range(0, numberOfNodes):

        cmd = ['docker', 'inspect', '--format', "'{{ .State.Pid }}'", nameList[x]]
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = p.communicate()
        pid = out[1:-2].strip()

        with open(pidsDirectory+nameList[x], "w") as text_file:
            text_file.write("%s"%(pid))

        r_code = subprocess.call("sudo bash net/container.sh %s %s" % (nameList[x], x), shell=True)
        if r_code != 0:
            acc_status+=r_code
            print "Error creating bridge side-X-%s" %(nameList[x])
        else:
            print "Created bridge side-int-%s and side-ext-%s" % (nameList[x],nameList[x])

    ## If something went wrong creating the bridges and tap interfaces, we panic and exit
    if acc_status > 0:
        sys.exit(2)

    print "Done."

    return

################################################################################
################################################################################
########### end create ()
################################################################################
################################################################################



################################################################################
################################################################################
########### destroy ()
################################################################################
################################################################################
def destroy():
    print "Destroying ..."

    for x in range(0, numberOfNodes):
        r_code = subprocess.call("docker stop %s" % (nameList[x]), shell=True)
        if r_code != 0:
            print "Error stop docker container %s" %(nameList[x])
        else:
            print "Stopped %s" % (nameList[x])

        r_code = subprocess.call("docker rm %s" % (nameList[x]), shell=True)
        if r_code != 0:
            print "Error removing docker container %s" %(nameList[x])
        else:
            print "Removed %s" % (nameList[x])

        r_code = subprocess.call("sudo bash net/singleDestroy.sh %s" % (nameList[x]), shell=True)
        if r_code != 0:
            acc_status+=r_code
            print "Error destroying bridge br-%s" %(nameList[x])
        else:
            print "Destroyed bridge br-%s and tap interface tap-%s" % (nameList[x],nameList[x])

        with open(pidsDirectory+nameList[x], "rt") as in_file:
            text = in_file.read()
            r_code = subprocess.call("sudo rm -rf /var/run/netns/%s" %(text.strip()), shell=True)
            if r_code != 0:
                acc_status+=r_code
                print "Error destroying bridge side-X-%s" %(nameList[x])
            else:
                print "Destroyed bridge side-int-%s and side-ext-%s" % (nameList[x], nameList[x])

        r_code = subprocess.call("sudo rm -rf %s" %(pidsDirectory+nameList[x]), shell=True)


    ## This is SO SO UNSAFE, but I'll tweak it later to remove based on stored PID in the fs.
    # r_code = subprocess.call("sudo rm -rf /var/run/netns", shell=True) 
    # if r_code != 0:
    #     acc_status+=r_code
    #     print "Error destroying bridge container-X-%s" %(nameList[x])
    # else:
    #     print "Destroyed bridge side-int-X and side-ext-X"

    return
################################################################################
################################################################################
########### end destroy ()
################################################################################
################################################################################



numberOfNodes=int(numberOfNodesStr)
emulationTime=int(emulationTimeStr)

nameList=[]
baseName="emu"

for x in range(0, numberOfNodes):
    nameList.append( baseName + str(x+1) )

if 'create' == operationStr:
    create()
elif 'destroy' == operationStr:
    destroy()
else:
    print "Nothing to be done ..."
