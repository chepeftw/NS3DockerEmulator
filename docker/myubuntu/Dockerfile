############################################################
# Dockerfile to build an Ubuntu instance with ping
# to be used with NS3.
# Based on Ubuntu
############################################################

FROM mybaseubuntu
MAINTAINER <Chepe Alvarez chepeftw@gmail.com>

RUN mkdir -p /data/golang/src/github.com/chepeftw /data/golang/bin
ENV GOPATH /data/golang
ENV GOBIN $GOPATH/bin
ENV PATH $PATH:$GOBIN

#ARG NNODES
#ARG ROOTN
#ARG FSMMODE
#ENV NNODES ${NNODES}
#ENV ROOTN ${ROOTN}
#ENV FSMMODE ${FSMMODE}

WORKDIR /data/golang/src/github.com/chepeftw
RUN git clone https://github.com/chepeftw/Beacon.git

WORKDIR Beacon
RUN go get && go build && go install

CMD Beacon