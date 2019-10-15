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
#include "stdafx.h"
#include "LTENR_MAC.h"
#include "LTENR_PHY.h"

#pragma region FUNCTION_PROTOTYPE
//PRB
static void LTENR_form_prb_list(NETSIM_ID d, NETSIM_ID in);

//Frame
static void LTENR_addStartFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time);
void LTENR_handleStartFrameEvent();

//SubFrame
static void LTENR_addStartSubFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time);
void LTENR_handleStartSubFrameEvent();

//Slot
static void LTENR_addStartSlotEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time);
void LTENR_handleStartSlotEvent();

//Association
void LTENR_PHY_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
						   NETSIM_ID ueId, NETSIM_ID ueIf,
						   bool isAssociated);
#pragma endregion

#pragma region PHY_INIT
void fn_NetSim_LTENR_PHY_Init()
{
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PHY_STARTFRAME, "LTENR_STARTFRAME", LTENR_handleStartFrameEvent);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PHY_STARTSUBFRAME, "LTENR_STARTSUBFRAME", LTENR_handleStartSubFrameEvent);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PHY_STARTSLOT, "LTENR_STARTSLOT", LTENR_handleStartSlotEvent);

	fn_NetSim_LTENR_RegisterCallBackForAssociation(LTENR_PHY_ASSOCIATION);

}

void fn_NetSim_LTENR_UEPHY_Init(NETSIM_ID ueId, NETSIM_ID ueIf)
{
}

void fn_NetSim_LTENR_GNBPHY_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf)
{
	LTENR_form_prb_list(gnbId, gnbIf);
	LTENR_addStartFrameEvent(gnbId, gnbIf, 0);

	ptrLTENR_ASSOCIATIONINFO info = LTENR_ASSOCIATEINFO_FIND(gnbId, gnbIf, 0, 0);
	while (info)
	{
		LTENR_PHY_ASSOCIATION(gnbId, gnbIf,
							  info->d, info->in,
							  true);
		info = info->next;
	}
}

ptrLTENR_GNBPHY LTENR_GNBPHY_NEW(NETSIM_ID gnbId, NETSIM_ID gnbIf)
{
	ptrLTENR_GNBPHY phy = calloc(1, sizeof * phy);
	phy->spectrumConfig = calloc(1, sizeof * phy->spectrumConfig);
	phy->propagationConfig = calloc(1, sizeof * phy->propagationConfig);
	phy->frameInfo = calloc(1, sizeof * phy->frameInfo);
	LTENR_GNBPHY_SET(gnbId, gnbIf, phy);
	phy->gnbId = gnbId;
	phy->gnbIf = gnbIf;
	return phy;
}

ptrLTENR_UEPHY LTENR_UEPHY_NEW(NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_UEPHY phy = calloc(1, sizeof * phy);
	phy->propagationConfig = calloc(1, sizeof * phy->propagationConfig);
	LTENR_UEPHY_SET(ueId, ueIf, phy);
	phy->ueId = ueId;
	phy->ueIf = ueIf;
	return phy;
}

#pragma endregion

#pragma region PRB
static void LTENR_form_prb_list(NETSIM_ID d, NETSIM_ID in)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(d, in);
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;
	UINT i;
	print_ltenr_log("Forming PRB list for gNB %d:%d -- \n", d, in);
	double prbBandwidth_kHz = sc->prbBandwidth_kHz;
	double guard_kHz = sc->guardBand_kHz;
	print_ltenr_log("\tFlow_MHz = %d\n", sc->Flow_MHZ);
	print_ltenr_log("\tFhigh_MHz = %d\n", sc->Fhigh_MHz);
	print_ltenr_log("\tChannelBandwidth_MHz = %lf\n", sc->channelBandwidth_mHz);
	print_ltenr_log("\tPRBBandwidth_kHz = %lf\n", prbBandwidth_kHz);
	print_ltenr_log("\tGuardBandwidth_kHz = %lf\n", guard_kHz);
	print_ltenr_log("\tPRBId\tFlow\tFhigh\tFcenter\n");

	phy->prbList = calloc(sc->PRBCount, sizeof * phy->prbList);
	for (i = 0; i < sc->PRBCount; i++)
	{
		phy->prbList[i].prbId = i + 1;
		phy->prbList[i].lFrequency_MHz = sc->Flow_MHZ + guard_kHz * 0.001 + prbBandwidth_kHz * i * 0.001;
		phy->prbList[i].uFrequency_MHz = phy->prbList[i].lFrequency_MHz + prbBandwidth_kHz * 0.001;
		phy->prbList[i].centralFrequency_MHz = (phy->prbList[i].lFrequency_MHz + phy->prbList[i].uFrequency_MHz) / 2.0;
		phy->prbList[i].prbBandwidth_MHz = prbBandwidth_kHz * 0.001;
		print_ltenr_log("\t%3d\t%lf\t%lf\t%lf\n",
						i + 1,
						phy->prbList[i].lFrequency_MHz,
						phy->prbList[i].uFrequency_MHz,
						phy->prbList[i].centralFrequency_MHz);
	}
	print_ltenr_log("\n\n");
}
#pragma endregion

