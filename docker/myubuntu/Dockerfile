############################################################
# Dockerfile to build an Ubuntu instance with ping
# to be used with NS3.
# Based on Ubuntu
############################################################

FROM mybaseubuntu
MAINTAINER <Chepe Alvarez chepeftw@gmail.com>

WORKDIR /data/golang/src/github.com/chepeftw
RUN git clone https://github.com/chepeftw/Treesiplibs.git \
        && git clone https://github.com/chepeftw/Gossip.git \
        && git clone https://github.com/chepeftw/Treesip.git \
        && cd /data/golang/src/github.com/chepeftw/Treesiplibs \
        && go get && go build && go install \
        && cd /data/golang/src/github.com/chepeftw/Gossip \
        && go get && go build && go install \
        && cd /data/golang/src/github.com/chepeftw/Treesip \
        && go get && go build && go install

ADD wrapper.sh /data/wrapper.sh

CMD bash /data/wrapper.sh
