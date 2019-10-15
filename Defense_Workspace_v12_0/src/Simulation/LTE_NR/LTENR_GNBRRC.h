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
#ifndef _NETSIM_LTENR_GNBRRC_H_
#define _NETSIM_LTENR_GNBRRC_H_

#ifdef  __cplusplus
extern "C" {
#endif
	
	//Default Timer
#define LTENR_MSG_MIB_REGENERATION_TIME		40*MILLISECOND

	//Default packet size
#define LTENR_MSG_MIB_SIZE					3	// Bytes


	typedef enum enum_LTENR_GNB_RRCSTATE
	{
		RRCSTATE_NULL,
		RRCSTATE_RANDOMACCESS,
		RRCSTATE_CONNSETUP,
		RRCSTATE_CONNREJECT,
		RRCSTATE_CONNECTED,
		RRCSTATE_HANDOVERJOIN,
		RRCSTATE_HANDOVERPATHSWITCH,
		RRCSTATE_HANDOVERPREP,
		RRCSTATE_RECONFIGURATION,
		RRCSTATE_HANDOVERLEAVING,
		RRCSTATE_DESTROYED,
	}LTENR_GNB_RRCSTATE;

	typedef struct stru_LTENR_GNBRRC
	{
		double T300;
	}LTENR_GNBRRC, *ptrLTENR_GNBRRC;

	void fn_NetSim_LTENR_GNBRRC_GenerateMIB();

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_GNBRRC_H_
