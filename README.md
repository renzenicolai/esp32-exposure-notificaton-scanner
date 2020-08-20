# Exposure notification beacon scanner
Simple exposure notification scanner application for ESP32 devices.

# Output

The application outputs the exposure notification advertising payload as a JSON string.

```
{"mac":"00:01:02:03:04:05", "rssi":-83, "rpi":"d3adb33f133713371337133713371337", "aem":"deadbeef"}
```

The JSON object contains the following fields:

 - MAC (Bluetooth address, 6 bytes)
 - RSSI (signal strength)
 - RPI (Rolling Proximity Identifier, 16 bytes)
 - AEM (Associated Encrypted Metadata, 4 bytes)
 
 Note that the MAC address is the MAC address transmitted by the bluetooth device that sent the beacon packet. This MAC address is randomized and will change whenever the RPI changes.

# How to build
Install the ESP32 toolchain and ESP-IDF version 4.2, then create a folder inside this repository, for example ```build```. From within this folder run ```cmake ..``` followed by ```make -j 4 flash monitor``` to build the software using 4 threads, flash it to an ESP32 board and start the monitor function.

# Specification of the exposure notifications

https://covid19-static.cdn-apple.com/applications/covid19/current/static/contact-tracing/pdf/ExposureNotification-BluetoothSpecificationv1.2.pdf

# License
Copyright 2020 Renze Nicolai

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
