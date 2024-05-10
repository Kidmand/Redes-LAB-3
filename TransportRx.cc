#ifndef TRANSPORT_RX
#define TRANSPORT_RX

#include <string.h>
#include <omnetpp.h>
#include "./FeedbackPkt_m.h"

using namespace omnetpp;
class TransportRx : public cSimpleModule
{
private:
    cQueue buffer;
    bool isBufferSaturated;
    bool isNetworkSaturated;
    cMessage *endServiceEvent;
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
    cOutVector sendFeedbackPktVector;

public:
    TransportRx();
    virtual ~TransportRx();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    void sendFeedbackPkt(bool isBufferSaturated);
    bool isActuallBufferSatured(void);
};

Define_Module(TransportRx);

TransportRx::TransportRx()
{
    endServiceEvent = NULL;
    isBufferSaturated = false;
    isNetworkSaturated = false;
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

    sendFeedbackPktVector.setName("SendFeedbackPktVector");
    sendFeedbackPktVector.record(0); // Inicializar con 0
}

void TransportRx::finish()
{
}

void TransportRx::sendFeedbackPkt(bool isBufferSaturated)
{
    // Create FeedbackPkt:
    FeedbackPkt *feedbackPkt = new FeedbackPkt();

    // Usamos este tamaño debido a que FeedbackPkt tiene simplemente un campo booleano.
    feedbackPkt->setByteLength(1);

    // Establecemos el tipo de mensaje a 2, para indicar que es un FeedbackPkt.
    feedbackPkt->setKind(2);

    // Establecemos el valor de isBufferSaturated en el FeedbackPkt.
    feedbackPkt->setIsBufferSaturated(isBufferSaturated);

    // Enviamos el FeedbackPkt por la salida correspondiente.
    send(feedbackPkt, "toOut$o");
}

bool TransportRx::isActuallBufferSatured(void)
{
    return buffer.getLength() > par("bufferSize").intValue() * par("bufferCota").doubleValue();
}

void TransportRx::handleMessage(cMessage *msg)
{
    if (msg->getKind() == 2) // Recibimos un FeedbackPkt desde algun nodo de la red.
    {
        isNetworkSaturated = ((FeedbackPkt *)msg)->getIsBufferSaturated();
        delete (msg);
    }
    else if (msg == endServiceEvent) // if msg is signaling an endServiceEvent
    {
        // if packet in buffer, send next one
        if (!buffer.isEmpty())
        {
            // dequeue packet
            cPacket *pkt = (cPacket *)buffer.pop();
            // send packet
            send(pkt, "toApp");

            // start new service
            scheduleAt(simTime() + pkt->getDuration(), endServiceEvent);

            // Manejo de FeedbackPkt:
            isBufferSaturated = isActuallBufferSatured();

            // FIXME: borrar en algun momento estos prints:
            // EV << "RX: IsBufferSaturated: " << (isBufferSaturated ? "True" : "False") << endl;
            // EV << "RX: isNetworkSaturated: " << (isNetworkSaturated ? "True" : "False") << endl;

            bool response = isBufferSaturated || isNetworkSaturated;
            sendFeedbackPkt(response);
            sendFeedbackPktVector.record(response ? 1 : 0);
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