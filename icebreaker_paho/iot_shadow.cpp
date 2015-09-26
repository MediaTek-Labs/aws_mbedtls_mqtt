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

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "iot_shadow.h"
#include "iot_error.h"
#include "iot_log.h"
#include "json_utils.h"

//@TODO: This needs to be moved into utils
#include "timer_interface.h"

char shadowRxBuffer[SHADOW_MAX_SIZE_OF_RX_BUFFER];
static jsmn_parser shadowJsonParser;
static jsmntok_t jsonTokenStruct[MAX_TOKEN_EXPECTED];
typedef struct {
	const char *pKey;
	void *pStruct;
	genericFunctionPointer_t FunctionPointer;
} JsonTokenTable_t;

static JsonTokenTable_t tokenTable[MAX_TOKEN_EXPECTED];
static uint32_t tokenTableIndex = 0;
static bool deltaTopicSubscribedFlag = false;
static uint32_t clientTokenNum = 0;
static bool isMyThingNameAcceptedTopicSubscribed = false;
static bool isMyThingNameRejectedTopicSubscribed = false;
static bool isMyThingNameGetAcceptedTopicSubscribed = false;
static bool isMyThingNameGetRejectedTopicSubscribed = false;

#define MAX_SIZE_OF_UNIQUE_CLIENT_ID_BYTES 80
char uniqueClientID[MAX_SIZE_OF_UNIQUE_CLIENT_ID_BYTES];
#define MAX_NUM_OF_CLIENT_RESPONSES 10
#define MAX_SIZE_CLIENT_ID_WITH_SEQUENCE MAX_SIZE_OF_UNIQUE_CLIENT_ID_BYTES + 20
#define MAX_SIZE_OF_THING_NAME 20
// Assuming a byte is 8 bits
#define MAX_SHADOW_TOPIC_LENGTH_WITHOUT_THINGNAME 60
#define MAX_SHADOW_TOPIC_LENGTH_BYTES MAX_SHADOW_TOPIC_LENGTH_WITHOUT_THINGNAME + MAX_SIZE_OF_THING_NAME

typedef struct {
	char clientTokenID[MAX_SIZE_CLIENT_ID_WITH_SEQUENCE];
	pStatusCallback_t callback;
	void *pContextData;
	bool isFree;
	Timer timer;
	char ThingName[MAX_SIZE_OF_THING_NAME];
} AckClientTokenBased_t;

AckClientTokenBased_t expectedAckTable[MAX_NUM_OF_CLIENT_RESPONSES];

static void InitAckTableEntry() {
	uint8_t i;
	for (i = 0; i < MAX_NUM_OF_CLIENT_RESPONSES; i++) {
		expectedAckTable[i].isFree = true;
	}
}

// Callbacks
static int32_t updateStatusCallback(MQTTCallbackParams params);

static IoT_Error_t UpdateValueIfNoObject(const char *pJsonString, primitiveJson_t *pDataStruct, jsmntok_t token) {
	IoT_Error_t ret_val = NONE_ERROR;
	if (pDataStruct->type == SHADOW_JSON_BOOL) {
		ret_val = parseBooleanValue((bool*)pDataStruct->pData, pJsonString, &token);
	} else if (pDataStruct->type == SHADOW_JSON_INT32) {
		ret_val = parseInteger32Value((int32_t*)pDataStruct->pData, pJsonString, &token);
	} else if (pDataStruct->type == SHADOW_JSON_INT16) {
		ret_val = parseInteger16Value((int16_t*)pDataStruct->pData, pJsonString, &token);
	} else if (pDataStruct->type == SHADOW_JSON_INT8) {
		ret_val = parseInteger8Value((int8_t*)pDataStruct->pData, pJsonString, &token);
	} else if (pDataStruct->type == SHADOW_JSON_UINT32) {
		ret_val = parseUnsignedInteger32Value((uint32_t*)pDataStruct->pData, pJsonString, &token);
	} else if (pDataStruct->type == SHADOW_JSON_UINT16) {
		ret_val = parseUnsignedInteger16Value((uint16_t*)pDataStruct->pData, pJsonString, &token);
	} else if (pDataStruct->type == SHADOW_JSON_UINT8) {
		ret_val = parseUnsignedInteger8Value((uint8_t*)pDataStruct->pData, pJsonString, &token);
	} else if (pDataStruct->type == SHADOW_JSON_FLOAT) {
		ret_val = parseFloatValue((float*)pDataStruct->pData, pJsonString, &token);
	} else if (pDataStruct->type == SHADOW_JSON_DOUBLE) {
		ret_val = parseDoubleValue((double*)pDataStruct->pData, pJsonString, &token);
	}

	return ret_val;
}

