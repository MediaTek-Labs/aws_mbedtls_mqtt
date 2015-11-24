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
#include "aws_iot_mqtt_interface.h"
#include "aws_iot_version.h"
#include "aws_iot_log.h"
#include "aws_mtk_iot_config.h"
#ifdef connect
#undef connect
#endif
#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
char HostAddress[255] = AWS_IOT_MQTT_HOST;
/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
VMINT port = AWS_IOT_MQTT_PORT;
/**
 * @brief This parameter will avoid infinite loop of publish and exit the program after certain number of publishes
 */
uint32_t publishCount = 0;

char cafileName[] = AWS_IOT_ROOT_CA_FILENAME;
char clientCRTName[] = AWS_IOT_CERTIFICATE_FILENAME;
char clientKeyName[] = AWS_IOT_PRIVATE_KEY_FILENAME;

LWiFiClient c;
bool infinitePublishFlag;
char cPayload[100];
int32_t i;
int rc;

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

MQTTConnectParams connectParams;
MQTTSubscribeParams subParams;
MQTTMessageParams Msg;
MQTTPublishParams Params;

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
              /************************ Add your code here ************************/ 
            
              rc = NONE_ERROR;
              i = 0;
              infinitePublishFlag = true;
  
              connectParams = MQTTConnectParamsDefault;
  
              connectParams.KeepAliveInterval_sec = 10;
              connectParams.isCleansession = true;
              connectParams.MQTTVersion = MQTT_3_1_1;
              connectParams.pClientID = "CSDK-test-device";
              connectParams.pHostURL = HostAddress;
              connectParams.port = port;
              connectParams.isWillMsgPresent = false;
              connectParams.pRootCALocation = cafileName;
              connectParams.pDeviceCertLocation = clientCRTName;
              connectParams.pDevicePrivateKeyLocation = clientKeyName;
              connectParams.mqttCommandTimeout_ms = 2000;
	      connectParams.tlsHandshakeTimeout_ms = 5000;
	      connectParams.isSSLHostnameVerify = true;// ensure this is set to true for production
	      connectParams.disconnectHandler = disconnectCallbackHandler;
  
              rc = aws_iot_mqtt_connect(&connectParams);
              if (NONE_ERROR != rc) {
                  Serial.println("Error in connecting...");
              }
  
              subParams = MQTTSubscribeParamsDefault;
              subParams.mHandler = MQTTcallbackHandler;
              subParams.pTopic = AWS_IOT_TOPIC_NAME;
              subParams.qos = QOS_0;
              
              if (NONE_ERROR == rc) {
                  Serial.print("Subscribing...");
                  rc = aws_iot_mqtt_subscribe(&subParams);
                  if (NONE_ERROR != rc) {
                      Serial.println("failed in iot_mqtt_subscribe.");
                  }
              }
 
              Msg = MQTTMessageParamsDefault;
              Msg.qos = QOS_0;
              Msg.isRetained = false;
              
              sprintf(cPayload, "%s : %d ", "hello from SDK", i);
              Msg.pPayload = (void *) cPayload;
              Msg.PayloadLen = strlen(cPayload) + 1;
  
              Params = MQTTPublishParamsDefault;
              Params.pTopic = AWS_IOT_TOPIC_NAME;
              Params.MessageParams = Msg;

              if(publishCount != 0){
                  infinitePublishFlag = false;  
              }
  
              while (NONE_ERROR == rc && (publishCount > 0 || infinitePublishFlag)) {
                    rc = aws_iot_mqtt_yield(1000); //please don't try to put it lower than 1000, otherwise it may going to timeout easily and no response  
                    Serial.println("-->sleep");
                    delay(1000);
                    sprintf(cPayload, "%s : %d ", "hello from SDK", i++);
                    rc = aws_iot_mqtt_publish(&Params);
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
              
              /************************ End for your own code ************************/ 
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

  CONNECT_IP_ADDRESS = IP_ADDRESS;
  CONNECT_PORT = port;
  
  LTask.remoteCall(bearer_open, NULL);
}

void loop()
{
}
