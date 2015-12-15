#include "veins/modules/application/wave/BaseTestWaveApplLayer.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;   //@KM

const simsignalwrap_t BaseTestWaveApplLayer::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);
const simsignalwrap_t BaseTestWaveApplLayer::parkingStateChangedSignal = simsignalwrap_t(TRACI_SIGNAL_PARKING_CHANGE_NAME);    //@KM

//Define_Module(BaseTestWaveApplLayer);

// Initialize node's parameter @KM
void BaseTestWaveApplLayer::initialize(int stage) {
	BaseApplLayer::initialize(stage);

	if (stage==0) {
		// Find the modules MAC
	    myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
		assert(myMac);
		
		// To access mobility info of a vehicle
		mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();

		myId = getParentModule()->getIndex(); // Get node's id

		headerLength = par("headerLength").longValue();
		double maxOffset = par("maxOffset").doubleValue();
		// Beacons @KM
		sendBeacons = par("sendBeacons").boolValue();
		beaconLengthBits = par("beaconLengthBits").longValue();
		beaconPriority = par("beaconPriority").longValue();
		Beacon_msgSerialnum = 0;
		// Safety related message @KM
		safetyLengthBits = par("safetyLengthBits").longValue();
		// Warning message @KM
		sendWarningMsg = par("sendWarningMsg").boolValue();
		warningMsgPriority = par("warningMsgPriority").longValue();
		warningMsgInterval = par("warningMsgInterval").doubleValue();
		// Data @KM
		sendData = par("sendData").boolValue();
		dataLengthBits = par("dataLengthBits").longValue();
		dataPriority = par("dataPriority").longValue();
		dataOnSch = par("dataOnSch").boolValue();
		
		individualOffset = dblrand() * maxOffset;
	}
}

// Prepare message to send @KM
WaveShortMessage* BaseTestWaveApplLayer::prepareMsg(std::string name, int lengthBits, int psid, t_channel channel, int priority,
														int rcvId, int msgHopCount, int serial) {
	WaveShortMessage* newWSMmsg = new WaveShortMessage(name.c_str());
	newWSMmsg->addBitLength(headerLength);
	newWSMmsg->addBitLength(lengthBits);

	newWSMmsg->setWsmVersion(1);
	newWSMmsg->setPsid(psid);

	switch (channel) {
		case type_SCH: newWSMmsg->setChannelNumber(Channels::SCH1); break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
		case type_CCH: newWSMmsg->setChannelNumber(Channels::CCH); break;
	}

	newWSMmsg->setDataRate(bitrate);
	newWSMmsg->setTx_power(txPower);

	newWSMmsg->setPriority(priority);
	newWSMmsg->setTimestamp(simTime());
	newWSMmsg->setSenderAddress(myId);
	newWSMmsg->setRecipientAddress(rcvId);
	newWSMmsg->setSenderPos(curPosition);
	newWSMmsg->setSerial(serial);
	newWSMmsg->setMsgHopCount(msgHopCount);

	direction = mobility->getDirection();
	if (direction.x < 0) {
	    newWSMmsg->setVehicleDirection("South");
	}
	else {
	    newWSMmsg->setVehicleDirection("North");
	}

	if (name == "beacon") {
		DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << newWSMmsg->getTimestamp() << std::endl;
		newWSMmsg->setVehicleSpeed(mobility->getSpeed());
		newWSMmsg->setWsmData("beacon");
	}	
	if (name == "warning") {
		DBG << "Creating Warning message with Priority " << priority << " at Applayer at " << newWSMmsg->getTimestamp() << std::endl;
		newWSMmsg->setWsmData("warning");
	}
	if (name == "data") {
		DBG << "Creating Data with Priority " << priority << " at Applayer at " << newWSMmsg->getTimestamp() << std::endl;
		newWSMmsg->setWsmData("data");
	}

	return newWSMmsg;
}

void BaseTestWaveApplLayer::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
	Enter_Method_Silent();
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate(obj);
	}
}

void BaseTestWaveApplLayer::handlePositionUpdate(cObject* obj) {
	ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
	curPosition = mobility->getCurrentPosition();
}

// Process messages from the MAC layer @KM
void BaseTestWaveApplLayer::handleLowerMsg(cMessage* msg) {
	WaveShortMessage* LowerMsg = dynamic_cast<WaveShortMessage*>(msg);
	ASSERT(LowerMsg);
	if (std::string(LowerMsg->getName()) == "beacon") {
		onBeacon(LowerMsg);
	}
	else if (std::string(LowerMsg->getName()) == "warning") {
		onEventDrivenMsg(LowerMsg);
	}
	else if (std::string(LowerMsg->getName()) == "data") {
		onData(LowerMsg);
	}
	else {
		DBG << "unknown message (" << LowerMsg->getName() << ")  received\n";
	}
	delete(msg);
}

// Process internal timer messages @KM
void BaseTestWaveApplLayer::handleSelfMsg(cMessage* msg) {
	DBG<< "KM---> Generate new msg. "<<std::endl;
	switch (msg->getKind()) {
		case SEND_BEACON_EVT: {
			Beacon_msgSerialnum = Beacon_msgSerialnum + 1;
		    WaveShortMessage* MAC_pkt = prepareMsg("beacon", beaconLengthBits, 0, type_CCH, beaconPriority, 0, 0, Beacon_msgSerialnum);
		    DBG<< "----> Beacon msg : length "<<MAC_pkt->getBitLength()<<", Seq num "<<MAC_pkt->getSerial()<<std::endl;
			sendWSM(MAC_pkt);
			scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
			break;
		}
		case SEND_EVENT_DRIVEN_MSG_EVT: {
			sendWSM(prepareMsg("warning", safetyLengthBits, 2, type_CCH, warningMsgPriority, 0, 0,-1));
			//scheduleAt(simTime() + par("warningMsgInterval").doubleValue(), sendWarningEvt);
			break;
		}
		default: {
			if (msg)
				DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
			break;
		}
	}
}

void BaseTestWaveApplLayer::sendWSM(WaveShortMessage* wsm) {
    DBG <<"---> Sending packet to the MAC layer."<<std::endl;
    sendDelayedDown(wsm,individualOffset);
}

// Cancel all dynamic allocated messages @KM
void BaseTestWaveApplLayer::finish() {
    if (sendBeaconEvt->isScheduled()) {
        cancelAndDelete(sendBeaconEvt);
    }
    else if (sendWarningEvt->isScheduled()) {
        cancelAndDelete(sendWarningEvt);
    }
    else {
        delete sendBeaconEvt;
        delete sendWarningEvt;
    }

    findHost()->unsubscribe(mobilityStateChangedSignal, this);
}

BaseTestWaveApplLayer::~BaseTestWaveApplLayer() {}


