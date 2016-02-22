LinkIt ONE Connect to AWS IoT Services:

This reporsitory contains a port of the mbed TLS library for use in LinkIt ONE applications to enable connectivity to AWS IoT services using the MQTT protocol.

For details on getting started please visit: https://labs.mediatek.com/site/global/developer_tools/mediatek_linkit/get-started/aws/introduction/index.gsp

Details on the LinkIt ONE kit for AWS can be found on this page: https://labs.mediatek.com/site/global/developer_tools/mediatek_linkit/hdk_intro/aws_kit/index.gsp

More on Amazon IoT is available here: http://aws.amazon.com/iot/getting-started

Amazon maraca example:
http://iot-hackseries.s3-website-us-west-2.amazonaws.com/linkitone.html




Release Note for 2/22/2016 update:

a.) Added feature for resolving AWS server domain name automatically

b.) Supported GPRS connection

c.) Added example code for reading value from sensor periodically

d.) Fixed unstable issue for long time running




Note: If you have problem for showing logs in monitor tool, please try to replace platform.txt in the following locations with the platform.txt file included in the zip archive above (This issue should be resolved in the latest SDK):

Mac:

Arduino 1.5.7: /Applications/Arduino.app/Contents/Resources/Java/hardware/arduino/mtk/platform.txt

Arduino 1.6.5: /Users/{YOUR_USERNAME}/Library/Arduino15/packages/LinkIt/hardware/arm/1.1.17/platform.txt

Windows:

Arduino 1.5.7: Arduino/hardware/arduino/mtk/platform.txt

Arduino 1.6.5: C:\Users<username>\AppData\Roaming\Arduino15\packages\LinkIt\hardware\arm\1.1.X
