#ifndef TRANSPORT_RX
#define TRANSPORT_RX

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;
class TransportRx : public cSimpleModule
{
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
    cOutVector sendAckVector;

public:
    TransportRx();
    virtual ~TransportRx();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportRx);

TransportRx::TransportRx()
{
    endServiceEvent = NULL;
}

TransportRx::~TransportRx()
{
    cancelAndDelete(endServiceEvent);
}

void TransportRx::initialize()
{
    buffer.setName("buffer");
    endServiceEvent = new cMessage("endService");

    // Inicializamos los vectores para almacenar los datos:
    bufferSizeVector.setName("BufferSizeVector");
    bufferSizeVector.record(0); // Inicializar con 0

    packetDropVector.setName("PacketDropVector");
    packetDropVector.record(0); // Inicializar con 0

    sendAckVector.setName("SendAckVector");
    sendAckVector.record(0); // Inicializar con 0
}

void TransportRx::finish()
{
}

void TransportRx::handleMessage(cMessage *msg)
{
    // if msg is signaling an endServiceEvent
    if (msg == endServiceEvent)
    {
        // if packet in buffer, send next one
        if (!buffer.isEmpty())
        {
            // dequeue packet
            cPacket *pkt = (cPacket *)buffer.pop();
            // send packet
            send(pkt, "toApp");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    }
    else
    {
        // Verificar si el buffer está lleno
        if (buffer.getLength() >= par("bufferSize").intValue())
        {
            delete msg; // Descartar el paquete
            this->bubble("packet dropped");
            packetDropVector.record(1);
        }
        else
        {
            // Enqueue the packet
            buffer.insert(msg);
            bufferSizeVector.record(buffer.getLength());
            // If the server is idle
            if (!endServiceEvent->isScheduled())
            {
                // Start the service
                scheduleAt(simTime() + 0, endServiceEvent);
            }
        }
    }
}

#endif /* TRANSPORT_RX */