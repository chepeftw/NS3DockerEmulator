#!/usr/bin/python
__author__ = 'chepeftw'
import sys
import subprocess
import os
import time
import argparse
import datetime
import yaml

numberOfNodesStr='20'
emulationTimeStr='600'
scenarioSize='300'
operationStr='none'
noBuildCacheDocker=''
timeoutStr='800'
mode='single'
rootNode='10.12.0.1'
nodeSpeed='5'
nodePause='1'
simulationCount=0

baseContainerName0='mybaseubuntu'
baseContainerName1='myubuntu'

pidsDirectory = "./var/pid/"
logsDirectory = "./var/log/"


###############################
# n == number of nodes
# t == simulation time in seconds
###############################
parser = argparse.ArgumentParser()
parser.add_argument("operationStr", action="store",
                    help="The name of the operation to perform, options: full, create, destroy")
parser.add_argument("-n", "--number", action="store",
                    help="The number of nodes to simulate")
parser.add_argument("-t", "--time", action="store",
                    help="The time in seconds of NS3 simulation")
parser.add_argument("-to", "--timeout", action="store",
                    help="The timeout in seconds of NS3 simulation")
parser.add_argument("-s", "--size", action="store",
                    help="The size in meters of NS3 network simulation")
parser.add_argument("-ns", "--nodespeed", action="store",
                    help="The speed of the nodes expressed in m/s")
parser.add_argument("-np", "--nodepause", action="store",
                    help="The pause of the nodes expressed in s")
parser.add_argument("-c", "--count", action="store",
                    help="The count of simulations")
parser.add_argument('-v', '--version', action='version', version='%(prog)s 1.0')
args = parser.parse_args()

if args.number:
    numberOfNodesStr = args.number
if args.time:
    emulationTimeStr = args.time
if args.timeout:
    timeoutStr = args.timeout
if args.size:
    scenarioSize = args.size
if args.nodespeed:
    nodeSpeed = args.nodespeed
if args.nodepause:
    nodePause = args.nodepause
if args.count:
    simulationCount = int(args.count)

operationStr = args.operationStr

# Display input and output file name passed as the args
print("Number of nodes : %s" % (numberOfNodesStr))
print("Emulation time : %s" % (emulationTimeStr))
print("Operation : %s" % (operationStr))
print("Timeout : %s" % (timeoutStr))
print("Node Speed : %s" % (nodeSpeed))
print("Node Pause : %s" % (nodePause))
print("Simulation Count : %s" % (simulationCount))
print("Scenario Size : %s x %s" % (scenarioSize, scenarioSize))

################################################################################
########### error handling ()
################################################################################

def checkReturnCode( rCode, str ):
    if rCode == 0:
        print("Success: %s" %(str))
        return

    print("Error: %s" %(str))
    destroy() ## Adding this in case something goes wrong, at least we do some cleanup
    sys.exit(2)
    return

def checkReturnCodePassive( rCode, str ):
    if rCode == 0:
        print("Success: %s" %(str))
        return

    print("Error: %s" %(str))
    return



