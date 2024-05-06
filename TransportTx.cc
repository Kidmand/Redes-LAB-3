#ifndef TRANSPORT_TX
#define TRANSPORT_TX

#include <string.h>
#include <omnetpp.h>
#include "./FeedbackPkt_m.h"

using namespace omnetpp;
class TransportTx : public cSimpleModule
{
private:
    unsigned int delayPkt;
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
    cOutVector receiveAckVector;

public:
    TransportTx();
    virtual ~TransportTx();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportTx);

TransportTx::TransportTx()
{
    endServiceEvent = NULL;
    delayPkt = 0;
}

TransportTx::~TransportTx()
{
    cancelAndDelete(endServiceEvent);
}

void TransportTx::initialize()
{
    buffer.setName("buffer");
    endServiceEvent = new cMessage("endService");

    // Inicializamos los vectores para almacenar los datos:
    bufferSizeVector.setName("BufferSizeVector");
    bufferSizeVector.record(0); // Inicializar con 0

    packetDropVector.setName("PacketDropVector");
    packetDropVector.record(0); // Inicializar con 0

    receiveAckVector.setName("PacketDropVector");
    receiveAckVector.record(0); // Inicializar con 0
}

void TransportTx::finish()
{
}

void TransportTx::handleMessage(cMessage *msg)
{
    // if msg is signaling an endServiceEvent
    if (msg->getKind() == 2)
    {
        FeedbackPkt *feedbackPkt = (FeedbackPkt *)msg;

        // FIXME: borrar en algun momento estos prints:
        // EV << "TX: IsBufferSaturated: " << (feedbackPkt->getIsBufferSaturated() ? "True" : "False") << endl;
        if (feedbackPkt->getIsBufferSaturated())
        {
            delayPkt += 1;
        }
        else
        {
            delayPkt = 0;
        }

        delete (msg);
    }
    else if (msg == endServiceEvent)
    {
        // if packet in buffer, send next one
        if (!buffer.isEmpty())
        {
            // dequeue packet
            cPacket *pkt = (cPacket *)buffer.pop();
            // send packet
            send(pkt, "toOut$o");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime + delayPkt, endServiceEvent);
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

#endif /*TRANSPORT_TX */