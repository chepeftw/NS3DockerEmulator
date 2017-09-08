############################################################
# Build stage
############################################################

FROM golang:1.9 as builder

# Building Bchainlibs
WORKDIR /go/src/github.com/chepeftw
RUN git clone https://github.com/chepeftw/bchainlibs.git \
    && cd bchainlibs \
    && go get && go build

# Building Router
WORKDIR /go/src/github.com/chepeftw
RUN git clone https://github.com/chepeftw/BlockchainRouterLayer.git \
    && cd BlockchainRouterLayer \
    && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"' \
    && ldd /go/bin/BlockchainRouterLayer | grep -q "not a dynamic executable"

# Building Miner
WORKDIR /go/src/github.com/chepeftw
RUN git clone https://github.com/chepeftw/BlockchainMiner.git \
    && cd BlockchainMiner \
    && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"' \
    && ldd /go/bin/BlockchainMiner | grep -q "not a dynamic executable"

# Building Monitor
WORKDIR /go/src/github.com/chepeftw
RUN git clone https://github.com/chepeftw/BlockchainMANET.git \
    && cd BlockchainMANET \
    && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"' \
    && ldd /go/bin/BlockchainMANET | grep -q "not a dynamic executable"


############################################################
# Run stage
############################################################

FROM alpine

#RUN apk update && apk add bash

COPY wrapper.sh /wrapper.sh
RUN chmod +x /wrapper.sh
COPY --from=builder /go/bin/BlockchainRouterLayer /router
COPY --from=builder /go/bin/BlockchainMiner /miner
COPY --from=builder /go/bin/BlockchainMANET /blockchain

#CMD ["bash", "/wrapper.sh"]
CMD /wrapper.sh

# docker build -t hello-world-multi-stage .
# docker run hello-world-multi-stage
# docker image ls hello-world-multi-stage