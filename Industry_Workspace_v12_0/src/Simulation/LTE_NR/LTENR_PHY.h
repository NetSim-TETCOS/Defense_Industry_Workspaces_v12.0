#pragma once
/************************************************************************************
* Copyright (C) 2019																*
* TETCOS, Bangalore. India															*
*																					*
* Tetcos owns the intellectual property rights in the Product and its content.		*
* The copying, redistribution, reselling or publication of any or all of the		*
* Product or its content without express prior written consent of Tetcos is			*
* prohibited. Ownership and / or any other right relating to the software and all	*
* intellectual property rights therein shall remain at all times with Tetcos.		*
*																					*
* This source code is licensed per the NetSim license agreement.					*
*																					*
* No portion of this source code may be used as the basis for a derivative work,	*
* or used, for any purpose other than its intended use per the NetSim license		*
* agreement.																		*
*																					*
* This source code and the algorithms contained within it are confidential trade	*
* secrets of TETCOS and may not be used as the basis for any other software,		*
* hardware, product or service.														*
*																					*
* Author:    Shashi Kant Suman	                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#ifndef _NETSIM_LTENR_PHY_H_
#define _NETSIM_LTENR_PHY_H_
#ifdef  __cplusplus
extern "C" {
#endif

#include "LTENR_Spectrum.h"
#include "LTENR_PropagationModel.h"

	typedef struct stru_LTENR_propagationInfo
	{
		__IN__ NETSIM_ID gnbId;
		__IN__ NETSIM_ID gnbIf;
		__IN__ NETSIM_ID ueId;
		__IN__ NETSIM_ID ueIf;
		__IN__ NetSim_COORDINATES gnbPos;
		__IN__ NetSim_COORDINATES uePos;
		__IN__ double centralFrequency_MHz;
		__IN__ double bandwidth_MHz;
		__IN__ double txPower_dbm;
		__IN__ ptrLTENR_PROPAGATIONCONFIG propagationConfig;

		__OUT__ double dTotalLoss;
		__OUT__ double dPathLoss;
		__OUT__ double dShadowFadingLoss;
		__OUT__ double dO2ILoss;

		//Will be calculated based on input and output
		double rxPower_dbm;
		double thermalNoise;
		double EB_by_N0;
		double SNR_db;
		double spectralEfficiency;

		//Local to Propagation model
		struct {
			bool isConstructiveShadow;
			double Gset;
			double Iset;
		}SHADOWVAR;

		LTENR_POSITION uePosition;
		LTENR_POSITION gnbPosition;

		double frequency_gHz;

		LTENR_SCENARIO currentScenario;
		double dist2D;
		double dist3D;
		double dist2Dindoor;
		double dist2Doutdoor;

	}LTENR_PROPAGATIONINFO, * ptrLTENR_PROPAGATIONINFO;

	typedef struct stru_LTENR_UEPHY
	{
		NETSIM_ID ueId;
		NETSIM_ID ueIf;
		NETSIM_ID gnBId;
		NETSIM_ID gnbIf;

		ptrLTENR_PROPAGATIONCONFIG propagationConfig;
	}LTENR_UEPHY, * ptrLTENR_UEPHY;
	ptrLTENR_UEPHY LTENR_UEPHY_NEW(NETSIM_ID ueId, NETSIM_ID ueIf);

	typedef struct stru_LTENR_AssociatedUEPhyInfo
	{
		NETSIM_ID ueId;
		NETSIM_ID ueIf;

		ptrLTENR_AMCINFO uplinkAMCInfo;
		ptrLTENR_AMCINFO downlinkAMCInfo;

		ptrLTENR_PROPAGATIONINFO propagationInfo;
		_ptr_ele ele;
	}LTENR_ASSOCIATEDUEPHYINFO, * ptrLTENR_ASSOCIATEDUEPHYINFO;
#define LTENR_ASSOCIATEDUEPHYINFO_ALLOC()			(list_alloc(sizeof(LTENR_ASSOCIATEDUEPHYINFO),offsetof(LTENR_ASSOCIATEDUEPHYINFO,ele)))
#define LTENR_ASSOCIATEDUEPHYINFO_ADD(phy,info)		(LIST_ADD_LAST(&(phy)->associatedUEPhyInfo,(info)))
#define LTENR_ASSOCIATEDUEPHYINFO_NEXT(info)		((info) = LIST_NEXT((info)))

	typedef enum enum_SLOTTYPE
	{
		SLOT_UPLINK,
		SLOT_DOWNLINK,
		SLOT_MIXED,
	}LTENR_SLOTTYPE;
	static char strLTENR_SLOTTYPE[][50] = {"UPLink","Downlink","Mixed"};

	typedef struct stru_LTENR_FrameInfo
	{
		UINT frameId;
		double frameStartTime;
		double frameEndTime;

		UINT subFrameId;
		double subFrameStartTime;
		double subFrameEndTime;

		UINT slotId;
		double slotStartTime;
		double slotEndTime;
		LTENR_SLOTTYPE slotType;
		LTENR_SLOTTYPE prevSlotType;
	}LTENR_FRAMEINFO, * ptrLTENR_FRAMEINFO;

	typedef struct stru_LTENR_GNBPHY
	{
		NETSIM_ID gnbId;
		NETSIM_ID gnbIf;

		ptrLTENR_SPECTRUMCONFIG spectrumConfig;

		double gnbHeight;
		//config parameter -- PDSCHConfig
		struct stru_pdschConfig
		{
			void* mcsTable;
			UINT xOverhead;
		}PDSCHConfig;

		//config parameter -- PUSCHConfig
		struct stru_puschConfig
		{
			void* mcsTable;
			bool isTransformPrecoding;
		}PUSCHConfig;

		//config parameter -- CSI-Reporting
		struct stru_CSIReportConfig
		{
			void* cqiTable;
		}CSIReportConfig;

		ptrLTENR_PROPAGATIONCONFIG propagationConfig;

		ptrLTENR_PRB prbList;

		ptrLTENR_FRAMEINFO frameInfo;

		UINT associatedUECount;
		ptrLTENR_ASSOCIATEDUEPHYINFO associatedUEPhyInfo;
	}LTENR_GNBPHY, * ptrLTENR_GNBPHY;
	ptrLTENR_GNBPHY LTENR_GNBPHY_NEW(NETSIM_ID gnbId, NETSIM_ID gnbIf);
	
	//LTENR-AMC
	void LTENR_SetCQITable(ptrLTENR_GNBPHY phy, char* table);
	void LTENR_SetPDSCHMCSIndexTable(ptrLTENR_GNBPHY phy, char* table);
	void LTENR_SetPUSCHMCSIndexTable(ptrLTENR_GNBPHY phy, char* table);

	//Spectrum config
#define LTENR_PHY_GET_SPECTRUMCONFIG(d,in) (((ptrLTENR_GNBPHY)LTENR_GNBPHY_GET((d),(in)))->spectrumConfig)
	void LTENR_PHY_GET_OH(ptrLTENR_SPECTRUMCONFIG sc,
						  double* dlOH,
						  double* ulOH);

	//Propagation model
	void LTENR_Propagation_TotalLoss(ptrLTENR_PROPAGATIONINFO info);
#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_PHY_H_ */
