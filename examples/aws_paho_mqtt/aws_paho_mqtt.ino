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
//#include <net.h>
//#include <mtk.h>

#include <signal.h>
#include <limits.h>
/*#include "aws_iot_mqtt_interface.h"
#include "aws_iot_version.h"
#include "aws_iot_log.h"*/
#include "linkit_aws_header.h"
#include "aws_mtk_iot_config.h"
#ifdef connect
#undef connect
#endif
#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LGPRS.h>

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
//char cPayload[100];
int32_t i;
int rc;
QoSLevel qos = QOS_0;
char mqtt_message[30];
bool doingSetup = true;

int32_t MQTTcallbackHandler(MQTTCallbackParams params) {

  Serial.println("Subscribe callback");
        Serial.print("Topic Name is and message is ");
        Serial.println(params.pTopicName);
        Serial.flush();

//  INFO("%.*s\t%.*s",
//      (int)params.TopicNameLen, params.pTopicName,
//      (int)params.MessageParams.PayloadLen, (char*)params.MessageParams.pPayload);

  return 0;
}

void disconnectCallbackHandler(void) {
  Serial.println("MQTT Disconnect");
        Serial.flush();
}

MQTTConnectParams connectParams;
MQTTSubscribeParams subParams;
MQTTMessageParams Msg;
MQTTPublishParams Params;
int subscribe_MQTT(int32_t (*callbackHandler)(MQTTCallbackParams), char * sTopic);

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
              LTask.post_signal();
              break;
        case VM_BEARER_DEACTIVATING:
            break;
        default:
            break;
    }
}

/* start the mqtt connection to AWS server and subscribe the topic*/
boolean  mqtt_start(void* ctx)
{
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
  
              rc = subscribe_MQTT(MQTTcallbackHandler, "mtktestTopic5");
              delay(1000);
              
              return true;
}

boolean bearer_open(void* ctx){
    if (WIFI_USED)
        g_bearer_hdl = vm_bearer_open(VM_BEARER_DATA_ACCOUNT_TYPE_WLAN ,  NULL, bearer_callback);
    else
        g_bearer_hdl = vm_bearer_open(VM_APN_USER_DEFINE ,  NULL, bearer_callback);
    if(g_bearer_hdl >= 0)
        return true;
    return false;
}

/* Resolve IP address for AWS server */
VMINT wifiResolveCallback(vm_soc_dns_result *pDNS)
{
  //C_ADDRESS = (const char*)&pDNS->address[0];
  IN_ADDR addr;
  addr.S_un.s_addr = pDNS->address[0];
  CONNECT_IP_ADDRESS = inet_ntoa(addr);
//  Serial.println("wifiResolveCallback");
//  Serial.print("ip address is ");
//  Serial.println(CONNECT_IP_ADDRESS);
  LTask.post_signal();
  return 0;
}

boolean wifiResolveDomainName(void *userData)
{
  VMCHAR *domainName = (VMCHAR *)userData;
  vm_soc_dns_result dns;
  IN_ADDR addr;
  
//  Serial.print("in wifiResolveDomainName, host name is ");
//  Serial.println(domainName);

  VMINT resolveState;
  if (WIFI_USED){
        resolveState = vm_soc_get_host_by_name(VM_TCP_APN_WIFI,
                                 domainName,
                                 &dns,
                                 &wifiResolveCallback);
      Serial.flush();
  }
  else{
      Serial.flush();
        resolveState = vm_soc_get_host_by_name(6,
                                 domainName,
                                 &dns,
                                 &wifiResolveCallback);
      Serial.flush();
  }
                           
  if (resolveState > 0)
  {
    // not done yet
    return false;
  }

  switch (resolveState)
  {
  case VM_E_SOC_SUCCESS:  // Get IP address successfully, result is filled.
    addr.S_un.s_addr = dns.address[0];
    CONNECT_IP_ADDRESS = inet_ntoa(addr);
    Serial.print("ip address is ");
    Serial.println(CONNECT_IP_ADDRESS);

    return true;
  case VM_E_SOC_WOULDBLOCK:  // wait response from network, result could be gotten from callback.
    // need to wait, return directly
    // so MMI message loop may continue.
    return false;
  case VM_E_SOC_INVAL:  // invalid arguments: null domain_name, etc.
  case VM_E_SOC_ERROR:  // unspecified error
  case VM_E_SOC_LIMIT_RESOURCE:  // socket resources not available
  case VM_E_SOC_INVALID_ACCOUNT:  // invalid data account id
    return true;
  }
}

