
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
#include <vmsock.h>
#include <net.h>
#include <mtk.h>

#include <signal.h>
#include <limits.h>
#include "mqtt_interface.h"
#include "iot_version.h"
#include "iot_shadow.h"
#include "json_utils.h"
#include "iot_log.h"

#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>

/* change Wifi settings here */
#define WIFI_AP "MUS"
#define WIFI_PASSWORD "mediatekmus"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.

/* change server settings here */
VMSTR IP_ADDRESS = "107.23.31.248"; //currently only support IP address
VMINT PORT = 8883;
int numPubs = 5;
char cafileName[] = "G5.pem";
char clientCRTName[] = "cert.pem";
char clientKeyName[] = "privatekey.pem";
VMSTR thingName = "mtktest2";
/* end of user settings */


QoSLevel qos = QOS_0;
bool infinitePublishFlag;
char cPayload[100];
int32_t i;
int rc;
char HostAddress[255] = IOT_PB_US_EAST_1;
uint16_t port = 8883;
uint32_t publishCount = 0;
double tempSensor[] = {73.1, 71.2, 68.7, 70.1, 75.4};
int test = 0;
uint8_t loopcnt = 0;
uint16_t pubcnt = 0;
uint8_t tempSensorIndex = 0;

LWiFiClient c;
VMSTR ca_path = cafileName;
VMSTR crt_file = clientCRTName;
VMSTR key_file = clientKeyName;

typedef struct {
  double temperature;
  bool windowOpen;
} ShadowReported;

ShadowReported reported;

typedef struct {
  bool windowOpen;
} ShadowDesired;

ShadowDesired desired;

char shadowTxBuffer[256];
char deltaBuffer[256];

void ShadowUpdateStatusCallback(void *pContext, Shadow_Ack_Status_t status) {
	if (status == SHADOW_ACK_TIMEOUT) {
		Serial.println("Update Timeout--");
	}
	if (status == SHADOW_ACK_REJECTED) {
		Serial.println("Update RejectedXX");
	}
	if (status == SHADOW_ACK_ACCEPTED) {
		Serial.println("Update Accepted !!");
	}
}

void windowActuate_Callback(const char *pJsonString, uint32_t JsonStringDataLen, primitiveJson_t *pContext) {
	Serial.print("Delta - Window state changed to ");
        Serial.println(*(bool *)(pContext->pData));

}


MQTTClient_t mqttClient;
char *pJsonStringToUpdate;
float temperature = 0.0;

bool windowOpen = false;
primitiveJson_t windowActuator;

primitiveJson_t temperatureHandler;

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
              
	      iot_mqtt_init(&mqttClient);

              windowActuator.cb = windowActuate_Callback;
              windowActuator.pData = &windowOpen;
              windowActuator.pKey = "windowOpen";
              windowActuator.type = SHADOW_JSON_BOOL;
              
              temperatureHandler.cb = NULL;
              temperatureHandler.pKey = "temperature";
              temperatureHandler.pData = &temperature;
              temperatureHandler.type = SHADOW_JSON_FLOAT;


              ShadowParameters_t sp;
	      sp.pUniqueThingId = thingName;
	      sp.pHost = HostAddress;
	      sp.port = port;
	      sp.pClientCRT = clientCRTName;
	      sp.pClientKey = clientKeyName;
	      sp.pRootCA = cafileName;
  
              Serial.print("  . Shadow Init... ");
              rc = iot_shadow_init(&mqttClient);
              if (NONE_ERROR != rc) {
                Serial.println("Error in connecting...");
              }
              Serial.println("ok");
              
              rc = iot_shadow_connect(&mqttClient, &sp);

	      if (NONE_ERROR != rc) {
		Serial.println("Shadow Connection Error");
	      }

              rc = iot_shadow_register_delta(&mqttClient, &windowActuator);
              
              if (NONE_ERROR != rc) {
		Serial.println("Shadow Register Delta Error");
	      }

              // loop and publish a change in temperature
	      while (NONE_ERROR == rc) {
		rc = iot_shadow_yield(&mqttClient, 2000);
		delay(1000);
		Serial.println("=======================================================================================");
		Serial.print("On Device: window state ");
                if (windowOpen)
                    Serial.println("true");
                else
                    Serial.println("false");
		// increment temperature randomly
		temperature += 1.23f;
                if (temperature > 20)
			windowOpen = true;
		pJsonStringToUpdate = iot_shadow_init_json_document();
		rc = iot_shadow_add_reported(pJsonStringToUpdate, 2, &temperatureHandler, &windowActuator);
		if (rc == NONE_ERROR) {
			iot_finalize_json_document(pJsonStringToUpdate);
			Serial.print("Update Shadow: ");
                        Serial.println(pJsonStringToUpdate);
			rc = iot_shadow_update(&mqttClient, pJsonStringToUpdate, ShadowUpdateStatusCallback, NULL, 4);
		}
		Serial.println("*****************************************************************************************");
	      }

	      if (NONE_ERROR != rc) {
		Serial.println("An error occurred in the loop.");
	      }

	      Serial.println("Disconnecting");
	      rc = iot_shadow_disconnect(&mqttClient);

	      if (NONE_ERROR != rc) {
		ERROR("Disconnect error");
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
  //while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  if (WIFI_AUTH == LWIFI_WPA){
      while (0 == LWiFi.connectWPA(WIFI_AP, WIFI_PASSWORD))
      {
          delay(1000);
      }
  }
  else if (WIFI_AUTH == LWIFI_WEP){
      while (0 == LWiFi.connectWEP(WIFI_AP, WIFI_PASSWORD))
      {
          delay(1000);
      }
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