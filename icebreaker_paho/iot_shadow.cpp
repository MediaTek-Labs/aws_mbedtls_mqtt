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
#include "iot_shadow.h"
#include "iot_log.h"
#include "json_utils.h"

// internal parser for shadow
static jsmn_parser shadow_parser;
static jsmntok_t st[128]; // max expected tokens is 128

// definition of shadow-specific topics
static char toShadowTopic[70];		// <-- ThingID dependent
static char fromShadowTopic[70];	// <-- ThingID dependent

state_change_handler userStateHandler;
char *userDeltaBuffer;
char shadowRxBuf[256];

// connection parameters
MQTTConnectParams shadowConnectParams;

// internal MQTT callback
int32_t __shadow_callback_handler(MQTTCallbackParams params);

int iot_shadow_init(ShadowParameters *shadowParams) {

	int rc = NONE_ERROR;

	// initialize the JSON parser
	jsmn_init(&shadow_parser);

	// default
	shadowConnectParams.KeepAliveInterval_sec = 10;
	shadowConnectParams.isCleansession = true;
	shadowConnectParams.MQTTVersion = MQTT_3_1_1;
	shadowConnectParams.isWillMsgPresent = false;
	shadowConnectParams.pUserName = NULL;
	shadowConnectParams.pPassword = NULL;
	shadowConnectParams.commandTimeout_ms = 10000;
	shadowConnectParams.isSSLHostnameVerify = false;// ensure this is set to true for production
	// shadow parameters
	shadowConnectParams.pClientID = shadowParams->pThingId;
	shadowConnectParams.pHostURL = shadowParams->pHost;
	shadowConnectParams.port = shadowParams->port;
	shadowConnectParams.pRootCALocation = shadowParams->pRootCA;
	shadowConnectParams.pDeviceCertLocation = shadowParams->pClientCRT;
	shadowConnectParams.pDevicePrivateKeyLocation = shadowParams->pClientKey;

	// save user's callback handler to be used in our callback>>
	userStateHandler = shadowParams->stateChangeHandler;
	// save user's delta buffer to be used in our callback
	userDeltaBuffer = shadowParams->deltaBuffer;

	// build shadow topics
	sprintf(toShadowTopic,   "$shadow/beta/state/%s", shadowParams->pThingId);
	INFO("UpdateTopic: %s", toShadowTopic);
	sprintf(fromShadowTopic, "$shadow/beta/sync/%s", shadowParams->pThingId);
	INFO("  SyncTopic: %s", fromShadowTopic);

	return NONE_ERROR;
}

int iot_shadow_connect(void) {
	int rc = NONE_ERROR;
	rc = iot_mqtt_connect(&shadowConnectParams);
	if (NONE_ERROR != rc) {
		ERROR("ERROR[%d] Connecting to %s:%d\n", rc, shadowConnectParams.pHostURL, shadowConnectParams.port);
		return rc;
	}

	MQTTSubscribeParams subParams;
	subParams.mHandler = __shadow_callback_handler;
	subParams.pTopic = fromShadowTopic;
	subParams.qos = QOS_0;

	if (NONE_ERROR == rc) {
		rc = iot_mqtt_subscribe(&subParams);
		if (NONE_ERROR != rc) {
			ERROR("ERROR subscribing\n");
			return rc;
		}
	}

	return NONE_ERROR;
}

int iot_shadow_sync_reported(char *reportedJson) {
	int rc = NONE_ERROR;

	char envelopedJson[200];
	sprintf(envelopedJson, "{\"state\":{\"reported\":%s}}", reportedJson);
	Serial.print("in iot_shadow_sync_reported envelopedJson is ");
	Serial.println(envelopedJson);

	MQTTMessageParams Msg;
	Msg.qos = QOS_0;
	Msg.isRetained = false;
	Msg.pPayload = (void *) envelopedJson;
	Msg.PayloadLen = strlen(envelopedJson) + 1;

	MQTTPublishParams pubParams;
	pubParams.pTopic = toShadowTopic;
	pubParams.MessageParams = Msg;

	rc = iot_mqtt_publish(&pubParams);
	Serial.print("iot_mqtt_publish rc is ");
	Serial.println(rc);

	return rc;
}

int iot_shadow_sync_desired(char *desiredJson) {
	int rc = NONE_ERROR;

	char envelopedJson[200];
	sprintf(envelopedJson, "{\"state\":{\"desired\":%s}}", desiredJson);

	MQTTMessageParams Msg;
	Msg.qos = QOS_0;
	Msg.isRetained = false;
	Msg.pPayload = (void *) envelopedJson;
	Msg.PayloadLen = strlen(envelopedJson) + 1;

	MQTTPublishParams pubParams;
	pubParams.pTopic = toShadowTopic;
	pubParams.MessageParams = Msg;

	rc = iot_mqtt_publish(&pubParams);

	return rc;
}

int iot_shadow_disconnect(void) {
	int rc = NONE_ERROR;
	rc = iot_mqtt_disconnect();
	return rc;
}

int32_t __shadow_callback_handler(MQTTCallbackParams params) {
	int i;
	int r;
	int rc = NONE_ERROR;

	jsmn_init(&shadow_parser);

	// TODO: will need length checking (don't overflow RxBuf)
	memcpy(shadowRxBuf, params.MessageParams.pPayload, params.MessageParams.PayloadLen);
	shadowRxBuf[params.MessageParams.PayloadLen] = '\0';	// jsmn_parse relies on a string

	r = jsmn_parse(&shadow_parser, shadowRxBuf, strlen(shadowRxBuf), st, sizeof(st) / sizeof(st[0]));

	if (r < 0) {
		WARN("Failed to parse JSON: %d\n", r);
		return 1;
	}

	/* Assume the top-level element is an object */
	if (r < 1 || st[0].type != JSMN_OBJECT) {
		return 1;
	}

	int32_t version = 0;

	// Loop over all keys of the root object
	for (i = 1; i < r; i++) {
		if (jsoneq(shadowRxBuf, &st[i], "version") == 0) {
			rc = parseIntegerValue(&version, shadowRxBuf, st+i+1);
			INFO("Shadow Version: %d\n", version);
			i++;
		} else if (jsoneq(shadowRxBuf, &st[i], "state") == 0) {
			// a delta is present
			memcpy(userDeltaBuffer, shadowRxBuf + st[i + 1].start, st[i + 1].end - st[i + 1].start);
			userDeltaBuffer[st[i + 1].end - st[i + 1].start] = '\0';

			(userStateHandler)();	// call the user state change callback
			break;					// stop parsing the message after the delta
		}
	}

	return 0;
}

int iot_shadow_yield(int timeout) {
	return iot_mqtt_yield(timeout);
}
