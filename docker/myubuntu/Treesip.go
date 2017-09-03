package main
 
import (
    "os"
    "fmt"
    "net"
    "time"
    "strings"
    "strconv"
    "math/rand"
    "encoding/json"

    "github.com/op/go-logging"
)


// +++++++++++++++++++++++++++
// +++++++++ Go-Logging Conf
// +++++++++++++++++++++++++++
var log = logging.MustGetLogger("treesip")

var format = logging.MustStringFormatter(
    "%{level:.4s}=> %{time:0102 15:04:05.999} %{shortfile} %{message}",
)


// +++++++++ Constants
const (
    Port              = ":10001"
    Protocol          = "udp"
    BroadcastAddr     = "255.255.255.255"
    StartType         = "start"
    TimeoutType       = "timeout"
    QueryType         = "query"
    AggregateType     = "aggregate"
    AggregateFwdType  = "aggregateForward"
    AggregateRteType  = "aggregateRoute"
)

const (
    INITIAL = iota
    Q1
    Q2
    A1
    A2
    A3
)

// +++++++++ Global vars
var state = INITIAL
var myIP net.IP = net.ParseIP("127.0.0.1")
var parentIP net.IP = net.ParseIP("127.0.0.1")
var timeout float32 = 0

var stateQuery Packet = Packet{}
var queryACKlist []net.IP = []net.IP{}
var timer *time.Timer

var accumulator float32 = 0
var observations int = 0
var rootNode bool = false
var startTime int64 = 0

var globalNumberNodes int = 0
var globalCounter int = 0
var electionNode string = ""
var runMode string = ""

var s1 = rand.NewSource(time.Now().UnixNano())
var r1 = rand.New(s1)

// +++++++++ Channels
var buffer = make(chan string)
var done = make(chan bool)

// +++++++++ Packet structure
type Packet struct {
    Type      string        `json:"type"`
    Parent    net.IP        `json:"parent"`
    Source    net.IP        `json:"source"`
    Timeout   float32       `json:"timeout"`
    Query     *Query        `json:"query,omitempty"`
    Aggregate *Aggregate    `json:"aggregate,omitempty"`
}

type Query struct {
    Function  string    `json:"function,omitempty"`
    RelaySet  []*net.IP `json:"relaySet,omitempty"`
}

type Aggregate struct {
    Outcome      float32 `json:"outcome,omitempty"`
    Destination  net.IP `json:"destination,omitempty"`
    Observations int    `json:"observations,omitempty"`
}

 
// A Simple function to verify error
func CheckError(err error) {
    if err  != nil {
        log.Error("Error: ", err)
    }
}

// Getting my own IP, first we get all interfaces, then we iterate
// discard the loopback and get the IPv4 address, which should be the eth0
func SelfIP() net.IP {
    addrs, err := net.InterfaceAddrs()
    if err != nil {
        panic(err)
    }

    for _, a := range addrs {
        if ipnet, ok := a.(*net.IPNet); ok && !ipnet.IP.IsLoopback() {
            if ipnet.IP.To4() != nil {
                return ipnet.IP
            }
        }
    }

    return net.ParseIP("127.0.0.1")
}

func SendAggregate(destination net.IP, outcome float32, observations int) {
    aggregate := Aggregate{
            Destination: destination,
            Outcome: outcome,
            Observations: observations,
        }

    payload := Packet{
        Type: AggregateType,
        Parent: parentIP,
        Source: myIP,
        Timeout: timeout,
        Aggregate: &aggregate,
    }

    SendMessage(payload)
}

func SendQuery(receivedPacket Packet) {
    relaySet := CalculateRelaySet(receivedPacket.Source, receivedPacket.Query.RelaySet)

    if receivedPacket.Type == StartType {
        relaySet = []*net.IP{}
    }

    query := Query{
            Function: receivedPacket.Query.Function,
            RelaySet: relaySet,
        }

    payload := Packet{
        Type: QueryType,
        Parent: parentIP,
        Source: myIP,
        Timeout: timeout,
        Query: &query,
    }

    SendMessage(payload)
}

func SendMessage(payload Packet) {
    SendMessageExt(payload, BroadcastAddr+Port)
}

