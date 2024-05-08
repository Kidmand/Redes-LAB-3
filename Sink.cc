#ifndef SINK
#define SINK

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Sink : public cSimpleModule
{
private:
    cStdDev delayStats;
    cOutVector delayVector;
    cOutVector packetsReceivedVector;
    int packetsReceived;

public:
    Sink();
    virtual ~Sink();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Sink);

Sink::Sink()
{
}

Sink::~Sink()
{
}

void Sink::initialize()
{
    // stats and vector names
    packetsReceivedVector.setName("packetReceived");
    packetsReceivedVector.record(0);
    packetsReceived = 0;
    delayStats.setName("TotalDelay");
    delayVector.setName("Delay");
}

void Sink::finish()
{
    // stats record at the end of simulation
    recordScalar("Avg delay", delayStats.getMean());
    recordScalar("Number of packets", delayStats.getCount());
}

void Sink::handleMessage(cMessage *msg)
{
    // compute queuing delay
    simtime_t delay = simTime() - msg->getCreationTime();
    // update stats
    delayStats.collect(delay);
    delayVector.record(delay);
    packetsReceived++;
    packetsReceivedVector.record(packetsReceived);
    // delete msg
    delete (msg);
}

#endif /* SINK */