################################################################################
################################################################################
########### create ()
################################################################################
################################################################################
def create():
    print("Creating ...")

    #############################
    ## First we make sure we are running the latest version of our Ubuntu container
    ## This Ubuntu has tools like ping and ifconfig available.
    ## docker build -t myubuntu docker/.
    ## It's separated in two images so I can cache all that has to do with apt-get
    ## because that barely changes and the second image is for the code which is more likely
    ## to change really often. This recommendation of two images was done by the devs of docker.
    ## I got it from a thread where they were discussing a "selective no cache".

    ## All of this should run only one time, the first time to be exact
    ## There is no need of building this every time, just one, then I pass the arguments like TARGETSYNC
    ## by using a yml file and the ns3 script, literally never changes so one update should be more than enough.
    if simulationCount == 0:
        # r_code = subprocess.call("docker build -t %s docker/mybase/." % (baseContainerName0), shell=True)
        # checkReturnCode( r_code, "Building base container %s" %(baseContainerName0))
        #
        # arguments = noBuildCacheDocker + " "
        # arguments += "--build-arg FSMMODE=%s " % (mode)
        # arguments += "--build-arg NNODES=%s " % (numberOfNodesStr)
        # arguments += "--build-arg NTIMEOUT=%s " % (timeoutStr)
        # arguments += "--build-arg ROOTN=%s " % (rootNode)
        # arguments += "--build-arg TARGETSYNC=%s " % (str( int(time.time()) + (numberOfNodes*2.1) ))

        r_code = subprocess.call("docker build -t %s docker/myubuntu/." % (baseContainerName1), shell=True)
        checkReturnCode( r_code, "Building regular container %s" %(baseContainerName1))

        r_code = subprocess.call("cd ns3 && bash update.sh tap-wifi-virtual-machine.cc", shell=True)
        if r_code != 0:
            print("Error copying latest ns3 file")
        else:
            print("NS3 up to date!")
            print("Go to NS3 folder, probably cd $NS3_HOME")
            print("Run sudo ./waf --run \"scratch/tap-vm --NumNodes=%s --TotalTime=%s --TapBaseName=emu\"" % (numberOfNodesStr, emulationTimeStr))
            print("or run sudo ./waf --run \"scratch/tap-vm --NumNodes=%s --TotalTime=%s --TapBaseName=emu --SizeX=100 --SizeY=100\"" % (numberOfNodesStr, emulationTimeStr))

        r_code = subprocess.call("cd $NS3_HOME && sudo ./waf build", shell=True)
        if r_code == 0:
            print("NS3 BUILD WIN!")
        else:
            print("NS3 BUILD FAIL!")

        print('NS3 Build finished | Date now: %s' % datetime.datetime.now())


    #############################
    ## First and a half ... we generate the configuration yaml files.

    ## syncConfigTime (s) = seconds + ~seconds
    syncConfigTime = int(time.time()) + (numberOfNodes*2.2)

    config1 = {
        'target': syncConfigTime,
        'nodes': numberOfNodes,
        'timeout': int(timeoutStr),
        'rootnode': 1,
        'port': 10001
    }
    with open('conf1.yml', 'w') as yaml_file:
        yaml.dump(config1, yaml_file, default_flow_style=False)

    config2 = {
        'target': syncConfigTime+1,
        'nodes': numberOfNodes,
        'timeout': int(timeoutStr),
        'rootnode': 10,
        'port': 10002
    }
    with open('conf2.yml', 'w') as yaml_file:
        yaml.dump(config2, yaml_file, default_flow_style=False)



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

        volumes = "-v " + logHostPath + ":/var/log/golang "
        volumes += "-v conf1.yml:/treesip/conf1.yml "
        volumes += "-v conf2.yml:/treesip/conf2.yml "

        acc_status += subprocess.call("docker run --privileged -dit --net=none %s --name %s %s" % (volumes, nameList[x], baseContainerName1), shell=True)

    ## If something went wrong running the docker containers, we panic and exit
    checkReturnCode( acc_status, "Running docker containers")

    time.sleep(1)
    print('Finished running containers | Date now: %s' % datetime.datetime.now())

    #############################
    ## Third, we create the bridges and the tap interfaces for NS3
    ## Based on NS3 scripts ... https://www.nsnam.org/docs/release/3.25/doxygen/tap-wifi-virtual-machine_8cc.html
    ## But in the source you can find more examples in the same dir.
    acc_status=0
    for x in range(0, numberOfNodes):
        acc_status += subprocess.call("sudo bash net/singleSetup.sh %s" % (nameList[x]), shell=True)

    checkReturnCode( acc_status, "Creating bridge and tap interface")

    acc_status += subprocess.call("sudo bash net/singleEndSetup.sh", shell=True)
    checkReturnCode( acc_status, "Finalizing bridges and tap interfaces" )


    if not os.path.exists(pidsDirectory):
        os.makedirs(pidsDirectory)


    time.sleep(1)
    print('Finished creating bridges and taps | Date now: %s' % datetime.datetime.now())


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
            text_file.write(str(pid,'utf-8'))

        acc_status += subprocess.call("sudo bash net/container.sh %s %s" % (nameList[x], x), shell=True)

    ## If something went wrong creating the bridges and tap interfaces, we panic and exit
    ## checkReturnCode( acc_status, "Creating bridge side-int-X and side-ext-X" )
    ## Old behaviour, but I got situations where this failed, who knows why and basically stopped everything
    ## therefore I changed it to passive, if one fails, who cares but keep on going so the next simulations
    ## dont break
    checkReturnCodePassive( acc_status, "Creating bridge side-int-X and side-ext-X" )

    print("Done.")

    print('Finished setting up bridges | Date now: %s' % datetime.datetime.now())

    return