#pragma region FRAME
static void LTENR_addStartFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIf;
	pevent.nProtocolId = MAC_PROTOCOL_LTE_NR;
	pevent.nSubEventType = LTENR_SUBEVENT_PHY_STARTFRAME;
	fnpAddEvent(&pevent);
}

static void LTENR_resetFrame(ptrLTENR_GNBPHY phy)
{
	ptrLTENR_FRAMEINFO info = phy->frameInfo;
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;

	info->frameId++;
	info->frameStartTime = pstruEventDetails->dEventTime;
	info->frameEndTime = pstruEventDetails->dEventTime + sc->frameDuration;

	//reset slot
	info->slotEndTime = 0;
	info->slotId = 0;
	info->slotStartTime = 0;

	//reset subframe
	info->subFrameEndTime = 0;
	info->subFrameId = 0;
	info->subFrameStartTime = 0;
}

void LTENR_handleStartFrameEvent()
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	
	LTENR_resetFrame(phy);
	print_ltenr_log("Starting new frame for gNB %d:%d\n", gnbId, gnbIf);
	print_ltenr_log("\tFrame Id = %d\n", phy->frameInfo->frameId);
	print_ltenr_log("\tFrame start time (us) = %lf\n", phy->frameInfo->frameStartTime);
	print_ltenr_log("\tFrame end time (us) = %lf\n", phy->frameInfo->frameEndTime);

	LTENR_addStartFrameEvent(gnbId, gnbIf,
							 phy->frameInfo->frameEndTime);

	LTENR_addStartSubFrameEvent(gnbId, gnbIf,
								phy->frameInfo->frameStartTime);
}
#pragma endregion

#pragma region SUBFRAME
static void LTENR_addStartSubFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIf;
	pevent.nProtocolId = MAC_PROTOCOL_LTE_NR;
	pevent.nSubEventType = LTENR_SUBEVENT_PHY_STARTSUBFRAME;
	fnpAddEvent(&pevent);
}

static void LTENR_resetSubFrame(ptrLTENR_GNBPHY phy)
{
	ptrLTENR_FRAMEINFO info = phy->frameInfo;
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;

	//reset slot
	info->slotEndTime = 0;
	info->slotId = 0;
	info->slotStartTime = 0;

	//reset subframe
	info->subFrameStartTime = pstruEventDetails->dEventTime;
	info->subFrameEndTime = pstruEventDetails->dEventTime + sc->subFrameDuration;
	info->subFrameId++;
	
}

void LTENR_handleStartSubFrameEvent()
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);

	LTENR_resetSubFrame(phy);
	print_ltenr_log("Starting new sub frame for gNB %d:%d\n", gnbId, gnbIf);
	print_ltenr_log("\tFrame Id = %d\n", phy->frameInfo->frameId);
	print_ltenr_log("\tSubFrame Id = %d\n", phy->frameInfo->subFrameId);
	print_ltenr_log("\tSubFrame start time (us) = %lf\n", phy->frameInfo->subFrameStartTime);
	print_ltenr_log("\tSubFrame end time (us) = %lf\n", phy->frameInfo->subFrameEndTime);

	if (phy->frameInfo->subFrameId != phy->spectrumConfig->subFramePerFrame)
		LTENR_addStartSubFrameEvent(gnbId, gnbIf,
									phy->frameInfo->subFrameEndTime);

	LTENR_addStartSlotEvent(gnbId, gnbIf,
								phy->frameInfo->subFrameStartTime);
}
#pragma endregion