func SendMessageExt(payload Packet, target string) {
    ServerAddr,err := net.ResolveUDPAddr(Protocol, target)
    CheckError(err)
    LocalAddr, err := net.ResolveUDPAddr(Protocol, myIP.String()+":0")
    CheckError(err)
    Conn, err := net.DialUDP(Protocol, LocalAddr, ServerAddr)
    CheckError(err)
    defer Conn.Close()

    log.Debug( myIP.String() + " SENDING_MESSAGE=1" )

    js, err := json.Marshal(payload)
    CheckError(err)

    if Conn != nil {
        msg := js
        buf := []byte(msg)
        log.Debug( myIP.String() + " MESSAGE_SIZE=" + strconv.Itoa(len(buf)) )
        _,err = Conn.Write(buf)
        CheckError(err)
    }
}

// Function to calculate the RelaySet
// the idea is to always have 3 elements. Seen as a tree, the closer 3 parents.
func CalculateRelaySet( newItem net.IP, receivedRelaySet []*net.IP ) []*net.IP {
    slice := []*net.IP{&newItem}

    if len(receivedRelaySet) < 3 {
        return append(slice, receivedRelaySet...)
    }

    return append(slice, receivedRelaySet[:len(receivedRelaySet)-1]...)
}

// Timeout functions to start and stop the timer
func StartTimer(d float32) {
    timer = time.NewTimer(time.Millisecond * time.Duration(float32(r1.Intn(1200))+d))

    go func() {
        <- timer.C
        buffer <- "{\"type\":\"" + TimeoutType + "\"}"
        log.Info("Timer expired")
    }()
}

func StopTimer() {
    if &timer != nil {
        timer.Stop()
    }
}

func CalculateOwnValue() float32 {
    return 50.0
}

func CalculateAggregateValue( v float32, o int ) {
    accumulator += v
    observations += o
}

func RemoveFromList(del net.IP) bool {
    index := -1
    for i, b := range queryACKlist {
        if b.Equal(del) {
            index = i
            break
        }
    }

    if index >= 0 {
        queryACKlist = append(queryACKlist[:index], queryACKlist[index+1:]...)
        return true
    }

    return false
}

func CleanupTheHouse() {
    parentIP = net.ParseIP("127.0.0.1")
    timeout = 0

    stateQuery = Packet{}
    queryACKlist = []net.IP{}
    StopTimer()

    accumulator = 0
    observations = 0
    rootNode = false
    startTime = 0

    go selectLeaderOfTheManet()
}

func listOfIPsToString() string {
    list := []string{}

    for _, b := range queryACKlist {
        list = append(list, b.String())
    }

    return strings.Join(list,", ")
}

