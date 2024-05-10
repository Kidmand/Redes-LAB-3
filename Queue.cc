#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <omnetpp.h>
#include "./FeedbackPkt_m.h"

using namespace omnetpp;

class Queue : public cSimpleModule
{
private:
    cQueue buffer;
    bool isBufferSaturated;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
    cOutVector feedbackVector;

public:
    Queue();
    virtual ~Queue();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    void sendFeedbackPkt(bool isBufferSaturated);
    bool isActuallBufferSatured(void);
};

Define_Module(Queue);

Queue::Queue()
{
    endServiceEvent = NULL;
    isBufferSaturated = false;
}

Queue::~Queue()
{
    cancelAndDelete(endServiceEvent);
}

void Queue::initialize()
{
    buffer.setName("buffer");
    endServiceEvent = new cMessage("endService");

    // Inicializar bufferSizeVector si es necesario
    bufferSizeVector.setName("BufferSizeVector"); // Establecer el nombre del vector de salida
    bufferSizeVector.record(0);                   // Inicializar el vector con un valor inicial (puede ser el tamaño inicial del buffer)

    // Inicializar packetDropVector si es necesario
    packetDropVector.setName("PacketDropVector");
    packetDropVector.record(0); // Inicializar con 0, si no hay descartes al inicio de la simulación

    // Inicializar feedbackVector si es necesario
    feedbackVector.setName("FeedbackVector");
    feedbackVector.record(0); // Inicializar con 0, si no hay feedbacks al inicio de la simulación
}

void Queue::finish()
{
}

void Queue::sendFeedbackPkt(bool isBufferSaturated)
{
    // Create FeedbackPkt:
    FeedbackPkt *feedbackPkt = new FeedbackPkt();

    // Usamos este tamaño debido a que FeedbackPkt tiene simplemente un campo booleano.
    feedbackPkt->setByteLength(1);

    // Establecemos el tipo de mensaje a 2, para indicar que es un FeedbackPkt.
    feedbackPkt->setKind(2);

    // Establecemos el valor de isBufferSaturated en el FeedbackPkt.
    feedbackPkt->setIsBufferSaturated(isBufferSaturated);

    // Lo colocamos en la cola de salida para que se envie en el siguiente ciclo.
    buffer.insertBefore(buffer.front(), feedbackPkt);
}

bool Queue::isActuallBufferSatured(void)
{
    return buffer.getLength() > par("bufferSize").intValue() * par("bufferCota").doubleValue();
}

void Queue::handleMessage(cMessage *msg)
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
            send(pkt, "out");
            // start new service
            serviceTime = par("serviceTime");
            scheduleAt(simTime() + serviceTime + pkt->getDuration(), endServiceEvent);

            // Verificar si el buffer está saturado y  enviamos el feedback correspondiente:
            if (isActuallBufferSatured())
            {
                // Si el buffer está saturado y no lo estaba antes enviamos
                // un feedbackIPkt avisando que el buffer está saturado:
                if (!isBufferSaturated)
                {
                    isBufferSaturated = true;
                    sendFeedbackPkt(isBufferSaturated);
                    feedbackVector.record(1);
                }
            }
            else
            {
                // Si el buffer no está saturado y lo estaba antes enviamos
                // un feedbackIPkt avisando que el buffer no está saturado:
                if (isBufferSaturated)
                {
                    isBufferSaturated = false;
                    sendFeedbackPkt(isBufferSaturated);
                    feedbackVector.record(0);
                }
            }
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

#endif /* QUEUE */
