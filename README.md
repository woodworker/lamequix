# LameQuix
A [lameboy](https://github.com/davedarko/LAMEBOY/) based mqtt pager

Setup a sd card with wifi.txt and mqtt.txt in the root folder

wifi.txt
 *  line 1: SSID
 *  line 2: password

mqtt.txt
 *  line 1: servername
 *  line 2: topic to subscribe to

## Featureset

- [x] Vibration Motor
- [x] incomming message notification via vibration
- [x] light up display to indicate unread message
- [x] message acknoledgement via button press
- [x] Last 4 Message storage
- [x] Basic Wifi Settings via SD card changable
- [x] MQTT Setting via SD card changable


## Hardware changes need

- a vibration motor
- a npn transistor
- [tutorial on hackaday.io](https://hackaday.io/project/26823-lameboy-another-esp12-handheld/log/120933-lameboy-mqtt-pager-aka-lamequix-by-woodworker)