// Function that handles the buffer channel
func attendBufferChannel() {
    for {
        j, more := <-buffer
        if more {
            // s := strings.Split(j, "|")
            // _, jsonStr := s[0], s[1]

            // First we take the json, unmarshal it to an object
            packet := Packet{}
            json.Unmarshal([]byte(j), &packet)

            // Now we start! FSM TIME!
            switch state {
            case INITIAL:
                // RCV start() -> SND Query
                if packet.Type == StartType {
                    startTime = time.Now().UnixNano()
                    log.Info( myIP.String() + " => State: INITIAL, start() -> SND Query")
                    state = Q1
                    stateQuery = packet
                    parentIP = nil
                    timeout = packet.Timeout
                    SendQuery(packet)
                    StartTimer(timeout)
                } else if packet.Type == QueryType { // RCV Query -> SND Query
                    log.Info(myIP.String() + " => State: INITIAL, RCV Query -> SND Query")
                    state = Q1
                    stateQuery = packet
                    parentIP = packet.Source
                    timeout = packet.Timeout
                    SendQuery(packet)
                    StartTimer(timeout)
                }
            break
            case Q1: 
                // RCV QueryACK -> acc(ACK_IP)
                if packet.Type == QueryType && packet.Parent.Equal(myIP) && !packet.Source.Equal(myIP) {
                    log.Info( myIP.String() + " => State: Q1, RCV QueryACK -> acc( " + packet.Source.String() + " )-> " + strconv.Itoa( len( queryACKlist ) ) )
                    state = Q2
                    queryACKlist = append(queryACKlist, packet.Source)
                    // log.Debug( myIP.String() + " => len(queryACKlist) = " + strconv.Itoa( len( queryACKlist ) ) )
                    // log.Debug( "queryACKlist = " + listOfIPsToString() )
                    StopTimer()
                } else if packet.Type == TimeoutType { // timeout() -> SND Aggregate
                    log.Info( myIP.String() + " => State: Q1, timeout() -> SND Aggregate")
                    state = A1
                    SendAggregate(parentIP, CalculateOwnValue(), 1)
                    StartTimer(timeout)
                    // Just one outcome and 1 observation because it should be the end of a branch
                    // Otherwise its a chapter of Stranger Things or Lost
                }

                // edgeNode() -> SND Aggregate
                // ToDo - not for now since we need GPS info or another mechanism
            break
            case Q2:
                // RCV QueryACK -> acc(ACK_IP)
                if packet.Type == QueryType && packet.Parent.Equal(myIP) && !packet.Source.Equal(myIP) {
                    log.Info( myIP.String() + " => State: Q2, RCV QueryACK -> acc( " + packet.Source.String() + " )-> " + strconv.Itoa( len( queryACKlist ) ))
                    state = Q2 // loop to stay in Q2
                    queryACKlist = append(queryACKlist, packet.Source)
                    // log.Debug( myIP.String() + " => len(queryACKlist) = " + strconv.Itoa( len( queryACKlist ) ) )
                    // log.Debug( "queryACKlist = " + listOfIPsToString() )
                } else if packet.Type == AggregateType && packet.Aggregate.Destination.Equal(myIP) { // RCV Aggregate -> SND Aggregate 
                    // not always but yes
                    // I check that the parent it is itself, that means that he already stored this guy
                    // in the queryACKList
                    log.Info( myIP.String() + " => State: Q2, RCV Aggregate -> SND Aggregate remove " + packet.Source.String() + " -> " + strconv.Itoa( len( queryACKlist ) ))
                    state = A1
                    StopTimer()
                    CalculateAggregateValue(packet.Aggregate.Outcome, packet.Aggregate.Observations)
                    RemoveFromList(packet.Source)
                    // log.Debug( myIP.String() + " => len(queryACKlist) = " + strconv.Itoa( len( queryACKlist ) ) )
                    // log.Debug( "queryACKlist = " + listOfIPsToString() )
                    if len(queryACKlist) == 0 {
                        SendAggregate(parentIP, accumulator + CalculateOwnValue(), observations + 1)
                        StartTimer(timeout)
                    }
                }
            break
            case A1:
                // RCV Aggregate -> SND Aggregate // not always but yes
                // I check that the parent it is itself, that means that he already stored this guy
                // in the queryACKList
                if packet.Type == AggregateType && packet.Aggregate.Destination.Equal(myIP) {
                    log.Info( myIP.String() + " => State: A1, RCV Aggregate & loop() -> SND Aggregate " + packet.Source.String() + " -> " + strconv.Itoa(len(queryACKlist)))
                    state = A1
                    StopTimer()
                    CalculateAggregateValue(packet.Aggregate.Outcome, packet.Aggregate.Observations)
                    RemoveFromList(packet.Source)
                    // log.Debug( myIP.String() + " => len(queryACKlist) = " + strconv.Itoa(len(queryACKlist)))
                    // log.Debug( "queryACKlist = " + listOfIPsToString() )
                    if len(queryACKlist) == 0 && !rootNode {
                        log.Info("if len(queryACKlist) == 0 && !rootNode")
                        SendAggregate(parentIP, accumulator + CalculateOwnValue(), observations + 1)
                        StartTimer(timeout)
                    } else if len(queryACKlist) == 0 && rootNode { // WE ARE DONE!!!!
                        log.Info("else if len(queryACKlist) == 0 && !rootNode")
                        SendAggregate(myIP, accumulator + CalculateOwnValue(), observations + 1) // Just for ACK
                        log.Info( 
                            myIP.String() + 
                            " => State: A1, **DONE**, Result: " + 
                            strconv.FormatFloat(float64(accumulator + CalculateOwnValue()), 'f', 6, 64) + 
                            " and Observations:" + 
                            strconv.Itoa(observations + 1))
                        log.Info( myIP.String() + " CONVERGENCE_TIME=" + strconv.FormatInt( (time.Now().UnixNano() - startTime) / int64(time.Millisecond), 10 ))
                        state = INITIAL
                        CleanupTheHouse()
                    } else {
                        StartTimer(timeout)
                    }
                } else if packet.Type == AggregateType && packet.Source.Equal(parentIP) { // RCV AggregateACK -> done()
                    log.Info( myIP.String() + " => State: A1, RCV Aggregate -> done()")
                    state = INITIAL
                    CleanupTheHouse()
                } else if packet.Type == TimeoutType { // timeout -> SND AggregateRoute // not today
                    // state = A2 // it should do this, but not today
                    log.Info( myIP.String() + " => State: A1, timeout() -> SND AggregateRoute")
                    state = INITIAL
                    CleanupTheHouse()
                    if rootNode { // Just to show something
                        log.Info( 
                            myIP.String() + 
                            " => State: A1, **DONE**, Result: " + 
                            strconv.FormatFloat(float64(accumulator + CalculateOwnValue()), 'f', 6, 64) + 
                            " and Observations:" + 
                            strconv.Itoa(observations + 1))
                        log.Info( myIP.String() + " CONVERGENCE_TIME=" + strconv.FormatInt( (time.Now().UnixNano() - startTime) / int64(time.Millisecond), 10 ))
                    }
                }
            break
            case A2:
                // Not happening bro!
            break
            default:
                // Welcome to Stranger Things ... THIS REALLY SHOULD NOT HAPPEN
            break
            }

        } else {
            log.Debug("closing channel")
            done <- true
            return
        }
    }
}

