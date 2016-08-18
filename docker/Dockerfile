############################################################
# Dockerfile to build an Ubuntu instance with ping
# to be used with NS3.
# Based on Ubuntu
############################################################

FROM ubuntu
MAINTAINER <Chepe Alvarez chepeftw@gmail.com>

RUN apt-get update && apt-get install -y \
    inetutils-ping \
    net-tools \
    golang \
    git

RUN mkdir -p /data/golang/src/github.com/chepeftw /data/golang/bin
ENV GOPATH /data/golang
ENV GOBIN $GOPATH/bin
ENV PATH $PATH:$GOBIN

WORKDIR /data/golang/src/github.com/chepeftw
RUN git clone https://github.com/chepeftw/Beacon.git

WORKDIR Beacon
RUN go get && go build && go install

CMD Beacon