#pragma region SLOT
static void LTENR_addStartSlotEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIf;
	pevent.nProtocolId = MAC_PROTOCOL_LTE_NR;
	pevent.nSubEventType = LTENR_SUBEVENT_PHY_STARTSLOT;
	fnpAddEvent(&pevent);
}

static void LTENR_resetSlot(ptrLTENR_GNBPHY phy)
{
	ptrLTENR_FRAMEINFO info = phy->frameInfo;
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;

	//reset slot
	info->slotId++;
	info->slotStartTime = pstruEventDetails->dEventTime;
	info->slotEndTime = pstruEventDetails->dEventTime + sc->slotDuration_us;

	if (info->prevSlotType == SLOT_UPLINK)
		info->slotType = SLOT_DOWNLINK;
	else if (info->prevSlotType == SLOT_DOWNLINK)
		info->slotType = SLOT_UPLINK;
	else
		info->slotType = SLOT_DOWNLINK;
}

void LTENR_handleStartSlotEvent()
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);

	LTENR_resetSlot(phy);
	print_ltenr_log("Starting new slot for gNB %d:%d\n", gnbId, gnbIf);
	print_ltenr_log("\tFrame Id = %d\n", phy->frameInfo->frameId);
	print_ltenr_log("\tSubFrame Id = %d\n", phy->frameInfo->subFrameId);
	print_ltenr_log("\tSlot Id = %d\n", phy->frameInfo->slotId);
	print_ltenr_log("\tSlot start time (us) = %lf\n", phy->frameInfo->slotStartTime);
	print_ltenr_log("\tslot end time (us) = %lf\n", phy->frameInfo->slotEndTime);
	print_ltenr_log("\tSlot type = %s\n", strLTENR_SLOTTYPE[phy->frameInfo->slotType]);

	if (phy->frameInfo->slotId != phy->spectrumConfig->slotPerSubframe)
		LTENR_addStartSlotEvent(gnbId, gnbIf,
								phy->frameInfo->slotEndTime);

	LTENR_NotifyMACForStartingSlot();

	phy->frameInfo->prevSlotType = phy->frameInfo->slotType;
}
#pragma endregion

#pragma region PHY_PROPAGATION_INTERFACE
static void LTENR_PHY_initPropagationInfo(ptrLTENR_GNBPHY phy,
										  ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo)
{
	ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(assocInfo->ueId, assocInfo->ueIf);
	ptrLTENR_PROPAGATIONINFO info = calloc(1, sizeof * info);
	assocInfo->propagationInfo = info;

	info->gnbId = phy->gnbId;
	info->gnbIf = phy->gnbIf;
	memcpy(&info->gnbPos, DEVICE_POSITION(phy->gnbId), sizeof info->gnbPos);
	memcpy(&info->uePos, DEVICE_POSITION(assocInfo->ueId), sizeof info->uePos);
	info->ueId = assocInfo->ueId;
	info->ueIf = assocInfo->ueIf;
	info->bandwidth_MHz = phy->spectrumConfig->channelBandwidth_mHz;
	info->centralFrequency_MHz = (phy->spectrumConfig->Fhigh_MHz + phy->spectrumConfig->Flow_MHZ) / 2;
	info->txPower_dbm = phy->propagationConfig->txPower_dbm;

	info->propagationConfig = phy->propagationConfig;
	info->propagationConfig->UE_height = uePhy->propagationConfig->UE_height;
}

static void LTENR_PHY_updatePropagationInfo(ptrLTENR_GNBPHY phy,
											ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo)
{
	ptrLTENR_PROPAGATIONINFO info = assocInfo->propagationInfo;
	memcpy(&info->gnbPos, DEVICE_POSITION(phy->gnbId), sizeof info->gnbPos);
	memcpy(&info->uePos, DEVICE_POSITION(assocInfo->ueId), sizeof info->uePos);
}

#define BOLTZMANN 1.38064852e-23 //m2kgs-2K-1
#define TEMPERATURE 300 //kelvin
static double LTENR_PHY_calculateThermalNoise(double bandwidth)
{
	double noise = BOLTZMANN * TEMPERATURE * bandwidth * 1000000; //in W
	noise *= 1000; // in mW
	double noise_dbm = MW_TO_DBM(noise);
	return noise_dbm;
}

