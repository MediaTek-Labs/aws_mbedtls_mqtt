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

#ifndef __NETWORK_INTERFACE_H_
#define __NETWORK_INTERFACE_H_

typedef struct Network Network;

typedef struct{
	char* pRootCALocation;
	char* pDeviceCertLocation;
	char* pDevicePrivateKeyLocation;
	char* pDestinationURL;
	int DestinationPort;
	unsigned int timeout_ms;
	unsigned char ServerVerificationFlag;
}TLSConnectParams;

struct Network{
	int my_socket;
	int (*mqttread) (Network*, unsigned char*, int, int);
	int (*mqttwrite) (Network*, unsigned char*, int, int);
	void (*disconnect) (Network*);
};

int iot_tls_init(Network *pNetwork);
int iot_tls_connect(Network *pNetwork, TLSConnectParams TLSParams);
int iot_tls_write(Network*, unsigned char*, int, int);
int iot_tls_read(Network*, unsigned char*, int, int);
void iot_tls_disconnect(Network *pNetwork);
int iot_tls_destroy(Network *pNetwork);

#endif //__NETWORK_INTERFACE_H_
