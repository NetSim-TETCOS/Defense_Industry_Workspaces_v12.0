#pragma once
/************************************************************************************
 * Copyright (C) 2018
 * TETCOS, Bangalore. India															*

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 * Author:	Shashi Kant Suman														*
 * ---------------------------------------------------------------------------------*/
#ifndef _NETSIM_LTENR_SUBEVENT_H_
#define _NETSIM_LTENR_SUBEVENT_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define LTENR_MAX_SUBEVENT	100 // Read-only.

	typedef enum enum_LTENR_Subevnet_type
	{
		LTENR_SUBEVNET_GENERATE_MIB = MAC_PROTOCOL_LTE_NR*100,

		//PDCP
		LTENR_SUBEVENT_PDCP_DISCARDTIMER,
		LTENR_SUBEVENT_PDCP_TREORDERINGTIMER,

		//RLC
		LTENR_SUBEVNT_RLC_TREASSEMBLYTIMER,
		LTENR_SUBEVNT_RLC_TPOLLRETRANSMITTIMER,
		LTENR_SUBEVNT_RLC_TSTATUSPROHIBITTIMER,

		//MAC

		//PHY
		LTENR_SUBEVENT_PHY_STARTFRAME,
		LTENR_SUBEVENT_PHY_STARTSUBFRAME,
		LTENR_SUBEVENT_PHY_STARTSLOT,

		LTENR_SUBEVENT_LAST, // Keep me at last
	}LTENR_SUBEVENTTYPE;
	
	typedef struct stru_LTENR_SubeventDataBase
	{
		LTENR_SUBEVENTTYPE subEvent;
		char* subeventName;
		void(*fnSubEventfunction)();
	}LTENR_SUBEVENT, *ptrLTENR_SUBEVENT;
	
	void LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENTTYPE type,
								 char* name,
								 void(*fun)());
	char* LTENR_SUBEVNET_NAME(NETSIM_ID id);
	void LTENR_SUBEVENT_CALL();

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_SUBEVENT_H_
