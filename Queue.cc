#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Queue : public cSimpleModule
{
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;

public:
    Queue();
    virtual ~Queue();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Queue);

Queue::Queue()
{
    endServiceEvent = NULL;
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
}

void Queue::finish()
{
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

#endif /* QUEUE */