static int32_t shadow_delta_callback(MQTTCallbackParams params) {

	int32_t tokenCount;
	int32_t i, j;
	jsmn_init(&shadowJsonParser);

//	DEBUG("shadow_delta_callback");

	if (params.MessageParams.PayloadLen > SHADOW_MAX_SIZE_OF_RX_BUFFER) {
		return GENERIC_ERROR;
	}

	memcpy(shadowRxBuffer, params.MessageParams.pPayload, params.MessageParams.PayloadLen);
	shadowRxBuffer[params.MessageParams.PayloadLen] = '\0';	// jsmn_parse relies on a string

//	DEBUG("Input JSON: %s\n", shadowRxBuffer);

	tokenCount = jsmn_parse(&shadowJsonParser, shadowRxBuffer, strlen(shadowRxBuffer), jsonTokenStruct,
			sizeof(jsonTokenStruct) / sizeof(jsonTokenStruct[0]));
//	DEBUG("total tokens %d", tokenCount);

	if (tokenCount < 0) {
		WARN("Failed to parse JSON: %d\n", tokenCount);
		return GENERIC_ERROR;
	}

	/* Assume the top-level element is an object */
	if (tokenCount < 1 || jsonTokenStruct[0].type != JSMN_OBJECT) {
		return GENERIC_ERROR;
	}
	i = 0;
//	DEBUG("JSON Token[%d] : start: %d end %d size %d type %d\n", 0, jsonTokenStruct[i].start, jsonTokenStruct[i].end, jsonTokenStruct[i].size, jsonTokenStruct[i].type);
	// TODO: make the search faster
	for (i = 1; i < tokenCount; i++) {
//		DEBUG("JSON Token[%d] : start: %d end %d size %d type %d\n", i, jsonTokenStruct[i].start, jsonTokenStruct[i].end, jsonTokenStruct[i].size, jsonTokenStruct[i].type);
//		DEBUG("token: %.*s", jsonTokenStruct[i].end - jsonTokenStruct[i].start, shadowRxBuffer+jsonTokenStruct[i].start);
		for (j = 0; j < tokenTableIndex; j++) {
			if (jsoneq(shadowRxBuffer, &jsonTokenStruct[i], tokenTable[j].pKey) == 0) {
				jsmntok_t keyToken = jsonTokenStruct[i];
				jsmntok_t dataToken = jsonTokenStruct[i + 1];
				uint32_t dataLength = dataToken.end - dataToken.start;
				if (UpdateValueIfNoObject(shadowRxBuffer, (primitiveJson_t*)tokenTable[j].pStruct, dataToken) == NONE_ERROR) {
					if (tokenTable[j].FunctionPointer != NULL) {
						tokenTable[j].FunctionPointer(shadowRxBuffer + dataToken.start, dataLength,
								(primitiveJson_t*)tokenTable[j].pStruct);
					}
				}
			}
		}
	}
	return NONE_ERROR;
}

// This needs to be removed
MQTTClient_t *pUnsubscribeClient;

IoT_Error_t iot_shadow_init(MQTTClient_t *pClient) {

	IoT_Error_t rc = NONE_ERROR;

	if (pClient == NULL) {
		return NULL_VALUE_ERROR;
	}
	InitAckTableEntry();
	pUnsubscribeClient = pClient;
	return NONE_ERROR;
}

// This library is not Thread safe
static MQTTConnectParams ConnectParams;
#define MAX_TOPIC_LENGTH 100
#define MAX_THING_NAME_SIZE_IN_BYTES 20
static char myThingName[MAX_THING_NAME_SIZE_IN_BYTES];
static char ShadowTopicUpdateDelta[MAX_TOPIC_LENGTH];

void setUniqueClientID_String(char *pUniqueID) {
	strncpy(uniqueClientID, pUniqueID, MAX_SIZE_OF_UNIQUE_CLIENT_ID_BYTES-1); // To ensure we have space for '\0'
}