################################################################################
################################################################################
########### end create ()
################################################################################
################################################################################




################################################################################
################################################################################
########### ns3 ()
################################################################################
################################################################################
def ns3():
    print("NS3 ...")

    print('About to start NS3 RUN | Date now: %s' % datetime.datetime.now())

    r_code = subprocess.call("cd $NS3_HOME && sudo ./waf --run \"scratch/tap-vm --NumNodes=%s --TotalTime=%s --TapBaseName=emu --SizeX=%s --SizeY=%s --MobilitySpeed=%s --MobilityPause=%s\"" % (numberOfNodesStr, emulationTimeStr, scenarioSize, scenarioSize, nodeSpeed, nodePause), shell=True)

    print('Finished simulation | Date now: %s' % datetime.datetime.now())

    if r_code == 0:
        print("NS3 WIN!")
    else:
        print("NS3 FAIL!")

    # r_code = subprocess.call("bash /home/ubuntu/github/Dropbox-Uploader/dropbox_uploader.sh upload /home/ubuntu/workspace/source/ns-3.25/animation.xml animation" + str(int(time.time())) + ".xml", shell=True)
    # r_code = subprocess.call("bash /home/ubuntu/github/Dropbox-Uploader/dropbox_uploader.sh upload /home/ubuntu/workspace/source/ns-3.25/animation.xml animation.xml", shell=True)
    # checkReturnCode( r_code, "Error dropbox file 1")

    # r_code = subprocess.call("bash /home/ubuntu/github/Dropbox-Uploader/dropbox_uploader.sh upload /home/ubuntu/workspace/source/ns-3.25/routingtable-wireless.xml routingtable-wireless.xml", shell=True)
    # checkReturnCode( r_code, "Error dropbox file 1")

    return
################################################################################
################################################################################
########### end ns3 ()
################################################################################
################################################################################




################################################################################
################################################################################
########### destroy ()
################################################################################
################################################################################
def destroy():
    print("Destroying ...")

    print("DESTROYING ALL CONTAINERS")
    r_code = subprocess.call("docker stop $(docker ps -a -q) && docker rm $(docker ps -a -q)", shell=True)
    checkReturnCodePassive(r_code, "ERROR Destroying ALL containers")

    print("DESTROYING ALL CONTAINERS")
    r_code = subprocess.call("docker stop $(docker ps -a -q) && docker rm $(docker ps -a -q)", shell=True)
    checkReturnCodePassive(r_code, "ERROR Destroying ALL containers")

    time.sleep(1)

    for x in range(0, numberOfNodes):
        time.sleep(1)

        # r_code = subprocess.call("docker stop -t 0 %s" % (nameList[x]), shell=True)
        # checkReturnCodePassive( r_code, "Stopping docker container %s" % (nameList[x]))
        #
        # r_code = subprocess.call("docker rm %s" % (nameList[x]), shell=True)
        # checkReturnCodePassive( r_code, "Removing docker container %s" % (nameList[x]))

        r_code = subprocess.call("sudo bash net/singleDestroy.sh %s" % (nameList[x]), shell=True)
        checkReturnCodePassive( r_code, "Destroying bridge and tap interface %s" % (nameList[x]))

        if os.path.exists(pidsDirectory+nameList[x]):
            with open(pidsDirectory+nameList[x], "rt") as in_file:
                text = in_file.read()
                r_code = subprocess.call("sudo rm -rf /var/run/netns/%s" %(text.strip()), shell=True)
                checkReturnCodePassive( r_code, "Destroying docker bridges %s" % (nameList[x]))

        r_code = subprocess.call("sudo rm -rf %s" %(pidsDirectory+nameList[x]), shell=True)

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
elif 'full' == operationStr:
    create()
    ns3()
    destroy()
else:
    print("Nothing to be done ...")

