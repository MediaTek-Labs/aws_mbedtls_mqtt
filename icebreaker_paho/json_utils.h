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

#ifndef AWS_IOT_SDK_SRC_JSON_UTILS_H_
#define AWS_IOT_SDK_SRC_JSON_UTILS_H_

#include <stdbool.h>
#include <stdint.h>
#include "iot_error.h"
#include "jsmn.h"

// utility functions
int8_t jsoneq(const char *json, jsmntok_t *tok, const char *s);
IoT_Error_t parseIntegerValue(int32_t *i, const char *jsonString, jsmntok_t *token);
IoT_Error_t parseDoubleValue(double *d, const char *jsonString, jsmntok_t *token);
IoT_Error_t parseBooleanValue(bool *b, const char *jsonString, jsmntok_t *token);
IoT_Error_t parseStringValue(char *buf, const char *jsonString, jsmntok_t *token);

#endif /* AWS_IOT_SDK_SRC_JSON_UTILS_H_ */
