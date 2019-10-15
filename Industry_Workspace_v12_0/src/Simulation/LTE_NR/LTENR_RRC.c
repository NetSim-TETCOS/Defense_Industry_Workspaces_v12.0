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

static void LTENR_GNBRRC_register_callback_function()
{
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVNET_GENERATE_MIB,
							"LTENR_GENERATE_MIB",
							fn_NetSim_LTENR_GNBRRC_GenerateMIB);
}

void fn_NetSim_LTENR_RRC_Init()
{
	LTENR_GNBRRC_register_callback_function();
}

void fn_NetSim_LTENR_RRC_MacOut()
{
#pragma message(__LOC__"Implement the function "__FUNCTION__)
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	packet->nReceiverId = LTENR_FIND_ASSOCIATEDGNB(d, in);
	LTENR_CallPDCPOut();
}

bool fn_NetSim_LTENR_RRC_isConnected(NETSIM_ID d, NETSIM_ID in,
									 NETSIM_ID r, NETSIM_ID rin)
{
#pragma message(__LOC__"Implement function "__FUNCTION__)
	return true;
}
