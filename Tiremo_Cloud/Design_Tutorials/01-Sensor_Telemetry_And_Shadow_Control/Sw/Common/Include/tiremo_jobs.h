/******************************************************************************
 * @file    tiremo_jobs.h
 * @brief   Tiremo Jobs: notify → start-next → SUCCEEDED / REJECTED.
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

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_JOBS_H */