IoT_Error_t iot_shadow_connect(MQTTClient_t *pClient, ShadowParameters_t *pParams) {
	IoT_Error_t rc = NONE_ERROR;

	if (pClient == NULL) {
		return NULL_VALUE_ERROR;
	}

	if (pClient->connect == NULL) {
		return NULL_VALUE_ERROR;
	}

	ConnectParams.KeepAliveInterval_sec = 10;
	ConnectParams.MQTTVersion = MQTT_3_1_1;
	ConnectParams.mqttCommandTimeout_ms = 2000;
	ConnectParams.tlsHandshakeTimeout_ms = 10000;
	ConnectParams.isCleansession = true;
	ConnectParams.isSSLHostnameVerify = true;
	ConnectParams.isWillMsgPresent = false;
	ConnectParams.pClientID = pParams->pUniqueThingId;
	ConnectParams.pDeviceCertLocation = pParams->pClientCRT;
	ConnectParams.pDevicePrivateKeyLocation = pParams->pClientKey;
	ConnectParams.pRootCALocation = pParams->pRootCA;
	ConnectParams.pPassword = NULL;
	ConnectParams.pUserName = NULL;
	ConnectParams.pHostURL = pParams->pHost;
	ConnectParams.port = pParams->port;

	//@@ TODO: change it back
	sprintf(ShadowTopicUpdateDelta, "$aws/things/%s/shadow/update/delta", pParams->pUniqueThingId);
	strncpy(myThingName, pParams->pUniqueThingId, MAX_THING_NAME_SIZE_IN_BYTES);

	/*
	 * On every connect the subscriptions have to be made again also the register delta has to be called again
	 */
	deltaTopicSubscribedFlag = false;
	isMyThingNameAcceptedTopicSubscribed = false;
	isMyThingNameRejectedTopicSubscribed = false;
	isMyThingNameGetAcceptedTopicSubscribed = false;
	isMyThingNameGetRejectedTopicSubscribed = false;
	clientTokenNum = 0;
	rc = pClient->connect(&ConnectParams);
	return rc;
}

IoT_Error_t iot_shadow_register_delta(MQTTClient_t *pClient, primitiveJson_t *pStruct) {
	IoT_Error_t rc = NONE_ERROR;

	if (!(pClient->isConnected())) {
		return CONNECTION_ERROR;
	}

	if (!deltaTopicSubscribedFlag) {
		MQTTSubscribeParams subParams;
		subParams.mHandler = shadow_delta_callback;
		subParams.pTopic = ShadowTopicUpdateDelta;
		subParams.qos = QOS_0;
		rc = pClient->subscribe(&subParams);
		DEBUG("delta topic %s", ShadowTopicUpdateDelta);
		deltaTopicSubscribedFlag = true;
	}

	if (tokenTableIndex >= MAX_TOKEN_EXPECTED) {
		return GENERIC_ERROR;
	}

	tokenTable[tokenTableIndex].pKey = pStruct->pKey;
	tokenTable[tokenTableIndex].FunctionPointer = pStruct->cb;
	tokenTable[tokenTableIndex].pStruct = pStruct;
	tokenTableIndex++;

	return rc;
}

inline static void FillThingName_UpdateAccepted(char *pTopic, const char *pThingName) {
	// @@ TODO: put back the $
	sprintf(pTopic, "$aws/things/%s/shadow/update/accepted", pThingName);
}

inline static void FillThingName_UpdateRejected(char *pTopic, const char *pThingName) {
	// @@ TODO: put back the $
	sprintf(pTopic, "$aws/things/%s/shadow/update/rejected", pThingName);
}

inline static void FillThingName_Update(char *pTopic, const char *pThingName) {
	// @@ TODO: put back the $
	sprintf(pTopic, "$aws/things/%s/shadow/update", pThingName);
}

inline static void FillThingName_GetAccepted(char *pTopic, const char *pThingName) {
	// @@ TODO: put back the $
	sprintf(pTopic, "$aws/things/%s/shadow/get/accepted", pThingName);
}

inline static void FillThingName_GetRejected(char *pTopic, const char *pThingName) {
	// @@ TODO: put back the $
	sprintf(pTopic, "$aws/things/%s/shadow/get/rejected", pThingName);
}

inline static void FillThingName_Get(char *pTopic, const char *pThingName) {
	// @@ TODO: put back the $
	sprintf(pTopic, "$aws/things/%s/shadow/get", pThingName);
}

