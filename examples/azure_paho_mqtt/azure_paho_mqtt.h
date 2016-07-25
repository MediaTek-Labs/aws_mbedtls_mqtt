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
 * @file aws_mtk_iot_config.h
 * @brief AWS IoT specific configuration file
 */

#ifndef SRC_SHADOW_IOT_SHADOW_CONFIG_H_
#define SRC_SHADOW_IOT_SHADOW_CONFIG_H_

// Get from console
// =================================================
#define AWS_IOT_MQTT_HOST              "nthuiot.azure-devices.net" ///< Customer specific MQTT HOST. The same will be used for Thing Shadow
#define AWS_IOT_MQTT_PORT              8883 ///< default port for MQTT/S
#define AWS_IOT_MQTT_CLIENT_ID         "yourclientID" ///< MQTT client ID should be unique for every device
#define AWS_IOT_MY_THING_NAME 	       "yourclientID" //IMPORTANT: for a temporary work around, you also need to modify the same name in your library for aws_iot_config.h (under arduino library path)
#define AWS_IOT_ROOT_CA_FILENAME       "root.pem" ///< Root CA file name
#define AWS_IOT_CERTIFICATE_FILENAME   "8cbd725746-certificate.pem.crt" ///< device signed certificate file name
#define AWS_IOT_PRIVATE_KEY_FILENAME   "8cbd725746-private.pem.key" ///< Device private key filename
#define AZURE_IOT_SUB_TOPIC_NAME       "devices/linkittest/messages/devicebound/#" //MQTT subscribe Topic name
#define AZURE_IOT_PUB_TOPIC_NAME       "devices/linkittest/messages/events/" //MQTT publish Topic name
#define AZURE_IOT_USER_NAME            "nthuiot.azure-devices.net/yourdevicename"
#define AZURE_IOT_USER_PASSWORD        "iothub Password"

// =================================================

//set to use Wifi or GPRS
#define WIFI_USED true  //true (Wifi) or false (GPRS)

//Set to Azure mqtt usage
#define AZURE_USED true

/* change Wifi settings here */
#define WIFI_AP "yourwifissid"
#define WIFI_PASSWORD "yourwifipassword"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP

/* change GPRS settings here */
#define GPRS_APN "wap.cingular"   //for AT&T
#define GPRS_USERNAME "wap@cingulargprs.com"
#define GPRS_PASSWORD "cingular1"

#endif /* SRC_SHADOW_IOT_SHADOW_CONFIG_H_ */
