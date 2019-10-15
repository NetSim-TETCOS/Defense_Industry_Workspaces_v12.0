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
* Author:    Kumar Gaurav		                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "LTENR_MAC.h"
#include "LTE_NR.h"

static ptrLTENR_UESCHEDULARINFO LTENR_PRB_SortingofRank(ptrLTENR_UESCHEDULARINFO list)
{
	ptrLTENR_UESCHEDULARINFO head = NULL;
	ptrLTENR_UESCHEDULARINFO tail = NULL;
	ptrLTENR_UESCHEDULARINFO t = NULL;
	while (list)
	{
		if (!head)
		{
			//first member
			head = list;
			tail = list;
			list = list->next;
			head->next = NULL;
			continue;
		}

		if (list->rank >= head->rank)
		{
			// Rank is less and head rank. So place at head
			t = list->next;
			list->next = head;
			head = list;
			list = t;
			continue;
		}

		if (list->rank <= tail->rank)
		{
			// Rank is more than tail rank, so place as tail
			t = list->next;
			tail->next = list;
			tail = list;
			list = t;
			tail->next = NULL;
			continue;
		}

		//Middle
		t = list->next;
		ptrLTENR_UESCHEDULARINFO info = head;
		while (info)
		{
			if (info->next->rank < list->rank)
			{
				list->next = info->next;
				info->next = list;
				break;
			}
			info = info->next;
		}
		list = t;
	}
	return head;
}

static void LTENR_PRB_SetRank(ptrLTENR_UESCHEDULARINFO list, ptrLTENR_SCHEDULARINFO schedularInfo) {
	double sigmaofBitperPRB = 0;
	ptrLTENR_UESCHEDULARINFO current = list;
	if (schedularInfo->schedulingType == LTENR_MAC_SCHEDULING_ROUNDROBIN) {//RR
		while (list) {
			list->rank = 1.0;
			list->initRank = 1.0;
			list = list->next;
		}
	}

	else if (schedularInfo->schedulingType == LTENR_MAC_SCHEDULING_PROPORTIONALFAIR) {//fair protitonal
		while (current) {
			sigmaofBitperPRB += current->bitsPerPRB;
			current = current->next;
		}
		while (list) {
			list->rank = (double)(list->bitsPerPRB / sigmaofBitperPRB);
			list->initRank = list->rank;
			list = list->next;
		}
	}

	else if (schedularInfo->schedulingType == LTENR_MAC_SCHEDULING_MAXTHROUGHTPUT) { //MAX throughput
		/*while (current) {
			sigmaofBitperPRB += current->bitsPerPRB;
			current = current->next;
		}
		double temp = 0, rankValue;
		while (list) {
			rankValue = (double)(list->bitsPerPRB / sigmaofBitperPRB);
			if (rankValue > temp) {
				current = list;
				temp = rankValue;
			}
			list = list->next;
		}
		current->rank = temp;
		current->initRank = current->rank;*/

		while (current) {
			sigmaofBitperPRB += current->bitsPerPRB;
			current = current->next;
		}
		while (list) {
			list->rank = (double)(list->bitsPerPRB / sigmaofBitperPRB);
			list->initRank = list->rank;
			list = list->next;
		}
	}
}

static void LTENR_PRB_ResetAllocationPRBCount(ptrLTENR_UESCHEDULARINFO list) {
	while (list) {
		list->allocatedPRBCount = 0;
		list = list->next;
	}
}

static void LTENR_PRB_UpdateRank(ptrLTENR_UESCHEDULARINFO list) {
	while (list) {
		list->rank = (list->rank - (list->allocatedPRBCount / initTotalPRBAvailable)) + list->initRank;
		list = list->next;
	}
}