static void unsubscribeFromAcceptedAndRejected(char *pThingName) {
	char TemporaryTopic[MAX_SHADOW_TOPIC_LENGTH_BYTES];
	if (strcmp(pThingName, myThingName) != 0) {
		FillThingName_UpdateAccepted(TemporaryTopic, pThingName);
		pUnsubscribeClient->unsubscribe(TemporaryTopic);
		FillThingName_UpdateRejected(TemporaryTopic, pThingName);
		pUnsubscribeClient->unsubscribe(TemporaryTopic);
	}
}

static void HandleExpiredUpdateResponseCallbacks(MQTTClient_t *pClient) {
	uint8_t i;
	for (i = 0; i < MAX_NUM_OF_CLIENT_RESPONSES; i++) {
		if (!expectedAckTable[i].isFree) {
			//if (expired(&(expectedAckTable[i].timer))) {
				if (expectedAckTable[i].callback != NULL) {
					expectedAckTable[i].callback(expectedAckTable[i].pContextData, SHADOW_ACK_TIMEOUT);
				}
				expectedAckTable[i].isFree = true;
				unsubscribeFromAcceptedAndRejected(expectedAckTable[i].ThingName);
			//}
		}
	}
}

IoT_Error_t iot_shadow_yield(MQTTClient_t *pClient, int timeout) {
	HandleExpiredUpdateResponseCallbacks(pClient);
	return pClient->yield(timeout);
}

IoT_Error_t iot_shadow_disconnect(MQTTClient_t *pClient) {
	return pClient->disconnect();
}
typedef struct {
	const char *pJsonString;
	const char *pThingNameToUpdate;
	pStatusCallback_t callback;
	void *pContext;
} updateParams_t;

#define CLIENT_TOKEN_STRING "clientToken"
// there cannot be a client token string in customer json. This can be fixed
static bool ExtractClientTokenFromJson(const char *pJsonString, jsmntok_t *pClientJsonToken) {
	bool ret_val = false;
	jsmn_init(&shadowJsonParser);
	int32_t tokenCount, i;

//	DEBUG("Input JSON: %s\n", pJsonString);

	tokenCount = jsmn_parse(&shadowJsonParser, pJsonString, strlen(pJsonString), jsonTokenStruct,
			sizeof(jsonTokenStruct) / sizeof(jsonTokenStruct[0]));

	if (tokenCount < 0) {
		WARN("Failed to parse JSON: %d\n", tokenCount);
		return false;
	}

	/* Assume the top-level element is an object */
	if (tokenCount < 1 || jsonTokenStruct[0].type != JSMN_OBJECT) {
		return false;
	}

//	DEBUG("total tokens %d", tokenCount);

	for (i = 1; i < tokenCount; i++) {
		if (jsoneq(pJsonString, &jsonTokenStruct[i], CLIENT_TOKEN_STRING) == 0) {
			*pClientJsonToken = jsonTokenStruct[i + 1];
			return true;
		}
	}
	return ret_val;
}

static int32_t updateStatusCallback(MQTTCallbackParams params) {
	// extract the client token
	int32_t tokenCount;
	int32_t i;
	jsmntok_t ClientJsonToken;
	jsmn_init(&shadowJsonParser);

	if (params.MessageParams.PayloadLen > SHADOW_MAX_SIZE_OF_RX_BUFFER) {
		return GENERIC_ERROR;
	}

	memcpy(shadowRxBuffer, params.MessageParams.pPayload, params.MessageParams.PayloadLen);
	shadowRxBuffer[params.MessageParams.PayloadLen] = '\0';	// jsmn_parse relies on a string

	//DEBUG("Input JSON: %s\n", shadowRxBuffer);

	tokenCount = jsmn_parse(&shadowJsonParser, shadowRxBuffer, strlen(shadowRxBuffer), jsonTokenStruct,
			sizeof(jsonTokenStruct) / sizeof(jsonTokenStruct[0]));
	//DEBUG("total tokens %d", tokenCount);

	if (tokenCount < 0) {
		WARN("Failed to parse JSON: %d\n", tokenCount);
		return GENERIC_ERROR;
	}

	/* Assume the top-level element is an object */
	if (tokenCount < 1 || jsonTokenStruct[0].type != JSMN_OBJECT) {
		return GENERIC_ERROR;
	}

	if (ExtractClientTokenFromJson(shadowRxBuffer, &ClientJsonToken))
		// look through the Ack table to compare the ones that have timer enabled
		for (i = 0; i < MAX_NUM_OF_CLIENT_RESPONSES; i++) {
			if (!expectedAckTable[i].isFree) {
				if (strncmp(expectedAckTable[i].clientTokenID, shadowRxBuffer + ClientJsonToken.start,
						ClientJsonToken.end - ClientJsonToken.start) == 0) {
					Shadow_Ack_Status_t status;
					if (strstr(params.pTopicName, "accepted") != NULL) {
						status = SHADOW_ACK_ACCEPTED;
					} else if (strstr(params.pTopicName, "rejected") != NULL) {
						status = SHADOW_ACK_REJECTED;
					}
					if (status == SHADOW_ACK_ACCEPTED || status == SHADOW_ACK_REJECTED) {
						expectedAckTable[i].callback(expectedAckTable[i].pContextData, status);
						expectedAckTable[i].isFree = true;
						unsubscribeFromAcceptedAndRejected(expectedAckTable[i].ThingName);
					}
					break;
				}
			}
		}
	return 0;
}

