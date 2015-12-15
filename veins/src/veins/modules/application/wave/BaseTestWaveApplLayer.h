#ifndef BASETESTWAVEAPPLLAYER_H_
#define BASETESTWAVEAPPLLAYER_H_

#include <map>
#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/WaveShortMessage_m.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#define DBG EV

using Veins::TraCIMobility;
using Veins::TraCICommandInterface;
using Veins::AnnotationManager; //@KM

class BaseTestWaveApplLayer : public BaseApplLayer {

	public:
		~BaseTestWaveApplLayer();
		virtual void initialize(int stage);
		virtual void finish();

		virtual  void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);
		// Messages used by the application layer
		enum WaveApplMessageKinds {
			SERVICE_PROVIDER = LAST_BASE_APPL_MESSAGE_KIND,
			SEND_BEACON_EVT, SEND_EVENT_DRIVEN_MSG_EVT,
		};
		
		//std::vector<int> neighbourNode;
		//std::map<int,int> neighbourTable;

	protected:

		static const simsignalwrap_t mobilityStateChangedSignal;

		/** @brief: handles messages from the MAC */
		virtual void handleLowerMsg(cMessage* msg);

		/** @brief: handles node's own messages */
		virtual void handleSelfMsg(cMessage* msg);

		/** @brief: Creates the packet to be sent */
		virtual WaveShortMessage* prepareMsg(std::string name, int dataLengthBits, int psid, t_channel channel, int priority, int rcvId, int msgHopCount, int serial=0);

		/** @brief: Send the created packet to the lower layers */
		virtual void sendWSM(WaveShortMessage* wsm);
		
		/** @brief: Update the position of modules */
		virtual void handlePositionUpdate(cObject* obj);

		/** @brief: Processes the received beacon messages - refer to TraCIDemo11p in /src/modules/applications/ieee80211p/	*/
		virtual void onBeacon(WaveShortMessage* wsm) = 0;

		/** @brief: Processes the received event-driven messages */
		virtual void onEventDrivenMsg(WaveShortMessage* wsm) = 0;

		/** @brief: Processes the received data messages - refer to TraCIDemo11p in /src/modules/applications/ieee80211p/ */
		virtual void onData(WaveShortMessage* wsm) = 0;
		
	protected:

		TraCIMobility* mobility;
        TraCICommandInterface* traci;
        TraCICommandInterface::Vehicle* traciVehicle;
        AnnotationManager* annotations;

		static const simsignalwrap_t parkingStateChangedSignal; //@KM


	protected:
		// Beacon
		int beaconLengthBits;
		int beaconPriority;
		bool sendBeacons;
		int Beacon_msgSerialnum;
		cMessage* sendBeaconEvt;
		// Safety related messages
		int safetyLengthBits;
		// Warning Message
		int warningMsgPriority;
		int warningMsgInterval;
		bool sendWarningMsg;
		cMessage* sendWarningEvt;
		// Data
		int dataLengthBits;
		int dataPriority;
		bool sendData;
		bool dataOnSch;
		
		simtime_t individualOffset;
		Coord curPosition;
		Coord direction;
		
		int mySCH;
		int myId;
		int bitrate;
		int txPower;

		WaveAppToMac1609_4Interface* myMac;
};

#endif /* BASETESTWAVEAPPLLAYER_H_ */
