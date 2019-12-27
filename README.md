To build and install clock software:

1) Download and install the Arduino IDE from https://www.arduino.cc/en/main/software
2) Run the Arduino IDE
3) Add https://dl.espressif.com/dl/package_esp32_index.json into the Arduino->Preferences Additional Board Manager URLs
4) Select the board manager in the Tool->Board->Board Manager menu and search for esp32 and install the esp32 board libraries.
5) Select and install NeoPixelBus by Makuna library from the Sketch->Include Library->Manage Libraries... menu
6) Select and install ESP32 BLE Arduino library from the Sketch->Include Library->Manage Libraries... menu
7) Select and install Wifi library from the Sketch->Include Library->Manage Libraries... menu
8) Select and install Time library from the Sketch->Include Library->Manage Libraries... menu
9) git clone https://github.com/dospalabras/clock.git
10) Open Clock.ino in the Arduino IDE using the File->Open menu
11) Select WEMOS LOLIN from the Tools->Board menu
12) Select "No OTA (Large APP)" in the Tools->Partition Scheme menu
13) Plug the clock into a USB port and select its port in the Tools->Port menu
14) Change code and compile and upload to clock using the Sketch->Upload menu
15) To debug, add Serial.println statements in the code and start the serial monitor from the Tools->Serial Monitor menu and use 115200 baud.

To set clock:

1) Install the Bluefruit application to your phone
2) Run the Bluefruit application and connect to the clock
3) Select the UART service
4) Type in the name of the wifi network
5) Type in the wifi network password.