static int8_t GetNextFreeAckTableElement() {
	int8_t i;
	for (i = 0; i < MAX_NUM_OF_CLIENT_RESPONSES; i++) {
		if (expectedAckTable[i].isFree == true) {
			return i;
		}
	}
	return -1;
}

static MQTTSubscribeParams subParams;
char TemporaryTopicA[MAX_SHADOW_TOPIC_LENGTH_BYTES];
char TemporaryTopicR[MAX_SHADOW_TOPIC_LENGTH_BYTES];
char TemporaryTopicP[MAX_SHADOW_TOPIC_LENGTH_BYTES];
/*
 * MQTT subscribe will not return till it gets a suback - which might take 30 seconds
 * all subscriptions are qos 0
 */
IoT_Error_t iot_shadow_update(MQTTClient_t *pClient, char *pJsonString, pStatusCallback_t callback, void *pContextData,
		uint8_t timeout_seconds) {
	IoT_Error_t ret_val = NONE_ERROR;

	int8_t freeAckTableIndex;
	jsmntok_t clientTokenJson;
	bool isClientTokenPresent = false;
	bool isCallbackPresent = false;
	isClientTokenPresent = ExtractClientTokenFromJson(pJsonString, &clientTokenJson);
	if (isClientTokenPresent) {
		freeAckTableIndex = GetNextFreeAckTableElement();
		if (freeAckTableIndex < 0) {
			return WAIT_FOR_PUBLISH;
		}
		if (callback != NULL) {
			isCallbackPresent = true;

			subParams.mHandler = updateStatusCallback;
			subParams.qos = QOS_0;

			if (!isMyThingNameAcceptedTopicSubscribed) {
				FillThingName_UpdateAccepted(TemporaryTopicA, myThingName);
				subParams.pTopic = TemporaryTopicA;
				ret_val = pClient->subscribe(&subParams);
				if (ret_val == NONE_ERROR) {
					isMyThingNameAcceptedTopicSubscribed = true;
				}
			}
			//DEBUG("accepted topic %s", TemporaryTopic);
			if (ret_val == NONE_ERROR) {
				if (!isMyThingNameRejectedTopicSubscribed) {
					FillThingName_UpdateRejected(TemporaryTopicR, myThingName);
					subParams.pTopic = TemporaryTopicR;
					ret_val = pClient->subscribe(&subParams);
					if (ret_val == NONE_ERROR) {
						isMyThingNameRejectedTopicSubscribed = true;
					}
				}
			}
		}
	}

	if (ret_val == NONE_ERROR) {
		MQTTPublishParams pubParams;
		FillThingName_Update(TemporaryTopicP, myThingName);
		pubParams.pTopic = TemporaryTopicP;
		MQTTMessageParams msgParams;
		msgParams.PayloadLen = strlen(pJsonString) + 1;
		msgParams.qos = QOS_0;
		msgParams.pPayload = pJsonString;
		pubParams.MessageParams = msgParams;
		ret_val = pClient->publish(&pubParams);
	}

	if (ret_val == NONE_ERROR && isClientTokenPresent && isCallbackPresent) {
		expectedAckTable[freeAckTableIndex].callback = callback;
		uint8_t clientIdLength = clientTokenJson.end - clientTokenJson.start;
		strncpy(expectedAckTable[freeAckTableIndex].clientTokenID, pJsonString + clientTokenJson.start, clientIdLength);
		expectedAckTable[freeAckTableIndex].pContextData = pContextData;
		//InitTimer(&(expectedAckTable[freeAckTableIndex].timer));
		//countdown(&(expectedAckTable[freeAckTableIndex].timer), timeout_seconds);
		expectedAckTable[freeAckTableIndex].isFree = false;
	}

	return ret_val;
}

