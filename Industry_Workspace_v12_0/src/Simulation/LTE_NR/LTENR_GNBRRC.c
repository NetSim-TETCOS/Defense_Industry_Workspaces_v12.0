/************************************************************************************
* Copyright (C) 2018                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Shashi Kant Suman                                                     *
*                                                                                  *
* ---------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "LTENR_GNBRRC.h"

static void start_mib_generation(NETSIM_ID gnbId,
								 NETSIM_ID gnbInterface)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = LTENR_MSG_MIB_REGENERATION_TIME;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbInterface;
	pevent.nProtocolId = MAC_PROTOCOL_LTE_NR;
	pevent.nSubEventType = LTENR_SUBEVNET_GENERATE_MIB;
	fnpAddEvent(&pevent);
}

void fn_NetSim_LTENR_GNBRRC_GenerateMIB()
{
//	ptrLTENR_MSG msg = LTENR_MSG_CREATE(LTENR_MSG_MIB);
}

void fn_NetSim_LTENR_GNBRRC_Init(NETSIM_ID gnbId, 
								 NETSIM_ID gnbInterface)
{

	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbInterface);
	ptrLTENR_GNBRRC rrc = calloc(1, sizeof* rrc);
	LTENR_GNBRRC_SET(gnbId, gnbInterface, rrc);

	//Read config parameter
	rrc->T300 = pd->T300 *MILLISECOND;
	
	//Start the MIB generation
	//start_mib_generation(gnbId, gnbInterface);
}


