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
    parameters;
        int bufferSize;
        @diplay("i=block/queue;q=buffer");
    gates: 
        input toApp;
        inout toOut;
}


simple TransportRx 
{
    parameters;
        int bufferSize;
        @diplay("i=block/queue;q=buffer");
    gates:
        output toApp;
        inout toOut;
}

module NodeTx 
{
    parameters:
        @display("i=block/routing");
    gates:
        output out;
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
        gen.out --> traTx.in;
        traTx.out --> out; 
}

module NodeRx 
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
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
        in --> traRx.in;
        //case study 1:
        traRx.out --> { datarate = 0.5Mps; } --> sink.in;
        //case study 2:
        // traRx.out --> { datarate = 1Mbps; } --> sink.in;
}



network Network
{
    @display("bgl=2");
    submodules:
        nodeTx: NodeTx {
            @display("p=30,30");
        }
        queue[0]: Queue {
            @display("p=130,30");
        }
        queue[1]: Queue {
            @display("p=130,50");
        }
        nodeRx: NodeRx {
            @display("p=230,30");
        }
    connections:
        //-------------------------------- LA IDA --------------------------------------------//
        
        //Enlace desde nodeTx.out --> queue[0].in:
        nodeTx.out --> { datarate = 1Mbps; delay = 100us; } --> queue[0].in;
     
        
        // Enlaces desde queue[0] --> nodeRx.in:
        // - case study 1:
        queue[0].out --> { datarate = 1Mbps; delay = 100us; } --> nodeRx.in;
        // - case study 2:
        // queue[0].out --> { datarate = 0.5Mbps; delay = 100us; } --> nodeRx.in;
        
        //-------------------------------------------------------------------------------------//  
        
        //-------------------------------- LA VUELTA ------------------------------------------//

        //Enlaces desde nodeRx.out --> queue[1].in:
        // - case study 1:
        nodeRx.out --> { datarate = 1Mbps; delay = 100us; } --> queue[1].in;
        // - case study 2:
        // nodeRx.out --> { datarate = 0.5Mbps; delay = 100us; } --> queue[1].in;
        
        
        //Enlace desde queue[1].out --> nodeTx.in:
        queue[1].out --> { datarate = 1Mbps; delay = 100us; } --> nodeTx.in 

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