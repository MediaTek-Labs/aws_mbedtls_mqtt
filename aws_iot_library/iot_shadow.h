/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
#ifndef AWS_IOT_SDK_SRC_IOT_SHADOW_H_
#define AWS_IOT_SDK_SRC_IOT_SHADOW_H_

#include "mqtt_interface.h"
#include "iot_shadow_json_data.h"

typedef struct {
	char *pUniqueThingId;
	char *pHost;
	int port;
	char *pRootCA;
	char *pClientCRT;
	char *pClientKey;
} ShadowParameters_t;

// connection management related functions
IoT_Error_t iot_shadow_init(MQTTClient_t *pClient);
IoT_Error_t iot_shadow_connect(MQTTClient_t *pClient, ShadowParameters_t *pParams);
IoT_Error_t iot_shadow_yield(MQTTClient_t *pClient, int timeout);
IoT_Error_t iot_shadow_disconnect(MQTTClient_t *pClient);

// Publish related functions
typedef enum {
	SHADOW_ACK_TIMEOUT, SHADOW_ACK_REJECTED, SHADOW_ACK_ACCEPTED
} Shadow_Ack_Status_t;

typedef enum {
	SHADOW_GET, SHADOW_UPDATE, SHADOW_DELETE
} ShadowActions_t;

typedef void (*fpActionCallback_t)(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
		const char *pReceivedJsonDocument, void *pContextData);

IoT_Error_t iot_shadow_update(MQTTClient_t *pClient, const char *pThingName, char *pJsonString,
		fpActionCallback_t callback, void *pContextData, uint8_t timeout_seconds, bool isPersistentSubscribe);

IoT_Error_t iot_shadow_get(MQTTClient_t *pClient, const char *pThingName, fpActionCallback_t callback,
		void *pContextData, uint8_t timeout_seconds, bool isPersistentSubscribe);

IoT_Error_t iot_shadow_delete(MQTTClient_t *pClient, const char *pThingName, fpActionCallback_t callback,
		void *pContextData, uint8_t timeout_seconds, bool isPersistentSubscriptions);

// Delta function
IoT_Error_t iot_shadow_register_delta(MQTTClient_t *pClient, jsonStruct_t *pStruct);
void iot_shadow_reset_last_received_version(void);
uint32_t iot_shadow_get_last_received_version(void);
void iot_shadow_enable_discard_old_delta_msgs(void);
void iot_shadow_disable_discard_old_delta_msgs(void);

#endif //AWS_IOT_SDK_SRC_IOT_SHADOW_H_