inline static void convertDataToString(char *pStringBuffer, JsonPrimitiveType type, void *pData) {
	if (type == SHADOW_JSON_INT32) {
		sprintf(pStringBuffer, "%d,", *(int32_t * )(pData));
	} else if (type == SHADOW_JSON_INT16) {
		sprintf(pStringBuffer, "%d,", *(int16_t * )(pData));
	} else if (type == SHADOW_JSON_INT8) {
		sprintf(pStringBuffer, "%d,", *(int8_t * )(pData));
	} else if (type == SHADOW_JSON_UINT32) {
		sprintf(pStringBuffer, "%d,", *(uint32_t * )(pData));
	} else if (type == SHADOW_JSON_UINT16) {
		sprintf(pStringBuffer, "%d,", *(uint16_t * )(pData));
	} else if (type == SHADOW_JSON_UINT8) {
		sprintf(pStringBuffer, "%d,", *(uint8_t * )(pData));
	}

	else if (type == SHADOW_JSON_DOUBLE) {
		sprintf(pStringBuffer, "%f,", *(double * )(pData));
	} else if (type == SHADOW_JSON_FLOAT) {
		sprintf(pStringBuffer, "%f,", *(float * )(pData));
	} else if (type == SHADOW_JSON_BOOL) {
		sprintf(pStringBuffer, "%s,", *(bool *)(pData)?"true":"false");
	} else if (type == SHADOW_JSON_STRING) {
		sprintf(pStringBuffer, "\"%s\",", (char * )(pData));
	}
}

static char UniqueClientTokenWithSequence[MAX_SIZE_CLIENT_ID_WITH_SEQUENCE];
// Protect this with mutex to ensure multiple threads dont update it at the same time
static char* generateClientToken() {
	sprintf(UniqueClientTokenWithSequence, "%s-%d", UNIQUE_CLIENT_TOKEN, clientTokenNum++);
	return UniqueClientTokenWithSequence;
}
char ShadowTxBuffer[SHADOW_MAX_SIZE_OF_TX_BUFFER];

char* iot_shadow_init_json_document(void) {
	sprintf(ShadowTxBuffer, "{\"state\":{");
	return ShadowTxBuffer;
}

IoT_Error_t iot_shadow_add_desired(char *pJsonDocument, uint8_t count, ...) {
	int8_t i;
	va_list pArgs;
	va_start(pArgs, count);
	primitiveJson_t *pTemporary;
	sprintf(ShadowTxBuffer + strlen(ShadowTxBuffer), "\"desired\":{");
	for (i = 0; i < count; i++) {
		pTemporary = va_arg (pArgs, primitiveJson_t *);
		if (pTemporary != NULL) {
			sprintf(ShadowTxBuffer + strlen(ShadowTxBuffer), "\"%s\":", pTemporary->pKey);
			if (pTemporary->pKey != NULL && pTemporary->pData != NULL) {
				convertDataToString(ShadowTxBuffer + strlen(ShadowTxBuffer), pTemporary->type, pTemporary->pData);
			} else {
				return NULL_VALUE_ERROR;
			}
		} else {
			return NULL_VALUE_ERROR;
		}
	}

	va_end(pArgs);
	sprintf(ShadowTxBuffer + strlen(ShadowTxBuffer) - 1, "},");
	return NONE_ERROR;
}

IoT_Error_t iot_shadow_add_reported(char *pJsonDocument, uint8_t count, ...) {
	int8_t i;
	va_list pArgs;
	va_start(pArgs, count);
	primitiveJson_t *pTemporary;
	sprintf(ShadowTxBuffer + strlen(ShadowTxBuffer), "\"reported\":{");
	for (i = 0; i < count; i++) {
		pTemporary = va_arg (pArgs, primitiveJson_t *);
		if (pTemporary != NULL) {
			sprintf(ShadowTxBuffer + strlen(ShadowTxBuffer), "\"%s\":", pTemporary->pKey);
			if (pTemporary->pKey != NULL && pTemporary->pData != NULL) {
				convertDataToString(ShadowTxBuffer + strlen(ShadowTxBuffer), pTemporary->type, pTemporary->pData);
			} else {
				return NULL_VALUE_ERROR;
			}
		} else {
			return NULL_VALUE_ERROR;
		}
	}

	va_end(pArgs);
	sprintf(ShadowTxBuffer + strlen(ShadowTxBuffer) - 1, "},");
	return NONE_ERROR;
}

