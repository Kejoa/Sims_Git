//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

cplusplus {{
#include "veins/base/utils/Coord.h"
}}

class noncobject Coord;

packet WaveShortMessage {
	//Version of the Wave Short Message
	int wsmVersion = 0;
	//Unique number to identify the service
	int psid = 0;
	//Determine which security mechanism was used
	int securityType = 0;
	//Channel Number on which this packet was sent
	int channelNumber;
	//Data rate with which this packet was sent
	int dataRate = 1;
	//Power level with which this packet was sent
	int Tx_power;	
	//Provider Service Context
	string psc = "Service with some Data";
	//Length of Wave Short Message
	int wsmLength;
	//Data of Wave Short Message
	string wsmData = "Some Data";
	//Priority level with wich this packet was sent
	int priority;
	//Message sender MAC address
	int senderAddress = 0;
	//Message recipiant MAC address
	int recipientAddress = -1;
	//Message serial number
	int serial = 0;
	//Coordinate of the sender
	Coord senderPos;
	//Time at which the message was sent
	simtime_t timestamp = 0;
	//Current vehicle speed
	double VehicleSpeed;
	//Current vehicle direction
    string VehicleDirection;
    //Message hop counts
    int msgHopCount;
}