static void LTENR_PRB_AllocatePRB(ptrLTENR_SCHEDULARINFO schedularInfo, ptrLTENR_UESCHEDULARINFO* curr, UINT totalPRBAvailable) {
	ptrLTENR_UESCHEDULARINFO current = *curr;
	if (!schedularInfo->isPRBRankInit) {//first time to set rank
		LTENR_PRB_SetRank(schedularInfo->downlinkInfo, schedularInfo);
		LTENR_PRB_SetRank(schedularInfo->uplinkInfo, schedularInfo);
	}
	schedularInfo->isPRBRankInit = TRUE;
	current = LTENR_PRB_SortingofRank(current);
	ptrLTENR_UESCHEDULARINFO current1 = current;
	*curr = current;

	while (current && totalPRBAvailable > 0) {
		UINT PRBallocation = (UINT)ceil(current->bufferSize * 8.0 / current->bitsPerPRB);

		if (PRBallocation < totalPRBAvailable) {
			current->allocatedPRBCount += PRBallocation;
			totalPRBAvailable -= current->allocatedPRBCount;
		}

		else {
			current->allocatedPRBCount += totalPRBAvailable;
			totalPRBAvailable = 0;
		}
#ifdef LTENR_PRB_LOG
		if (schedularInfo->schedulingType != LTENR_MAC_SCHEDULING_MAXTHROUGHTPUT) {
			print_ltenr_PRB_log("UE_ID = %d, bitsPerPRB=%d, BufferSize=%d, Rank=%lf, allocate_PRB = %d, New Rank=%lf\n", current->ueId, current->bitsPerPRB, current->bufferSize,
				current->rank, current->allocatedPRBCount, ((current->rank - (current->allocatedPRBCount / initTotalPRBAvailable)) + current->initRank));
		}
		else {
			print_ltenr_PRB_log("UE_ID = %d, bitsPerPRB=%d, BufferSize=%d, Rank=%lf, allocate_PRB = %d\n", current->ueId, current->bitsPerPRB, current->bufferSize,
				current->rank, current->allocatedPRBCount);
		}
#endif
		current = current->next;
	}
	//update rank after allocation
	if (schedularInfo->schedulingType != LTENR_MAC_SCHEDULING_MAXTHROUGHTPUT)
		LTENR_PRB_UpdateRank(current1);
}

static int sumofAllocation(ptrLTENR_UESCHEDULARINFO list) {
	int totalPRBAllocated = 0;
	while (list) {
		totalPRBAllocated += list->allocatedPRBCount;
		list = list->next;
	}
	return totalPRBAllocated;
}

