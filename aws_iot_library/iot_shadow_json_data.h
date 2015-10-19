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

#ifndef SRC_SHADOW_IOT_SHADOW_JSON_DATA_H_
#define SRC_SHADOW_IOT_SHADOW_JSON_DATA_H_

typedef struct jsonStruct jsonStruct_t;
typedef void (*jsonStructCallback_t)(const char *pJsonValueBuffer, uint32_t valueLength, jsonStruct_t *pJsonStruct_t);

typedef enum {
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
} JsonPrimitiveType;

struct jsonStruct {
	const char *pKey;
	void *pData;
	JsonPrimitiveType type;
	jsonStructCallback_t cb;
};

void iot_shadow_init_json_document(char *pJsonDocument);
IoT_Error_t iot_shadow_add_reported(char *pJsonDocument, uint8_t count, ...);
IoT_Error_t iot_shadow_add_desired(char *pJsonDocument, uint8_t count, ...);
void iot_finalize_json_document(char *pJsonDocument);

#endif /* SRC_SHADOW_IOT_SHADOW_JSON_DATA_H_ */
