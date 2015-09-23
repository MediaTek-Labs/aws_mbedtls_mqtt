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
VMSTR IP_ADDRESS = "54.88.127.209"; //currently only support IP address
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

void ShadowCallbackHandler(void) {
	IoT_Error_t rc = NONE_ERROR;
	int ret;

	Serial.print("My State Changed and delta: ");
	Serial.println(deltaBuffer);

	// init json parser
//	jsmn_init(&json_parser);
//
//	ret = jsmn_parse(&json_parser, deltaBuffer, strlen(deltaBuffer), tokens, sizeof(tokens) / sizeof(tokens[0]));
//
//	if (ret < 0) {
//		WARN("Failed to parse JSON: %d", ret);
//	}
//
//	// Assume the top-level element is a JSON object
//	if (ret < 1 || tokens[0].type != JSMN_OBJECT) {
//		WARN("Error: object expected");
//		return;
//	}
//
//	int version = 0;
//	int tIndex = 0;
//
//	// JSON --> struct
//	for (tIndex = 1; tIndex < ret; tIndex++) {
//		if (jsoneq(deltaBuffer, &tokens[tIndex], "window_open") == 0) {
//			rc = parseBooleanValue(&desired.windowOpen, deltaBuffer, tokens+tIndex+1);
//			INFO("Now %s window.", desired.windowOpen ? "opening" : "closing");
//			// for now just make "reported" mirror desired
//			reported.windowOpen = desired.windowOpen;
//		}
//	}
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
              
              ShadowParameters sp;
	      sp.pThingId = thingName;
	      sp.pHost = HostAddress;
	      sp.port = port;
	      sp.pClientCRT = clientCRTName;
	      sp.pClientKey = clientKeyName;
	      sp.pRootCA = cafileName;
	      sp.stateChangeHandler = ShadowCallbackHandler;
	      sp.deltaBuffer = deltaBuffer;
  
              Serial.print("  . Shadow Init... ");
              rc = iot_shadow_init(&sp);
              if (NONE_ERROR != rc) {
                Serial.println("Error in connecting...");
              }
              Serial.println("ok");
              
              rc = iot_shadow_connect();

	      if (NONE_ERROR != rc) {
		Serial.println("Shadow Connection Error");
	      }
              
              // reported values
	      reported.temperature = 75.5;
	      reported.windowOpen = false;
              test = 0;
  
              loopcnt = 0;
	      pubcnt = 0;
	      tempSensorIndex = 0;
	      

              // loop and publish a change in temperature
	      while (NONE_ERROR == rc && pubcnt < numPubs) {

		rc = iot_shadow_yield(100);
		delay(1000);

		// publish every 5 seconds
		if (loopcnt == 4) {
			loopcnt = 0;

			// update temperature "sensor" value
			reported.temperature = tempSensor[tempSensorIndex];
        
			// update index into "sensor" array - loop if > 5 pubs
			tempSensorIndex = tempSensorIndex >= 4 ? 0 : tempSensorIndex+1;
			Serial.print("Current Temperature: ");
                        Serial.println(reported.temperature);
                        Serial.print("window option now is ");
                        Serial.println(reported.windowOpen);

			// struct --> JSON
			sprintf(shadowTxBuffer, "{\"temperature\":\"%.1lf\",\"window_open\":%s}",
					reported.temperature, (reported.windowOpen ? "true" : "false"));

			rc = iot_shadow_sync_reported(shadowTxBuffer);
			pubcnt++;
		} else {
			loopcnt++;
		}
	      }

              if (NONE_ERROR != rc) {
		Serial.println("error in the final return part");
	      }

	      rc = iot_shadow_disconnect();

	      if (NONE_ERROR != rc) {
		Serial.println("error in disconnect");
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
