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

#ifndef SRC_SHADOW_IOT_SHADOW_CONFIG_H_
#define SRC_SHADOW_IOT_SHADOW_CONFIG_H_

#define UNIQUE_CLIENT_TOKEN "c-sdk-client#1"
#define MY_THING_NAME "AWS_IoT-C-SDK"

// Size
#define SHADOW_MAX_SIZE_OF_RX_BUFFER 512
#define MAX_SIZE_OF_UNIQUE_CLIENT_ID_BYTES 80  /** {"clientToken": ">>uniqueClientID<<+sequenceNumber"}*/
#define MAX_SIZE_CLIENT_ID_WITH_SEQUENCE MAX_SIZE_OF_UNIQUE_CLIENT_ID_BYTES + 10 /** {"clientToken": ">>uniqueClientID+sequenceNumber<<"}*/
#define MAX_SIZE_CLIENT_TOKEN_CLIENT_SEQUENCE MAX_SIZE_CLIENT_ID_WITH_SEQUENCE + 20 /** >>{"clientToken": "uniqueClientID+sequenceNumber"}<<*/
#define MAX_SIZE_OF_THINGNAME 30
#define MAX_ACKS_TO_COMEIN_AT_ANY_GIVEN_TIME 10
#define MAX_THINGNAME_HANDLED_AT_ANY_GIVEN_TIME 10
#define MAX_JSON_TOKEN_EXPECTED 30
#define MAX_SHADOW_TOPIC_LENGTH_WITHOUT_THINGNAME 60
#define MAX_SIZE_OF_THING_NAME 20
#define MAX_SHADOW_TOPIC_LENGTH_BYTES MAX_SHADOW_TOPIC_LENGTH_WITHOUT_THINGNAME + MAX_SIZE_OF_THING_NAME


#endif /* SRC_SHADOW_IOT_SHADOW_CONFIG_H_ */
