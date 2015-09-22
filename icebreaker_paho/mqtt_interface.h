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

/**
 * @file mqtt_interface.h
 * @brief Interface definition for MQTT client.
 */

#ifndef AWS_IOT_SDK_SRC_IOT_MQTT_INTERFACE_H_
#define AWS_IOT_SDK_SRC_IOT_MQTT_INTERFACE_H_

#include "iot_error.h"
#include "stddef.h"
#include "stdbool.h"
#include "stdint.h"


/// Private Beta US East (N. Virginia) - Convenience define for private beta US East service endpoint.
#define IOT_PB_US_EAST_1 "g.us-east-1.pb.iot.amazonaws.com"

/// US East (N. Virginia) - Convenience define for US East service endpoint.
#define IOT_US_EAST_1 "g.us-east-1.iot.amazonaws.com"

/// US West (Oregon) - Convenience define for US West service endpoint.
#define IOT_US_WEST_2 "g.us-west-2.iot.amazonaws.com"

/// EU (Ireland) - Convenience define for EU West service endpoint.
#define IOT_EU_WEST_1 "g.eu-west-1.iot.amazonaws.com"

/// Asia Pacific (Tokyo) - Convenience define for AP Northeast service endpoint.
#define IOT_AP_NORTHEAST_1 "g.ap-northeast-1.iot.amazonaws.com"


typedef enum {
	MQTT_3_1 = 3, MQTT_3_1_1 = 4
} MQTT_Ver_t;

typedef enum {
	QOS_0, QOS_1, QOS_2
} QoSLevel;

typedef struct {
	const char *pTopicName;
	const char *pMessage;
	bool isRetained;
	QoSLevel qos;
} MQTTwillOptions;

typedef struct {
	char *pHostURL;
	uint16_t port;
	char *pRootCALocation;
	char *pDeviceCertLocation;
	char *pDevicePrivateKeyLocation;
	char *pClientID;
	char *pUserName;
	char *pPassword;
	MQTT_Ver_t MQTTVersion;
	uint16_t KeepAliveInterval_sec;
	bool isCleansession;
	bool isWillMsgPresent;
	MQTTwillOptions will;
	uint32_t commandTimeout_ms;
	bool isSSLHostnameVerify;
} MQTTConnectParams;

typedef struct {
	QoSLevel qos;
	bool isRetained;
	bool isDuplicate;
	uint16_t id;
	void *pPayload;
	uint32_t PayloadLen;
} MQTTMessageParams;

typedef struct {
	char *pTopicName;
	uint16_t TopicNameLen;
	MQTTMessageParams MessageParams;
} MQTTCallbackParams;

typedef int32_t (*iot_message_handler)(MQTTCallbackParams params);

typedef struct {
	char *pTopic;
	QoSLevel qos;
	iot_message_handler mHandler;
} MQTTSubscribeParams;

typedef struct {
	char *pTopic;
	MQTTMessageParams MessageParams;
} MQTTPublishParams;

int iot_mqtt_connect(MQTTConnectParams *pParams);
int iot_mqtt_publish(MQTTPublishParams *pParams);
int iot_mqtt_subscribe(MQTTSubscribeParams *pParams);
int iot_mqtt_unsubscribe(char *pTopic);
int iot_mqtt_disconnect(void);
int iot_mqtt_yield(int timeout);
bool iot_is_mqtt_connected(void);

#endif /* AWS_IOT_SDK_SRC_IOT_MQTT_INTERFACE_H_ */