void LTENR_PRB_Schedular(ptrLTENR_SCHEDULARINFO schedularInfo) {

	UINT totalPRBAvailable = schedularInfo->PRBCount * schedularInfo->symbolPerSlot;
	initTotalPRBAvailable = totalPRBAvailable;
	int remainingPRBColumn = 0;

	if (schedularInfo->slotType == SLOT_UPLINK) {
		if (!schedularInfo->uplinkInfo)
			fnNetSimError("Uplink information for scheduling is not present for devide id=%d\n", schedularInfo->gnbId);
		else {
			totalPRBAvailable -= (UINT)ceil(totalPRBAvailable * schedularInfo->OH_UL);//allocate PRB for overhead 
			LTENR_PRB_ResetAllocationPRBCount(schedularInfo->uplinkInfo);
#ifdef LTENR_PRB_LOG
			print_ltenr_PRB_log("GNB ID = %d, SLOT = UPLINK, TOTAL_PRB = %d\n", schedularInfo->gnbId,totalPRBAvailable);
#endif
			LTENR_PRB_AllocatePRB(schedularInfo, &schedularInfo->uplinkInfo, totalPRBAvailable);
			
			LTENR_PRB_ResetAllocationPRBCount(schedularInfo->downlinkInfo);
			//if any resource remaining then it allocate for downlink
			remainingPRBColumn = (int)((totalPRBAvailable - sumofAllocation(schedularInfo->uplinkInfo)) / schedularInfo->PRBCount);
			if (remainingPRBColumn > 0) {
				totalPRBAvailable = (remainingPRBColumn - 1) * schedularInfo->PRBCount;//-1 for transition from uplink to downlink				
				if (totalPRBAvailable > schedularInfo->PRBCount * 2) {
					schedularInfo->slotType = SLOT_MIXED;
					totalPRBAvailable -= (UINT)ceil(totalPRBAvailable * schedularInfo->OH_DL);//allocate PRB for overhead 
#ifdef LTENR_PRB_LOG
					print_ltenr_PRB_log("After UPLINK allocation remaining PRB = %d\n slot type changing to mixed \n", totalPRBAvailable);
#endif
					LTENR_PRB_AllocatePRB(schedularInfo, &schedularInfo->downlinkInfo, totalPRBAvailable);
				}
			}
			print_ltenr_PRB_log("--------------------------------------------------------------\n");
		}
	}

	else if (schedularInfo->slotType == SLOT_DOWNLINK) {
		if (!schedularInfo->downlinkInfo)
			fnNetSimError("Dowblink information for scheduling is not present for devide id=%d\n", schedularInfo->gnbId);
		else {

			totalPRBAvailable -= (UINT)ceil(totalPRBAvailable * schedularInfo->OH_DL);//allocate PRB for overhead 
			LTENR_PRB_ResetAllocationPRBCount(schedularInfo->downlinkInfo);
#ifdef LTENR_PRB_LOG
			print_ltenr_PRB_log("GNB ID = %d, SLOT = DOWNLINK, TOTAL_PRB = %d\n", schedularInfo->gnbId, totalPRBAvailable);
#endif
			LTENR_PRB_AllocatePRB(schedularInfo, &schedularInfo->downlinkInfo, totalPRBAvailable);
			LTENR_PRB_ResetAllocationPRBCount(schedularInfo->uplinkInfo);
			//if any resource remaining then it allocate for uplink
			remainingPRBColumn = (int)((totalPRBAvailable - sumofAllocation(schedularInfo->downlinkInfo)) / schedularInfo->PRBCount);
			if (remainingPRBColumn > 0) {
				totalPRBAvailable = (remainingPRBColumn - 1) * schedularInfo->PRBCount;//-1 for transition from downlink to uplink				
				if (totalPRBAvailable > schedularInfo->PRBCount * 2) {
					schedularInfo->slotType = SLOT_MIXED;
					totalPRBAvailable -= (UINT)ceil(totalPRBAvailable * schedularInfo->OH_UL);//allocate PRB for overhead 
#ifdef LTENR_PRB_LOG
					print_ltenr_PRB_log("After UPLINK allocation remaining PRB = %d\n slot type changing to mixed \n", totalPRBAvailable);
#endif
					LTENR_PRB_AllocatePRB(schedularInfo, &schedularInfo->uplinkInfo, totalPRBAvailable);
				}
			}
			print_ltenr_PRB_log("--------------------------------------------------------------\n");
		}
	}

	else {
		if (!schedularInfo->downlinkInfo)
			fnNetSimError("Dowblink information for scheduling is not present for devide id=%d\n", schedularInfo->gnbId);
		else if (!schedularInfo->uplinkInfo)
			fnNetSimError("Uplink information for scheduling is not present for devide id=%d\n", schedularInfo->gnbId);
		else {
			//first allocating for downlink
			totalPRBAvailable -= (UINT)ceil(totalPRBAvailable * schedularInfo->OH_DL);//allocate PRB for overhead 
			LTENR_PRB_ResetAllocationPRBCount(schedularInfo->downlinkInfo);

			LTENR_PRB_AllocatePRB(schedularInfo, &schedularInfo->downlinkInfo, totalPRBAvailable);
			LTENR_PRB_ResetAllocationPRBCount(schedularInfo->uplinkInfo);
			//if any resource remaining then it allocate for uplink
			remainingPRBColumn = (int)((totalPRBAvailable - sumofAllocation(schedularInfo->downlinkInfo)) / schedularInfo->PRBCount);
			totalPRBAvailable = (remainingPRBColumn - 1) * schedularInfo->PRBCount;//-1 for transition from downlink to uplink

			if (totalPRBAvailable > 0) {
				totalPRBAvailable -= (UINT)ceil(totalPRBAvailable * schedularInfo->OH_UL);//allocate PRB for overhead 
				LTENR_PRB_AllocatePRB(schedularInfo, &schedularInfo->uplinkInfo, totalPRBAvailable);
			}
		}
	}
}