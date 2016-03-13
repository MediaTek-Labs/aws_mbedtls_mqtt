LinkIt ONE Connect to AWS IoT Services:
===================

This reporsitory contains a port of the mbed TLS library for use in LinkIt ONE applications to enable connectivity to AWS IoT services using the MQTT protocol.


Usage
-------------
Just download zip achieve of this repository and rest of steps and method of connecting with AWS were shown as bellow.

####Build image files
> **Verify Binary Steps**

> - Open your Arduino IDE and add this library **Sketch** -> **Include Library** -> **Add .ZIP Library** Select the zip file
> - Open example sketch **File** -> **Examples** -> **LinkIt One AWS mqtt library** -> **aws_paho_mqtt** or **aws_paho_shadow**
> - Verify it

#### Amazon Web Service IoT Setting

> **Create AWS IoT resource**

> - Login AWS and enter AWS IT console **Create a resource** -> **Create a thing ** -> **Fill up Name field** -> **Press Create**
> - Create Policy and certicate **Select the resource you just created** -> **Press connect a device** -> **Embedded C** -> **Generate certificate and policy**
> - There are 3 hyperlinks on the page and you must Keep all **keys** (Especially private keys)
> - Make certificate in ACTIVE status **Select the certificate** -> **Action** -> **Active**

#### Operations on LinkIt One

> **Connecting LinkIt ONE with AWS**

> - Put certifications into LinkIt One **Tick DIP switch from UART to MS nearby USB slot on the board** -> **Connecting USB to Computer ** -> **Copy 3 keys to root directory**
> - Root CA in tools\root_cert, Private key and public key are what you downloaded from AWS in section (*-private.pem.key for private key and *-public.pem.key for public key)
> - Modify header file **aws_mtk_iot_config.h**


```c
// Get from console
// =================================================
#define AWS_IOT_MQTT_HOST              "*.iot.us-west-2.amazonaws.com"
// Customer specific MQTT HOST. The same will be used for Thing Shadow
// Click the resource and it will show in REST API endpoint field
#define AWS_IOT_MQTT_PORT              8883
// default port for MQTT/S and noneed to change
#define AWS_IOT_MQTT_CLIENT_ID         "NTHU_LinkitOne"
// MQTT client ID should be unique for every device
// Just don't make your devices in the same any name you like is ok
#define AWS_IOT_MY_THING_NAME 	       "mtk_test_mf"
// IMPORTANT: for a temporary work around, you also need to modify the same name in your library for aws_iot_config.h (under arduino library path)
// Just copy your resource name in this field and it no need to change aws_iot_config.h
// At least no error shown in this repo
#define AWS_IOT_ROOT_CA_FILENAME       "root.pem"
// Root CA file name
// Check the file name one more time and make sure it's the same
#define AWS_IOT_CERTIFICATE_FILENAME   "8cbd725746-certificate.pem.crt"
// device signed certificate file name
#define AWS_IOT_PRIVATE_KEY_FILENAME   "8cbd725746-private.pem.key"
// Device private key filename
// =================================================
```

> - Modify the WIFI connecting information


```c
//set to use Wifi or GPRS
#define WIFI_USED true  //true (Wifi) or false (GPRS)

/* change Wifi settings here */
#define WIFI_AP "Home-WIFI"
#define WIFI_PASSWORD "Passw0rd"
#define WIFI_AUTH LWIFI_WPA  
// choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP
// This is very important, remember to check it again
```
> - Verify and Upload
> - Check your AWS console the messages should be shown up


----------
More Information
-------------
Getting started please visit:
https://labs.mediatek.com/site/global/developer_tools/mediatek_linkit/get-started/aws/introduction/index.gsp

Hardware details of LinkIt ONE for AWS IoT:
https://labs.mediatek.com/site/global/developer_tools/mediatek_linkit/hdk_intro/aws_kit/index.gsp

More on Amazon IoT is available here:
http://aws.amazon.com/iot/getting-started

Amazon maraca example:
http://iot-hackseries.s3-website-us-west-2.amazonaws.com/linkitone.html



Notice
-------------
The serial monitor problem should be resolve after SDK version 1.1.17. If you still have problem for showing logs in monitor tool, please try to replace platform.txt in the following locations with the platform.txt file included in the zip archive above
**(This issue should be resolved in the latest SDK):**

Mac:

Arduino 1.5.7: /Applications/Arduino.app/Contents/Resources/Java/hardware/arduino/mtk/platform.txt

Arduino 1.6.5: /Users/{YOUR_USERNAME}/Library/Arduino15/packages/LinkIt/hardware/arm/1.1.17/platform.txt

Windows:

Arduino 1.5.7: Arduino/hardware/arduino/mtk/platform.txt

Arduino 1.6.5: C:\Users<username>\AppData\Roaming\Arduino15\packages\LinkIt\hardware\arm\1.1.X


----------
Release Note
-------------
2/22/2016 update:

 1. Added feature for resolving AWS server domain name automatically

 2. Supported GPRS connection

 3. Added example code for reading value from sensor periodically

 4. Fixed unstable issue for long time running
