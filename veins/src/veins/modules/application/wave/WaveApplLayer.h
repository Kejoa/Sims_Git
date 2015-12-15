#ifndef WaveApplLayer_H_
#define TESTEAPPL_0_H_

#include "veins/base/modules/BaseModule.h"
#include "veins/modules/application/wave/BaseTestWaveApplLayer.h"
#include <map>
#include <vector>
#include <iostream>
#include <math.h>
#include "veins/base/utils/Coord.h"

#ifndef DBG
#define DBG EV
#endif

/**
 * @brief: WAVE application layer test_0.
 *
 * @ingroup applLayer
 */

//using Veins::TraCIMobility;
//using Veins::AnnotationManager;   @KM

class WaveApplLayer  :  public BaseTestWaveApplLayer {
    public:

        virtual ~WaveApplLayer();
        virtual void initialize(int stage);
        virtual void receiveSignal(cComponent* source,simsignal_t signalID, cObject* obj);
        virtual void finish();
	
	public:


    protected:
        
        simtime_t lastDroveAt;

    protected:
        virtual void onBeacon(WaveShortMessage* beaconMsg);
        virtual void onEventDrivenMsg(WaveShortMessage* safetyMsg);
        virtual void onData(WaveShortMessage* dataMsg);
        virtual void handlePositionUpdate(cObject* obj);
        virtual void handleParkingUpdate(cObject* obj);
        //virtual void sendWSM(WaveShortMessage* wsm);
        void reTransmitMessage(WaveShortMessage* safetyMsg);

    protected:

		bool sentMessage;
        bool isParking;
        bool sendWhileParking;

        std::string rcvdMsgName;
        int rcvdMsgPsid;
        int rcvdMsgNewPriority;
        int rcvdMsgHopCount;
        simtime_t rcvdMsgTimeToLive;
};

#endif /* WaveApplLayer_H_ */
