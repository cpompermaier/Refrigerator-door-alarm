# Refrigerator-door-alarm
Alarm to detect that a door is open. Works with Arduino hardware.

Modify the switch pin, default is 3:
SwitchPin = 3;           // The switch is connect to pin 3

Modify the buzzer pin, default is 11:
piezoPin = 11;           // Buzzer in the pin 11

The time it will wait before start bipping
TimeWait = 60000;       // Time (ms) after the door is open to beep the buzzer