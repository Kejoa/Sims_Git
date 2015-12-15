#include "veins/modules/application/wave/WaveApplLayer.h"

Define_Module(WaveApplLayer);

void WaveApplLayer::initialize(int stage) {
    BaseTestWaveApplLayer::initialize(stage);
    if(stage == 0) {
        
        sentMessage = false;
        lastDroveAt = simTime();
        isParking = false;
        //sendWhileParking = par("sendWhileParking").boolValue();

		rcvdMsgHopCount = 0;

        findHost()->subscribe(parkingStateChangedSignal,this);
        findHost()->subscribe(mobilityStateChangedSignal, this);

        //simulate asynchronous channel access @KM
        //double beaconOffSet = genk_dblrand(k) * (par("beaconInterval").doubleValue()/2);
        double beaconOffSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        beaconOffSet = beaconOffSet + floor(beaconOffSet/0.050)*0.050;

        double warningOffSet = dblrand() * (par("warningMsgInterval").doubleValue()/2);
        warningOffSet = warningOffSet + floor(warningOffSet/0.050)*0.050;

        // Self Messages kind @KM

        sendWarningEvt = new cMessage("warning Message evt", SEND_EVENT_DRIVEN_MSG_EVT);
        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);

        // Prepare to send a beacons on start-up. @KM
        simtime_t startTime = simTime();

        if (sendBeacons) {
            DBG << "Node " << myId << " getting ready to start send beacons" << std::endl;
            scheduleAt(startTime + beaconOffSet, sendBeaconEvt);
        }
    }
}

void WaveApplLayer::onBeacon(WaveShortMessage* beaconMsg) {

    DBG << "Received beacon from Node [" << beaconMsg->getSenderAddress() << "] at " << simTime() << std::endl;

}

void WaveApplLayer::onEventDrivenMsg(WaveShortMessage* safetyMsg) {

    DBG << "Received safety related message, from Node [" << safetyMsg->getSenderAddress() << "] at: " << simTime() << std::endl;

    rcvdMsgPsid = safetyMsg->getPsid();
    rcvdMsgHopCount = safetyMsg->getMsgHopCount();
    rcvdMsgTimeToLive = safetyMsg->getTimestamp();
    DBG <<"-----> Message time to live : "<<(simTime() - rcvdMsgTimeToLive)<<std::endl;

    int rcvdMsgHops = rcvdMsgHopCount;
    if((simTime() - rcvdMsgTimeToLive) < 0.5){
    //if(rcvdMsgHops <= 3){
        safetyMsg->setMsgHopCount(rcvdMsgHops++);
        safetyMsg->setWsmData("reDENMs");
        safetyMsg->setSenderAddress(myId);
        std::string senderDir = safetyMsg->getVehicleDirection();   // Get vehicles direction
        reTransmitMessage(safetyMsg);
        /*else if(rcvdMsgName == "warning") {

            std::string senderDir = safetyMsg->getVehicleDirection();   // Get vehicles direction
            Coord senderPos = safetyMsg->getSenderPos();
            Coord receiverPos = mobility->getCurrentPosition();
            EV<<"KM---> msgPrio(): 2;"<<" Sender vehicle direction: "<<senderDir<<
                    "; Sender position: "<<safetyMsg->getSenderPos()<<
                    "; Receiver position: "<<receiverPos<<std::endl;
            if(senderPos.x > receiverPos.x) {
                double dist = senderPos.x - receiverPos.x;
                dist = fabs(dist);
                double Dista = mobility->getDistance(senderPos);
                EV<<"KM---> Receiving node is behind, by "<<Dista<<" : "<<dist<<" meters"<<std::endl;
                if(dist < 166){
                    rcvdMsgNewPriority = 1;
                }
                else if(dist >= 166 && dist < 333){
                    rcvdMsgNewPriority = 2;
                }
                else if(dist >= 333 && dist <= 500){
                    rcvdMsgNewPriority = 3;
                }
                reTransmitMessage(safetyMsg);
            }
            else {
                double dist = senderPos.x - receiverPos.x;
                dist = fabs(dist);
                EV<<"KM---> Receiving node is in front, by "<<dist<<"meters"<<std::endl;
            }
        }*/
    }
    else{
        //DBG <<"-----> Message dropped due to high hop counts"<<std::endl;
        DBG <<"-----> Message dropped due to elapsed time to live"<<std::endl;
    }
}

void WaveApplLayer::onData(WaveShortMessage* dataMsg) {

}

void WaveApplLayer::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
    else if (signalID == parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}

void WaveApplLayer::handleParkingUpdate(cObject* obj) {
    isParking = mobility->getParkingState();
    if (sendWhileParking == false) {
        if (isParking == true) {
            (FindModule<BaseConnectionManager*>::findGlobalModule())->unregisterNic(this->getParentModule()->getSubmodule("nic"));
        }
        else {
            Coord pos = mobility->getCurrentPosition();
            (FindModule<BaseConnectionManager*>::findGlobalModule())->registerNic(this->getParentModule()->getSubmodule("nic"), (ChannelAccess*) this->getParentModule()->getSubmodule("nic")->getSubmodule("phy80211p"), &pos);
        }
    }
}

void WaveApplLayer::handlePositionUpdate(cObject* obj) {
    BaseTestWaveApplLayer::handlePositionUpdate(obj);

    // stopped for for at least 10s?
    if (mobility->getSpeed() < 1) {
        DBG << "Node " << myId << " has stopped."<< std::endl;
        if (simTime() - lastDroveAt >= 2) {
            findHost()->getDisplayString().updateWith("r=16,red");

            if (!sentMessage){
                DBG << "Node " << myId << " has stopped. Getting ready to start send a safety message" << std::endl;
                sentMessage = true;
                if (sendWarningEvt->isScheduled()) {
                        //cancelAndDelete(sendWarningEvt);
                    }
                else{
                    if(sendWarningMsg){
                        scheduleAt(simTime(), sendWarningEvt);
                    }
                }
            }
        }
    }
    else {
        lastDroveAt = simTime();
    }
}

void WaveApplLayer::reTransmitMessage(WaveShortMessage* safetyMsg) {
    //sentMessage = true;
    sendWSM(safetyMsg->dup());
}

// To do later
/*void WaveApplLayer::sendWSM(WaveShortMessage* wsm) {
    if (isParking && !sendWhileParking)
        return;
    sendDelayedDown(wsm,individualOffset);
}*/

void WaveApplLayer::finish() {


}

WaveApplLayer::~WaveApplLayer() {

}

