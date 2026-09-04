/******************************************************************************
 * @file    tiremo_jobs.h
 * @brief   Tiremo Jobs: notify → start-next → SUCCEEDED / FIRMWARE_UPDATE.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_JOBS_H
#define TIREMO_JOBS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Subscribe jobs/notify and drain any job already queued at boot.
 *
 * Same shape as the production Cloud_first_checkup jobs probe: publish
 * start-next once, then leave jobs/notify standing for the ready loop.
 *
 * @param[in] pThingName  Same Thing name as shadow topics.
 */
bool TIREMO_Jobs_Init(const char *pThingName);

/**
 * @brief Called from Shadow_Poll when an inbound topic is under /jobs/.
 *
 * A full execution document is completed immediately. A summary notify
 * (QUEUED jobId only) sets a flag; TIREMO_Jobs_Poll() claims it.
 */
void TIREMO_Jobs_OnMessage(const char *pTopic,
                           const char *pPayload,
                           uint32_t payloadLen);

/**
 * @brief Claim a job flagged by OnMessage (start-next, then update).
 *
 * Call from the ready loop after TIREMO_Shadow_Poll() so start-next is
 * not nested inside an inbound WaitMessage.
 */
void TIREMO_Jobs_Poll(void);

/**
 * @brief Publish a terminal result for a job the board is finishing
 *        outside the notify/start-next flow.
 *
 * Used after an OTA reboot: the freshly-swapped image reports its job
 * SUCCEEDED (or a rolled-back boot reports it FAILED) once MQTT is up.
 * TIREMO_Jobs_Init() must have run first (it sets the Thing name).
 *
 * @param[in] pJobId   Job id from TIREMO_OTA_PendingJobId().
 * @param[in] ok       true -> SUCCEEDED (with firmware_version), false -> FAILED.
 * @param[in] pDetail  statusDetails.reason when @p ok is false; ignored otherwise.
 */
void TIREMO_Jobs_ReportResult(const char *pJobId, bool ok, const char *pDetail);

/**
 * @brief Same as TIREMO_Jobs_ReportResult(), usable before Init().
 *
 * Only needs the Thing name and a connected MQTT session -- for closing
 * out a job that spanned a reboot *before* Init()'s own boot start-next
 * probe gets a chance to re-claim (and re-apply) that same still
 * IN_PROGRESS job.
 *
 * @param[in] pThingName  Thing name (same one Init() would take).
 */
void TIREMO_Jobs_ReportResultDirect(const char *pThingName, const char *pJobId,
                                    bool ok, const char *pDetail);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_JOBS_H */
