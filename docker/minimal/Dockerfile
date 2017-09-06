############################################################
# Build stage
############################################################

FROM golang:1.9 as builder

# Building Beacon simple app
WORKDIR /go/src/github.com/chepeftw
RUN git clone https://github.com/chepeftw/Beacon.git
WORKDIR /go/src/github.com/chepeftw/Beacon
RUN go get && CGO_ENABLED=0 go install -a -tags netgo -ldflags '-extldflags "-static"'

COPY wrapper.sh ./

RUN ldd /go/bin/beacon | grep -q "not a dynamic executable"


############################################################
# Run stage
############################################################

FROM scratch

COPY --from=builder /go/bin/github.com/chepeftw/beacon /beacon
CMD ["/wrapper.sh"]

# docker build -t hello-world-multi-stage .
# docker run hello-world-multi-stage
# docker image ls hello-world-multi-stage