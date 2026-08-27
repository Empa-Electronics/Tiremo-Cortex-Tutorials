/******************************************************************************
 * @file    tiremo_provision.h
 * @brief   AWS IoT Fleet Provisioning by Claim (ESP32-only tutorial).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_PROVISION_H
#define TIREMO_PROVISION_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

/**
 * @brief Check the ESP32 SYSMFG marker against this device ID.
 */
bool TIREMO_Provision_IsDone(const char *pDeviceId);

/**
 * @brief Run CreateKeysAndCertificate -> RegisterThing, install the issued
 *        certificate/key, write the marker, and reconnect permanently.
 */
bool TIREMO_Provision_Run(const char *pDeviceId, const char *pClientId);

/**
 * @brief Erase the issued MQTT cert/key and the provisioned marker.
 *
 * Leaves the board factory-fresh for a new claim exchange. Does not
 * restore WiFi or run AT+RESTORE -- this tutorial only needs the AWS
 * identity slots wiped.
 */
bool TIREMO_Provision_FactoryReset(void);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_PROVISION_H */
