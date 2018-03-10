############################################################
# Build stage
############################################################

FROM golang:1.9 as builder

# Building Beacon simple app
WORKDIR /go/src/github.com/chepeftw
RUN git clone https://github.com/chepeftw/raft.git
WORKDIR /go/src/github.com/chepeftw/raft
RUN go get \
    && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"' \
    && ldd /go/bin/raft | grep -q "not a dynamic executable"


############################################################
# Run stage
############################################################

FROM alpine

COPY --from=builder /go/src/github.com/chepeftw/raft/wrapper.sh /wrapper.sh
COPY --from=builder /go/bin/raft /raft

RUN chmod +x /wrapper.sh

CMD /wrapper.sh

# docker build -t hello-world-multi-stage .
# docker run hello-world-multi-stage
# docker image ls hello-world-multi-stage