/* Main setup function */
void setup()
{
  LTask.begin();
      
  Serial.begin(9600);
  while(!Serial)
    delay(100);

  // keep retrying until connected to AP
  if (WIFI_USED){
    LWiFi.begin();
    Serial.print("  . Connecting to AP...");
    Serial.flush();
    while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
    {
      delay(1000);
    }
  }
  else{  
    Serial.print("  . Connecting to GPRS...");
    Serial.flush();
    while (!LGPRS.attachGPRS(GPRS_APN, GPRS_USERNAME, GPRS_PASSWORD))
    {
      delay(500);
    }
  }

  Serial.println("ok");
  i = 1;

  LTask.remoteCall(&wifiResolveDomainName, (void*)HostAddress);
  CONNECT_PORT = port;
//  CONNECT_IP_ADDRESS = "52.1.16.3";
  
  LTask.remoteCall(&bearer_open, NULL);
  LTask.remoteCall(&mqtt_start, NULL);
}

/* for analogRead or other API, some may not be able to called in remoteCall. 
You may need to call it in loop function and then pass the parameter to nativeLoop through a pointer. */
void loop()
{
//    int aa[1];
//    aa[0] =analogRead(A0);
    Serial.flush();
//    LTask.remoteCall(nativeLoop, (void *)aa);  //pass analogRead(A0) value to nativeLoop, in that analogRead could not be called in remoteCall, otherwise it will cause a crash.
    LTask.remoteCall(nativeLoop, NULL);
    delay(2000);
}

/* subscribe MQTT function */
int subscribe_MQTT(int32_t (*callbackHandler)(MQTTCallbackParams), char * sTopic) {
  int rc = NONE_ERROR;
  MQTTSubscribeParams subParams;
  subParams.mHandler = callbackHandler;
  subParams.pTopic = sTopic;
  subParams.qos = qos;


  Serial.print("Subscribing...");
  Serial.flush();
  rc = aws_iot_mqtt_subscribe(&subParams);
  if (NONE_ERROR != rc) {
    Serial.println("failed in iot_mqtt_subscribe.");
  }
  Serial.println("Ok");
  Serial.flush();
  return rc;
}

/* publish MQTTT function */
int publish_MQTT(char * topic, char * message) {
  int rc = NONE_ERROR;

  // Message setup
  MQTTMessageParams Msg;
  Msg.qos = QOS_0;
  Msg.isRetained = false;
  Msg.pPayload = (void *) message;
  Msg.PayloadLen = strlen(message);
  Serial.print("publishMQTT - Payload length: ");
  Serial.println(Msg.PayloadLen);
  Serial.flush();

  // Publish setup
  MQTTPublishParams Params;
  Params.pTopic = topic;
  Params.MessageParams = Msg;

  rc = aws_iot_mqtt_yield(1000); //please don't try to put it lower than 1000, otherwise it may going to timeout easily and no response  
  Serial.println("-->sleep");
  delay(1000);

  // Publish
  Serial.print("Publishing...");
  rc = aws_iot_mqtt_publish(&Params);

  if(NONE_ERROR != rc){
    Serial.print("error and rc is ");
    Serial.println(rc);
  }
  else {
    Serial.println("Ok");
  }
  Serial.flush();
  return rc;
}

/* natvieLoop which will use to do your main job in the loop */
boolean nativeLoop(void* user_data) {
    
//    int *bb = (int*)user_data;
    sprintf(mqtt_message, "%s : and read valie is %d ", "hello from SDK", i++);
    
    Serial.println("publish_MQTT go");
    rc = publish_MQTT("mtktestTopic5", mqtt_message);
    Serial.flush();
}