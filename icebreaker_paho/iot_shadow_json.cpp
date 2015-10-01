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

#include "iot_shadow_json.h"
#include "iot_shadow_key.h"

#include "iot_log.h"
#include "json_utils.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "iot_shadow_config.h"

static uint32_t clientTokenNum = 0;

//helper functions
static void convertDataToString(char *pStringBuffer, JsonPrimitiveType type, void *pData);

void resetClientTokenSequenceNum(void) {
	clientTokenNum = 0;
}

static void emptyJsonWithClientToken(char *pJsonDocument) {
	sprintf(pJsonDocument, "{\"clientToken\":\"");
	FillWithClientToken(pJsonDocument + strlen(pJsonDocument));
	sprintf(pJsonDocument + strlen(pJsonDocument), "\"}");
}

void iot_shadow_get_request_json(char *pJsonDocument) {
	emptyJsonWithClientToken(pJsonDocument);
}

void iot_shadow_delete_request_json(char *pJsonDocument) {
	emptyJsonWithClientToken(pJsonDocument);
}

void iot_shadow_init_json_document(char *pJsonDocument) {
	sprintf(pJsonDocument, "{\"state\":{");
}

IoT_Error_t iot_shadow_add_desired(char *pJsonDocument, uint8_t count, ...) {
	int8_t i;
	va_list pArgs;
	va_start(pArgs, count);
	jsonStruct_t *pTemporary;
	sprintf(pJsonDocument + strlen(pJsonDocument), "\"desired\":{");
	for (i = 0; i < count; i++) {
		pTemporary = va_arg (pArgs, jsonStruct_t *);
		if (pTemporary != NULL) {
			sprintf(pJsonDocument + strlen(pJsonDocument), "\"%s\":", pTemporary->pKey);
			if (pTemporary->pKey != NULL && pTemporary->pData != NULL) {
				convertDataToString(pJsonDocument + strlen(pJsonDocument), pTemporary->type, pTemporary->pData);
			} else {
				return NULL_VALUE_ERROR;
			}
		} else {
			return NULL_VALUE_ERROR;
		}
	}

	va_end(pArgs);
	sprintf(pJsonDocument + strlen(pJsonDocument) - 1, "},");
	return NONE_ERROR;
}

IoT_Error_t iot_shadow_add_reported(char *pJsonDocument, uint8_t count, ...) {
	int8_t i;
	va_list pArgs;
	va_start(pArgs, count);
	jsonStruct_t *pTemporary;
	sprintf(pJsonDocument + strlen(pJsonDocument), "\"reported\":{");
	for (i = 0; i < count; i++) {
		pTemporary = va_arg (pArgs, jsonStruct_t *);
		if (pTemporary != NULL) {
			sprintf(pJsonDocument + strlen(pJsonDocument), "\"%s\":", pTemporary->pKey);
			if (pTemporary->pKey != NULL && pTemporary->pData != NULL) {
				convertDataToString(pJsonDocument + strlen(pJsonDocument), pTemporary->type, pTemporary->pData);
			} else {
				return NULL_VALUE_ERROR;
			}
		} else {
			return NULL_VALUE_ERROR;
		}
	}

	va_end(pArgs);
	sprintf(pJsonDocument + strlen(pJsonDocument) - 1, "},");
	return NONE_ERROR;
}

void iot_finalize_json_document(char *pJsonDocument) {
	// strlen(ShadowTxBuffer) - 1 is to ensure we remove the last ,(comma) that was added
	sprintf(pJsonDocument + strlen(pJsonDocument) - 1, "}, \"%s\":\"", SHADOW_CLIENT_TOKEN_STRING);
	FillWithClientToken(pJsonDocument + strlen(pJsonDocument));
	sprintf(pJsonDocument + strlen(pJsonDocument), "\"}");
}

void FillWithClientToken(char *pBufferToBeUpdatedWithClientToken) {
	sprintf(pBufferToBeUpdatedWithClientToken, "%s-%d", UNIQUE_CLIENT_TOKEN, clientTokenNum++);
}

