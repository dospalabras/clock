To set clock:

1) Install the Bluefruit application to your phone.
2) Run the Bluefruit application and connect to the clock.
3) Select the UART service.
4) Type in the name of the wifi network.
5) Type in the wifi network password. 

To build and install clock software:

1) Download and install the Arduino IDE from https://www.arduino.cc/en/main/software
2) Run the Arduino IDE.
3) Add https://dl.espressif.com/dl/package_esp32_index.json into the Arduino->Preferences Additional Board Manager URLs.
4) Select the board manager in the Tool->Board->Board Manager menu and search for esp32 and install the esp32 board libraries.
5) Select and install NeoPixelBus by Makuna library from the Sketch->Include Library->Manage Libraries... menu
6) git clone https://github.com/dospalabras/clock.git
7) Open Clock.ino in the Arduino IDE using the File->Open menu
8) Select WEMOS LOLIN from the Tools->Board menu
9) Select "No OTA (Large APP)" in the Tools->Partition Scheme menu
10) Plug the clock into a USB port and select its port in the Tools->Port menu
11) Change code and compile and upload to clock using the Sketch->Upload menu
12) To debug, add Serial.println statements in the code and start the serial monitor from the Tools->Serial Monitor menu and use 115200 baud.
