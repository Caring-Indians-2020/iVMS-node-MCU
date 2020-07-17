# PatientMonitor
Patient Monitoring System for Covid-19 patients: Measures Respiratory rate, BP, ECG, oxygen saturation

Usage Guide

Setp 1: Loading the HTML pages into the Flash
	i. Instal Arduino ESP8266 Filesystem Uploader: https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/
	ii. In the Tools menu check that you have the option “ESP8266 Sketch Data Upload“.
	iii. Following the instructions in the page above, load the index.html, index_retry.html and pos.html files into the Flash

Step 2: Copy all libraries to your Arduino Path
Source: PatientMonitor\ArduinoSketches\Libraries
Destination:C:\Users\<usrName>\Documents\Arduino\libraries

Step 3: Open PatientMonitor\ArduinoSketches\iVMS\iVMS.ino in Arduino IDE.. Compile and load to ESP8266.