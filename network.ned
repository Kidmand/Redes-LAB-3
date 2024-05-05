simple Generator
{
    parameters:
        volatile double generationInterval; // sec
        volatile int    packetByteSize;     // byte
        @display("i=block/source");
    gates:
        output out;
}

simple Queue
{
    parameters:
        volatile int bufferSize;  // cant packets
        @display("i=block/queue;q=buffer");
    gates:
        input in;
        output out;
}

simple Sink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in;
}

simple TransportTx 
{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates: 
        input toApp;
        inout toOut;
}


simple TransportRx 
{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        output toApp;
        inout toOut;
}

module NodeTx 
{
    parameters:
        @display("i=block/routing");
    gates:
        inout  out;
    submodules:
        gen: Generator {
            parameters:
                @display("p=75,50");
        }
        traTx: TransportTx {
            parameters: 
                @display("p=75,190");
        }
    connections:
        gen.out --> traTx.toApp;
        traTx.toOut <--> out; 
}

module NodeRx 
{
    parameters:
        @display("i=block/routing");
    gates:
        inout in;
    submodules:
        sink: Sink {
            parameters:
                @display("p=75,50");
        }
        traRx: TransportRx {
            parameters: 
                @display("p=75,190");
        }
    connections:
        in <--> traRx.toOut;
        //case study 1:
        traRx.toApp --> { datarate = 0.5Mbps; } --> sink.in;
        //case study 2:
        // traRx.toOut --> { datarate = 1Mbps; } --> sink.in;
}



network Network
{
    @display("bgl=2");
    submodules:
        nodeTx: NodeTx {
            @display("p=30,30");
        }
        queue0: Queue {
            @display("p=200,30");
        }
        queue1: Queue {
            @display("p=200,100");
        }
        nodeRx: NodeRx {
            @display("p=370,30");
        }
    connections:
        //-------------------------------- LA IDA --------------------------------------------//
        
        // Enlace desde nodeTx.out --> queue0.in:
        nodeTx.out$o --> { datarate = 1Mbps; delay = 100us; } --> queue0.in;
     
        
        // Enlaces desde queue0 --> nodeRx.in:
        // - case study 1:
        queue0.out --> { datarate = 1Mbps; delay = 100us; } --> nodeRx.in$i;
        // - case study 2:
        // queue0.out --> { datarate = 0.5Mbps; delay = 100us; } --> nodeRx.in$i;
        
        //-------------------------------------------------------------------------------------//  
        
        //-------------------------------- LA VUELTA ------------------------------------------//

        //Enlaces desde nodeRx.in --> queue1.in:
        // - case study 1:
        nodeRx.in$o --> { datarate = 1Mbps; delay = 100us; } --> queue1.in;
        // - case study 2:
        // nodeRx.in$o --> { datarate = 0.5Mbps; delay = 100us; } --> queue1.in;
        
        
        //Enlace desde queue1.out --> nodeTx.in:
        queue1.out --> { datarate = 1Mbps; delay = 100us; } --> nodeTx.out$i; 

        //------------------------------------------------------------------------------------//
}


// ● Caso de estudio 1:
//     ○ NodeTx a Queue: datarate = 1 Mbps y delay = 100 us
//     ○ Queue a NodeRx: datarate = 1 Mbps y delay = 100 us
//     ○ Queue a Sink: datarate = 0.5 Mbps
// ● Caso de estudio 2:
//     ○ NodeTx a Queue: datarate = 1 Mbps y delay = 100 us
//     ○ Queue a NodeRx: datarate = 0.5 Mbps y delay = 100 us
//     ○ Queue a Sink: datarate = 1 Mbps