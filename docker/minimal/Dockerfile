############################################################
# Build stage
############################################################

FROM golang:1.9 as builder

# Building Beacon simple app
WORKDIR /go/src/github.com/chepeftw
RUN git clone https://github.com/chepeftw/raft.git \
    && git clone https://github.com/chepeftw/bchainlibs.git \
    && git clone https://github.com/chepeftw/blockchainrouterlayer.git \
    && git clone https://github.com/chepeftw/blockchainminer.git \
    && git clone https://github.com/chepeftw/blockchainmanet.git

WORKDIR /go/src/github.com/chepeftw/raft
RUN go get \
    && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"' \
    && ldd /go/bin/raft | grep -q "not a dynamic executable"

WORKDIR /go/src/github.com/chepeftw/blockchainrouterlayer
RUN go get \
    && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"' \
    && ldd /go/bin/blockchainrouterlayer | grep -q "not a dynamic executable"

WORKDIR /go/src/github.com/chepeftw/blockchainminer
RUN go get \
    && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"' \
    && ldd /go/bin/blockchainminer | grep -q "not a dynamic executable"

WORKDIR /go/src/github.com/chepeftw/blockchainmanet
RUN go get \
    && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"' \
    && ldd /go/bin/blockchainmanet | grep -q "not a dynamic executable"


############################################################
# Run stage
############################################################

FROM alpine

COPY wrapper.sh /wrapper.sh
COPY --from=builder /go/bin/raft /raft
COPY --from=builder /go/bin/blockchainrouterlayer /router
COPY --from=builder /go/bin/blockchainminer /miner
COPY --from=builder /go/bin/blockchainmanet /blockchain

RUN chmod +x /wrapper.sh

CMD /wrapper.sh

# docker build -t hello-world-multi-stage .
# docker run hello-world-multi-stage
# docker image ls hello-world-multi-stage