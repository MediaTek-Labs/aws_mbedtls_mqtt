/*
  Web client

 This sketch connects to a website 
 using Wi-Fi functionality on MediaTek LinkIt platform.

 Change the macro WIFI_AP, WIFI_PASSWORD, WIFI_AUTH and SITE_URL accordingly.

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 modified 20 Aug 2014
 by MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <vmsock.h>
#include <net.h>
#include <mtk.h>

#include <signal.h>
#include <limits.h>
#include "mqtt_interface.h"
#include "iot_version.h"
#include "iot_log.h"

#ifdef connect
#undef connect
#endif
#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>

/* change Wifi settings here */
#define WIFI_AP "MUS"
#define WIFI_PASSWORD "mediatekmus"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.

/* change server settings here */
VMSTR IP_ADDRESS = "54.86.88.20"; //currently only support IP address
VMINT PORT = 8883;
char cafileName[] = "G5.pem";
char clientCRTName[] = "cert.pem";
char clientKeyName[] = "privatekey.pem";
/* end of user settings */

QoSLevel qos = QOS_0;
bool infinitePublishFlag;
char cPayload[100];
int32_t i;
int rc;
char HostAddress[255] = "data.iot.us-east-1.amazonaws.com";
uint16_t port = 8883;
uint32_t publishCount = 0;
LWiFiClient c;
VMSTR ca_path = cafileName;
VMSTR crt_file = clientCRTName;
VMSTR key_file = clientKeyName;

int32_t MQTTcallbackHandler(MQTTCallbackParams params) {

  Serial.println("Subscribe callback");
        Serial.print("Topic Name is and message is ");
        Serial.println(params.pTopicName);

//  INFO("%.*s\t%.*s",
//      (int)params.TopicNameLen, params.pTopicName,
//      (int)params.MessageParams.PayloadLen, (char*)params.MessageParams.pPayload);

  return 0;
}

void disconnectCallbackHandler(void) {
	Serial.println("MQTT Disconnect");
}

// invoked in main thread context
void bearer_callback(VMINT handle, VMINT event, VMUINT data_account_id, void *user_data)
{
    if (VM_BEARER_WOULDBLOCK == g_bearer_hdl)
    {
        g_bearer_hdl = handle;
    }
    
    switch (event)
    {
        case VM_BEARER_DEACTIVATED:
            break;
        case VM_BEARER_ACTIVATING:
            break;
        case VM_BEARER_ACTIVATED:
              rc = NONE_ERROR;
              i = 0;
              infinitePublishFlag = true;
  
              MQTTConnectParams connectParams;
  
              connectParams.KeepAliveInterval_sec = 10;
              connectParams.isCleansession = true;
              connectParams.MQTTVersion = MQTT_3_1_1;
              connectParams.pClientID = "CSDK-test-device";
              connectParams.pHostURL = HostAddress;
              connectParams.port = port;
              connectParams.isWillMsgPresent = false;
              connectParams.pUserName = NULL;
              connectParams.pPassword = NULL;
              connectParams.pRootCALocation = cafileName;
              connectParams.pDeviceCertLocation = clientCRTName;
              connectParams.pDevicePrivateKeyLocation = clientKeyName;
              connectParams.mqttCommandTimeout_ms = 2000;
	      connectParams.tlsHandshakeTimeout_ms = 5000;
	      connectParams.isSSLHostnameVerify = true;// ensure this is set to true for production
	      connectParams.disconnectHandler = disconnectCallbackHandler;
  
              rc = iot_mqtt_connect(&connectParams);
              if (NONE_ERROR != rc) {
            Serial.println("Error in connecting...");
              }
  
              MQTTSubscribeParams subParams;
              subParams.mHandler = MQTTcallbackHandler;
              subParams.pTopic = "mtktestTopic5";
              subParams.qos = qos;
              if (NONE_ERROR == rc) {
            Serial.print("Subscribing...");
            rc = iot_mqtt_subscribe(&subParams);
            if (NONE_ERROR != rc) {
          Serial.println("failed in iot_mqtt_subscribe.");
            }
              }
  
              MQTTMessageParams Msg;
              Msg.qos = QOS_0;
              Msg.isRetained = false;
              
              sprintf(cPayload, "%s : %d ", "hello from SDK", i);
              Msg.pPayload = (void *) cPayload;
              Msg.PayloadLen = strlen(cPayload) + 1;
  
              MQTTPublishParams Params;
              Params.pTopic = "mtktestTopic5";
              Params.MessageParams = Msg;

              if(publishCount != 0){
                  infinitePublishFlag = false;  
              }
  
              while (NONE_ERROR == rc && (publishCount > 0 || infinitePublishFlag)) {
              rc = iot_mqtt_yield(100);
                    delay(1000);
              Serial.println("-->sleep");
              delay(100);
              sprintf(cPayload, "%s : %d ", "hello from SDK", i++);
              rc = iot_mqtt_publish(&Params);
              if(publishCount > 0){
                        publishCount--;
              }
              }
  
              if(NONE_ERROR != rc){
              Serial.println("An error occurred in the loop.\n");
              }
              else{
              Serial.println("Publish done\n");
              }
              break;
        case VM_BEARER_DEACTIVATING:
            break;
        default:
            break;
    }
}

boolean bearer_open(void* ctx){
    g_bearer_hdl = vm_bearer_open(VM_BEARER_DATA_ACCOUNT_TYPE_WLAN,  NULL, bearer_callback);
    return true;
}

void setup()
{
  LWiFi.begin();
  Serial.begin(9600);
  while(!Serial)
    delay(100);

  // keep retrying until connected to AP
  Serial.print("  . Connecting to AP...");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
  }
  Serial.println("ok");
  
  read_time = 0;

  cert_path = ca_path;
  crt_p = crt_file;
  key_p = key_file;
  CONNECT_IP_ADDRESS = IP_ADDRESS;
  CONNECT_PORT = PORT;
  
  LTask.remoteCall(bearer_open, NULL);
}

void loop()
{
}