static void convertDataToString(char *pStringBuffer, JsonPrimitiveType type, void *pData) {
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
static jsmn_parser shadowJsonParser;
static jsmntok_t jsonTokenStruct[MAX_JSON_TOKEN_EXPECTED];

bool isJsonValidAndParse(const char *pJsonDocument, void *pJsonHandler, int32_t *pTokenCount) {
	int32_t tokenCount;

	jsmn_init(&shadowJsonParser);

	tokenCount = jsmn_parse(&shadowJsonParser, pJsonDocument, strlen(pJsonDocument), jsonTokenStruct,
			sizeof(jsonTokenStruct) / sizeof(jsonTokenStruct[0]));

	if (tokenCount < 0) {
		WARN("Failed to parse JSON: %d\n", tokenCount);
		return false;
	}

	/* Assume the top-level element is an object */
	if (tokenCount < 1 || jsonTokenStruct[0].type != JSMN_OBJECT) {
		WARN("Top Level is not an object\n");
		return false;
	}

	pJsonHandler = (void *) jsonTokenStruct;
	*pTokenCount = tokenCount;

	return true;
}

static IoT_Error_t UpdateValueIfNoObject(const char *pJsonString, jsonStruct_t *pDataStruct, jsmntok_t token) {
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

bool isJsonKeyMatchingAndUpdateValue(const char *pJsonDocument, void *pJsonHandler, int32_t tokenCount,
		jsonStruct_t *pDataStruct, uint32_t *pDataLength, int32_t *pDataPosition) {
	int32_t i;

	jsmntok_t *pJsonTokenStruct;

	pJsonTokenStruct = (jsmntok_t *) pJsonHandler;
	for (i = 1; i < tokenCount; i++) {
		if (jsoneq(pJsonDocument, &(jsonTokenStruct[i]), pDataStruct->pKey) == 0) {
			jsmntok_t dataToken = jsonTokenStruct[i+1];
			uint32_t dataLength = dataToken.end - dataToken.start;
			UpdateValueIfNoObject(pJsonDocument, pDataStruct, dataToken);
			*pDataPosition = dataToken.start;
			*pDataLength = dataLength;
			return true;
		}
	}
	return false;
}

bool isReceivedJsonValid(const char *pJsonDocument) {
	int32_t tokenCount;

	jsmn_init(&shadowJsonParser);

	tokenCount = jsmn_parse(&shadowJsonParser, pJsonDocument, strlen(pJsonDocument), jsonTokenStruct,
			sizeof(jsonTokenStruct) / sizeof(jsonTokenStruct[0]));

	if (tokenCount < 0) {
		WARN("Failed to parse JSON: %d\n", tokenCount);
		return false;
	}

	/* Assume the top-level element is an object */
	if (tokenCount < 1 || jsonTokenStruct[0].type != JSMN_OBJECT) {
		return false;
	}

	return true;
}

bool extractClientToken(const char *pJsonDocument, char *pExtractedClientToken) {
	bool ret_val = false;
	jsmn_init(&shadowJsonParser);
	int32_t tokenCount, i;
	jsmntok_t ClientJsonToken;

	tokenCount = jsmn_parse(&shadowJsonParser, pJsonDocument, strlen(pJsonDocument), jsonTokenStruct,
			sizeof(jsonTokenStruct) / sizeof(jsonTokenStruct[0]));

	if (tokenCount < 0) {
		WARN("Failed to parse JSON: %d\n", tokenCount);
		return false;
	}

	/* Assume the top-level element is an object */
	if (tokenCount < 1 || jsonTokenStruct[0].type != JSMN_OBJECT) {
		return false;
	}

	for (i = 1; i < tokenCount; i++) {
		if (jsoneq(pJsonDocument, &jsonTokenStruct[i], SHADOW_CLIENT_TOKEN_STRING) == 0) {
			ClientJsonToken = jsonTokenStruct[i + 1];
			uint8_t length = ClientJsonToken.end - ClientJsonToken.start;
			strncpy(pExtractedClientToken, pJsonDocument + ClientJsonToken.start, length);
			pExtractedClientToken[length] = '\0';
			return true;
		}
	}

	return false;
}

bool extractVersionNumber(const char *pJsonDocument, void *pJsonHandler, int32_t tokenCount, uint32_t *pVersionNumber){
	int32_t i;
	jsmntok_t *pJsonTokenStruct;
	IoT_Error_t ret_val = NONE_ERROR;

	pJsonTokenStruct = (jsmntok_t *) pJsonHandler;
	for (i = 1; i < tokenCount; i++) {
		if (jsoneq(pJsonDocument, &(jsonTokenStruct[i]), SHADOW_VERSION_STRING) == 0) {
			jsmntok_t dataToken = jsonTokenStruct[i+1];
			uint32_t dataLength = dataToken.end - dataToken.start;
			ret_val = parseUnsignedInteger32Value(pVersionNumber, pJsonDocument, &dataToken);
			if(ret_val == NONE_ERROR){
				return true;
			}
		}
	}
	return false;
}