func selectLeaderOfTheManet() {
    // This should be a super elegant way of choosing the leader of the MANET
    // The root, the source, the neo, the parent of the MANET, you name it
    time.Sleep(time.Second * 5)
    neo := electionNode

    if runMode != "single" {
        if globalNumberNodes != 0 {
            if globalCounter == globalNumberNodes {
                return
            }

            s3 := int(globalCounter/250)
            s4 := int(globalCounter%250)+1

            neo = "10.12." + strconv.Itoa(s3) + "." + strconv.Itoa(s4)
            globalCounter = globalCounter + 1
        }
    } else {
        if globalCounter > 0 {
            return
        }

        globalCounter = globalCounter + 1
    }


    if myIP.String() == neo {
        rootNode = true

        query := Query{
                Function: "avg",
                RelaySet: []*net.IP{},
            }

        payload := Packet{
            Type: StartType,
            Source: myIP,
            Timeout: 1000,
            Query: &query,
        }

        log.Info("The leader has been choosen!!! All hail the new KING!!! " + neo)
        // time.Sleep(time.Second * 5)

        SendMessageExt(payload, myIP.String()+Port)
    }
}
 
func main() {

    nnodes := os.Getenv("NNODES")
    rootn := os.Getenv("ROOTN")
    fsmmode := os.Getenv("FSMMODE")
    if nnodes != "" {
        globalNumberNodes, _ = strconv.Atoi( nnodes )
    }

    if rootn != "" {
        electionNode = rootn
    }

    if fsmmode != "" {
        runMode = fsmmode
    }


    // +++++++++++++++++++++++++++++
    // ++++++++ Logger conf
    var logPath = "/var/log/golang/"
    if _, err := os.Stat(logPath); os.IsNotExist(err) {
        os.MkdirAll(logPath, 0777)
    }

    var logFile = logPath + "treesip.log"
    f, err := os.OpenFile(logFile, os.O_APPEND | os.O_CREATE | os.O_RDWR, 0666)
    if err != nil {
        fmt.Printf("error opening file: %v", err)
    }

    // don't forget to close it
    defer f.Close()

    backend := logging.NewLogBackend(f, "", 0)
    backendFormatter := logging.NewBackendFormatter(backend, format)

    logging.SetBackend(backendFormatter)
    // ++++++++ END Logger conf
    // +++++++++++++++++++++++++++++

    log.Info("Starting Treesip process, waiting one minute to get my own IP...")

    // It gives one minute time for the network to get configured before it gets its own IP.
    time.Sleep(time.Second * time.Duration(globalNumberNodes))
    myIP = SelfIP();

    log.Info("Good to go, my ip is " + myIP.String())


    // Lets prepare a address at any address at port 10001
    ServerAddr,err := net.ResolveUDPAddr(Protocol, Port)
    CheckError(err)
 
    // Now listen at selected port
    ServerConn, err := net.ListenUDP(Protocol, ServerAddr)
    CheckError(err)
    defer ServerConn.Close()

    go attendBufferChannel()
    go selectLeaderOfTheManet()
 
    buf := make([]byte, 1024)
 
    for {
        n,_,err := ServerConn.ReadFromUDP(buf)
        // buffer <- addr.String() + "|" + string(buf[0:n])
        buffer <- string(buf[0:n])
        // log.Debug(myIP.String() + " received " + string(buf[0:n]) + " from " + addr.String() )
        
        if err != nil {
            log.Error("Error: ",err)
        }
    }

    close(buffer)

    <-done
}