static void LTENR_PHY_calculateSNR(ptrLTENR_PROPAGATIONINFO info)
{
	double p = DBM_TO_MW(info->rxPower_dbm);
	double n = DBM_TO_MW(info->thermalNoise);
	info->EB_by_N0 = p / n;
	info->SNR_db = MW_TO_DBM(info->EB_by_N0);
}

static void LTENR_PHY_calculateSpectralEfficiency(ptrLTENR_GNBPHY phy,
												  ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo)
{
	if (!assocInfo->propagationInfo)
		LTENR_PHY_initPropagationInfo(phy, assocInfo);
	else
		LTENR_PHY_updatePropagationInfo(phy, assocInfo);

	ptrLTENR_PROPAGATIONINFO info = assocInfo->propagationInfo;

	//Call propagation model
	LTENR_Propagation_TotalLoss(info);

	info->rxPower_dbm = info->txPower_dbm - info->dTotalLoss;
	info->thermalNoise = LTENR_PHY_calculateThermalNoise(info->bandwidth_MHz);
	LTENR_PHY_calculateSNR(info);
	info->spectralEfficiency = log2(1 + info->EB_by_N0);

	print_ltenr_log("\tThermal Noise = %lfdb\n", info->thermalNoise);
	print_ltenr_log("\tSignal to Noise Ratio (SNR) = %lfdb\n", info->SNR_db);
	print_ltenr_log("\tSpectral Efficiency = %lf\n", info->spectralEfficiency);
	print_ltenr_log("\n");
}

static void LTENR_PHY_setAMCInfo(ptrLTENR_GNBPHY phy, ptrLTENR_ASSOCIATEDUEPHYINFO info)
{
	//Downlink
	info->downlinkAMCInfo->SpectralEfficiency = info->propagationInfo->spectralEfficiency;
	info->downlinkAMCInfo->cqiTable = LTENR_GetCQITableFromSpectralEfficiency(phy->CSIReportConfig.cqiTable,
																		  info->downlinkAMCInfo->SpectralEfficiency);
	info->downlinkAMCInfo->mcsTable = LTENR_GetMCSIndexTableFromSpectralEfficiency(phy->PDSCHConfig.mcsTable,
																				   info->downlinkAMCInfo->cqiTable.efficiency);

	print_ltenr_log("\tAMC info between gNB %d:%d and UE %d:%d for downlink-\n",
					phy->gnbId, phy->gnbIf,
					info->ueId, info->ueIf);
	print_ltenr_log("\t\tSpectral Efficiency = %lf\n", info->downlinkAMCInfo->SpectralEfficiency);
	print_ltenr_log("\t\tCQI Table\n");
	print_ltenr_log("\t\t\t%d\t%s\t%d\t%lf\n",
					info->downlinkAMCInfo->cqiTable.CQIIndex,
					strPHY_MODULATION[info->downlinkAMCInfo->cqiTable.modulation],
					info->downlinkAMCInfo->cqiTable.codeRate,
					info->downlinkAMCInfo->cqiTable.efficiency);
	print_ltenr_log("\t\tMCS Table\n");
	print_ltenr_log("\t\t\t%d\t%s\t%d\t%lf\t%lf\n",
					info->downlinkAMCInfo->mcsTable.mcsIndex,
					strPHY_MODULATION[info->downlinkAMCInfo->mcsTable.modulation],
					info->downlinkAMCInfo->mcsTable.modulationOrder,
					info->downlinkAMCInfo->mcsTable.codeRate,
					info->downlinkAMCInfo->mcsTable.spectralEfficiency);

	//Uplink
	info->uplinkAMCInfo->SpectralEfficiency = info->propagationInfo->spectralEfficiency;
	info->uplinkAMCInfo->cqiTable = LTENR_GetCQITableFromSpectralEfficiency(phy->CSIReportConfig.cqiTable,
																			info->uplinkAMCInfo->SpectralEfficiency);
	info->uplinkAMCInfo->mcsTable = LTENR_GetMCSIndexTableFromSpectralEfficiency(phy->PUSCHConfig.mcsTable,
																				 info->uplinkAMCInfo->cqiTable.efficiency);
	print_ltenr_log("\tAMC info between gNB %d:%d and UE %d:%d for uplink-\n",
					phy->gnbId, phy->gnbIf,
					info->ueId, info->ueIf);
	print_ltenr_log("\t\tSpectral Efficiency = %lf\n", info->uplinkAMCInfo->SpectralEfficiency);
	print_ltenr_log("\t\tCQI Table\n");
	print_ltenr_log("\t\t\t%d\t%s\t%d\t%lf\n",
					info->uplinkAMCInfo->cqiTable.CQIIndex,
					strPHY_MODULATION[info->uplinkAMCInfo->cqiTable.modulation],
					info->uplinkAMCInfo->cqiTable.codeRate,
					info->uplinkAMCInfo->cqiTable.efficiency);
	print_ltenr_log("\t\tMCS Table\n");
	print_ltenr_log("\t\t\t%d\t%s\t%d\t%lf\t%lf\n",
					info->uplinkAMCInfo->mcsTable.mcsIndex,
					strPHY_MODULATION[info->uplinkAMCInfo->mcsTable.modulation],
					info->uplinkAMCInfo->mcsTable.modulationOrder,
					info->uplinkAMCInfo->mcsTable.codeRate,
					info->uplinkAMCInfo->mcsTable.spectralEfficiency);
}
#pragma endregion

