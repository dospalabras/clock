1) Install arduino development application from https://www.arduino.cc/en/main/software
2) Run arduino development application and select the board manager in the Tool->Board->Board Manager menu
   and search for esp32 and install the esp32 board libraries.
3) git clone https://github.com/dospalabras/clock.git
4) Open Clock.ino in the arduino development application
5) Select WEMOS LOLIN from the Tools->Board menu
6) Select "No OTA (Large APP)" in the Tools->Partition Scheme menu
7) Plug in the clock into a USB port and select its port in the Tools->Port menu
8) Change code and upload to clock using the Sketch->Uploade menu