void iot_finalize_json_document(char *pJsonDocument) {
	// strlen(ShadowTxBuffer) - 1 is to ensure we remove the last ,(comma) that was added
	sprintf(ShadowTxBuffer + strlen(ShadowTxBuffer) - 1, "}, \"%s\":\"%s\"}", CLIENT_TOKEN_STRING,
			generateClientToken());
}

static pGetJsonDocumentCallback_t GetJsonDocumentCB;

void getDocumentCallback(void *pContextData, Shadow_Ack_Status_t status) {
	if (GetJsonDocumentCB != NULL) {
		if (status != SHADOW_ACK_TIMEOUT) {
			GetJsonDocumentCB(shadowRxBuffer, status);
		} else {
			GetJsonDocumentCB(NULL, status);
		}
	}
}


static char ShadowGetTopic[MAX_SHADOW_TOPIC_LENGTH_BYTES];
static char ShadowGetAcceptedTopic[MAX_SHADOW_TOPIC_LENGTH_BYTES];
static char ShadowGetRejectedTopic[MAX_SHADOW_TOPIC_LENGTH_BYTES];

IoT_Error_t iot_shadow_get_document(MQTTClient_t *pClient, pGetJsonDocumentCallback_t callback,
		uint32_t timeout_seconds) {

	IoT_Error_t ret_val = NONE_ERROR;
	int8_t freeAckTableIndex;

	if (callback == NULL) {
		return NULL_VALUE_ERROR;
	}

	GetJsonDocumentCB = callback;


	MQTTSubscribeParams subParams;
	subParams.mHandler = updateStatusCallback;
	FillThingName_GetAccepted(ShadowGetAcceptedTopic, myThingName);
	subParams.qos = QOS_0;
	if (!isMyThingNameGetAcceptedTopicSubscribed) {
		subParams.pTopic = ShadowGetAcceptedTopic;
		ret_val = pClient->subscribe(&subParams);
		if (ret_val == NONE_ERROR) {
			isMyThingNameGetAcceptedTopicSubscribed = true;
		}
	}
	if (ret_val == NONE_ERROR) {
		if (!isMyThingNameGetRejectedTopicSubscribed) {
			subParams.pTopic = ShadowGetRejectedTopic;
			FillThingName_GetRejected(ShadowGetRejectedTopic, myThingName);
			ret_val = pClient->subscribe(&subParams);
			if (ret_val == NONE_ERROR) {
				isMyThingNameGetRejectedTopicSubscribed = true;
			}
		}
	}

	if (ret_val == NONE_ERROR) {
		sprintf(ShadowTxBuffer, "{\"%s\":\"%s\"}", CLIENT_TOKEN_STRING, generateClientToken());
		MQTTPublishParams pubParams;
		FillThingName_Get(ShadowGetTopic, myThingName);
		pubParams.pTopic = ShadowGetTopic;
		MQTTMessageParams msgParams;
		msgParams.PayloadLen = strlen(ShadowTxBuffer) + 1;
		msgParams.qos = QOS_0;
		msgParams.pPayload = ShadowTxBuffer;
		pubParams.MessageParams = msgParams;
		ret_val = pClient->publish(&pubParams);
	}
	if (ret_val == NONE_ERROR) {
		freeAckTableIndex = GetNextFreeAckTableElement();
		if (freeAckTableIndex < 0) {
			return WAIT_FOR_PUBLISH;
		}
		expectedAckTable[freeAckTableIndex].callback = getDocumentCallback;
		// UniqueClientTokenWithSequence will hold the last generated ClientToken
		strcpy(expectedAckTable[freeAckTableIndex].clientTokenID, UniqueClientTokenWithSequence);
		expectedAckTable[freeAckTableIndex].pContextData = NULL;
		//InitTimer(&(expectedAckTable[freeAckTableIndex].timer));
		//countdown(&(expectedAckTable[freeAckTableIndex].timer), timeout_seconds);
		expectedAckTable[freeAckTableIndex].isFree = false;
	}

	return ret_val;
}
