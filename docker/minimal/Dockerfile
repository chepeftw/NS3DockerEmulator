############################################################
# Build stage
############################################################

FROM golang:1.9 as builder

# Building Bchainlibs
WORKDIR /go/src/github.com/chepeftw
RUN git clone git@github.com:chepeftw/BchainLibs.git
WORKDIR /go/src/github.com/chepeftw/BchainLibs
RUN go get && go build

# Building Router
WORKDIR /go/src/github.com/chepeftw
RUN git clone git@github.com:chepeftw/BlockchainRouterLayer.git
WORKDIR /go/src/github.com/chepeftw/BlockchainRouterLayer
RUN go get \
    && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"' \
    && ldd /go/bin/Router | grep -q "not a dynamic executable"

# Building Miner
WORKDIR /go/src/github.com/chepeftw
RUN git clone git@github.com:chepeftw/BlockchainMiner.git
WORKDIR /go/src/github.com/chepeftw/BlockchainMiner
RUN go get \
    && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"' \
    && ldd /go/bin/Miner | grep -q "not a dynamic executable"

# Building Monitor
WORKDIR /go/src/github.com/chepeftw
RUN git clone git@github.com:chepeftw/BlockchainMANET.git
WORKDIR /go/src/github.com/chepeftw/BlockchainMANET
RUN go get \
    && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"' \
    && ldd /go/bin/Blockchain | grep -q "not a dynamic executable"


############################################################
# Run stage
############################################################

FROM alpine

#RUN apk update && apk add bash

COPY wrapper.sh /wrapper.sh
RUN chmod +x /wrapper.sh
COPY --from=builder /go/bin/Router /router
COPY --from=builder /go/bin/Miner /miner
COPY --from=builder /go/bin/Blockchain /blockchain

#CMD ["bash", "/wrapper.sh"]
CMD /wrapper.sh

# docker build -t hello-world-multi-stage .
# docker run hello-world-multi-stage
# docker image ls hello-world-multi-stage