#pragma region PHY_AMCINFO
static void LTENR_PHY_initAMCInfo(ptrLTENR_GNBPHY phy, ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo)
{
	assocInfo->downlinkAMCInfo = calloc(1, sizeof * assocInfo->downlinkAMCInfo);
	assocInfo->uplinkAMCInfo = calloc(1, sizeof * assocInfo->uplinkAMCInfo);

	LTENR_PHY_calculateSpectralEfficiency(phy, assocInfo);
	LTENR_PHY_setAMCInfo(phy, assocInfo);
}
#pragma endregion

#pragma region PHY_ASSOCIATION
static void LTENR_PHY_associateUE(NETSIM_ID gnbId, NETSIM_ID gnbIf,
								  NETSIM_ID ueId, NETSIM_ID ueIf)
{
	print_ltenr_log("PHY-- UE %d:%d is associated with gNB %d:%d\n",
					ueId, ueIf, gnbId, gnbIf);

	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_ALLOC();
	LTENR_ASSOCIATEDUEPHYINFO_ADD(phy, info);

	info->ueId = ueId;
	info->ueIf = ueIf;

	LTENR_PHY_initAMCInfo(phy, info);

	ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(ueId, ueIf);
	uePhy->gnBId = gnbId;
	uePhy->gnbIf = gnbIf;

	ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gnbId, gnbIf);
	LTENR_EPC_ASSOCIATION(mac->epcId, mac->epcIf,
						  gnbId, gnbIf,
						  ueId, ueIf);
}

static void LTENR_PHY_deassociateUE(NETSIM_ID gnbId, NETSIM_ID gnbIf,
								  NETSIM_ID ueId, NETSIM_ID ueIf)
{
	fnNetSimError("Implement deassociation function for LTENR-PHY\n");
#pragma message(__LOC__"Implement deassociation function for LTENR-PHY")
}

void LTENR_PHY_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
						   NETSIM_ID ueId, NETSIM_ID ueIf,
						   bool isAssociated)
{
	if (isAssociated)
		LTENR_PHY_associateUE(gnbId, gnbIf, ueId, ueIf);
	else
		LTENR_PHY_deassociateUE(gnbId, gnbIf, ueId, ueIf);
}
#pragma endregion

#pragma region PHY_API
double LTENR_PHY_GetSlotEndTime(NETSIM_ID d, NETSIM_ID in)
{
	if (isGNB(d, in))
	{
		ptrLTENR_FRAMEINFO fi = ((ptrLTENR_GNBPHY)LTENR_GNBPHY_GET(d, in))->frameInfo;
		return fi->slotEndTime;
	}
	else
	{
		ptrLTENR_UEPHY phy = LTENR_UEPHY_GET(d, in);
		return LTENR_PHY_GetSlotEndTime(phy->gnBId, phy->gnbIf);
	}
}
#pragma endregion
