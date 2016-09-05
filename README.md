# SolderStation-ext
SolderStation for Weller RT tips, with nice features.

Based on schematics and software from https://github.com/ConnyCola/SolderingStation

Software 95% rewritten, new housing.

Video: https://github.com/MarcusKoe/SolderStation-ext/blob/master/SolderStation-ext.mp4

Extended version with:
- 4 buttons
- - Temperature presets - stored in eeprom
- RTC
- - Change time via Buttons
- Precise temperature settings via potentiometer
- 4 seconds temperature boost (target temperature +50 degrees for 4 seconds for heating up big wires etc.)
- Settings via config.h (Recompile to change it)
- - Multilanguage support - Germen and English
- - Different units - Celsius, Kelvin and Fahrenheit
- - Display brightness
- - Standby times etc.
- Temperature drop down when put tip in holder
- Inactivity warning after 30min 
- Piezo buzzer
- USB-Slot type A for light support (IKEA etc.)
- Melt fuse
- RGB-LED strip for status or cool animations - not 100% ready

Housing as 3D file

Look at the wiki pages: https://github.com/MarcusKoe/SolderStation-ext/wiki

ToDo:

- More docu at the wiki. Next weeks
- Change settings over menu is not implemented. It will be done the in next 1-3 month. 
- Lights and animations - WS2812b strip. It will be done in the next 1-3 month. 
