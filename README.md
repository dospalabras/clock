To build and install clock software:

1) Install arduino development application from https://www.arduino.cc/en/main/software
2) Run arduino development application and select the board manager in the Tool->Board->Board Manager menu
   and search for esp32 and install the esp32 board libraries.
3) git clone https://github.com/dospalabras/clock.git
4) Open Clock.ino in the arduino development application
5) Select WEMOS LOLIN from the Tools->Board menu
6) Select "No OTA (Large APP)" in the Tools->Partition Scheme menu
7) Plug in the clock into a USB port and select its port in the Tools->Port menu
8) Change code and compile and upload to clock using the Sketch->Upload menu

To set clock:

1) Download the Bluefruit application to your phone
2) Run Bluefruit and connect to the clock
3) Select the UART service
4) Type in the name of the wifi network
5) Type in the wifi network password.
