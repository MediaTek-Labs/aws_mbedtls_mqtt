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

#define MAX_TOKEN_EXPECTED 100
#define SHADOW_MAX_SIZE_OF_RX_BUFFER 512
#define SHADOW_MAX_SIZE_OF_TX_BUFFER 512
#define UNIQUE_CLIENT_TOKEN "c-sdk-client#1"

typedef struct {
	char *pUniqueThingId;
	char *pHost;
	int port;
	char *pRootCA;
	char *pClientCRT;
	char *pClientKey;
} ShadowParameters_t;

typedef enum{
	SHADOW_JSON_INT32,
	SHADOW_JSON_INT16,
	SHADOW_JSON_INT8,
	SHADOW_JSON_UINT32,
	SHADOW_JSON_UINT16,
	SHADOW_JSON_UINT8,
	SHADOW_JSON_FLOAT,
	SHADOW_JSON_DOUBLE,
	SHADOW_JSON_BOOL,
	SHADOW_JSON_STRING,
	SHADOW_JSON_OBJECT
}JsonPrimitiveType;

typedef int32_t UpdateHandler_t;

typedef struct primitiveJson primitiveJson_t;
typedef void (*genericFunctionPointer_t)(const char *, uint32_t , primitiveJson_t *);

struct primitiveJson{
	const char *pKey;
	void *pData;
	JsonPrimitiveType type;
	genericFunctionPointer_t cb;
};

typedef enum{
	SHADOW_ACK_TIMEOUT,
	SHADOW_ACK_REJECTED,
	SHADOW_ACK_ACCEPTED
}Shadow_Ack_Status_t;

typedef void (*pStatusCallback_t)(void *, Shadow_Ack_Status_t);

IoT_Error_t iot_shadow_init(MQTTClient_t *pClient);
IoT_Error_t iot_shadow_connect(MQTTClient_t *pClient, ShadowParameters_t *pParams);
/*
 * This will be blocking, simple send and wait for response
 * - pJsonString and thing name string memory should not be destroyed till a status callback is received, if retry feature is implemented then we can use the data to send it again.
 * - this update is a blocking publish, but the callback happens only in yield. This will not wait for the message to come back in accepted or rejected topic
 */
IoT_Error_t iot_shadow_update(MQTTClient_t *pClient, char *pJsonString, pStatusCallback_t callback, void *pContextData, uint8_t timeout_seconds);
IoT_Error_t iot_shadow_register_delta(MQTTClient_t *pClient, primitiveJson_t *pStruct);

typedef void (*pGetJsonDocumentCallback_t)(const char *pEntireJsonDocument, Shadow_Ack_Status_t status);
IoT_Error_t iot_shadow_get_document(MQTTClient_t *pClient, pGetJsonDocumentCallback_t callback, uint32_t timeout_seconds);
IoT_Error_t iot_shadow_yield(MQTTClient_t *pClient, int timeout);
IoT_Error_t iot_shadow_disconnect(MQTTClient_t *pClient);

/*
 * This function will return the ShadowTxBuffer allocated, so dont use it from multiple threads to publish. Always have the publishing of final JSON document from one thread
 */
char* iot_shadow_init_json_document(void);
IoT_Error_t iot_shadow_add_reported(char *pJsonDocument, uint8_t count, ...);
IoT_Error_t iot_shadow_add_desired(char *pJsonDocument, uint8_t count, ...);
void iot_finalize_json_document(char *pJsonDocument);

#endif //AWS_IOT_SDK_SRC_IOT_SHADOW_H_
