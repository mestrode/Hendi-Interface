# Hendi-Interface
remote interface for inductive cooking plate (Hendi 3500M)

The control knob will be used for interaction w/ the user. The pot signal will be modified by the electronic, regarding to the wish of the user (and if selected by the remote value).

![Image: external interface](https://raw.githubusercontent.com/mestrode/Hendi-Interface/master/hardware/housing.JPG)

You can use a simple switch/relay to enable heating (activate one of two fixed power values)
Or you can use the RS232 interface for communication, and select the power value stepless.

There are 5 operation modes, you chose on the control know:
- off/standby (this cooking plate hasn't really a power off-switch - but in this state its not possible to enable heating)
- remote on/off with low power (e.g. 1000W, configurable) by external switch/relay
- remote on/off with high power (e.g. 3500W, configurable) by external switch/relay
- remote control (0W, 500W up to 3500W) by RS232 protocol
- manual mode for easy standalone operation (knob can be used across the whole range, like w/o modification)

see also: https://hobbybrauer.de/forum/viewtopic.php?f=58&t=1732 (german language)

## Hardware

![Image: interface PCB](https://raw.githubusercontent.com/mestrode/Hendi-Interface/master/hardware/PCB.jpg)

- offers RS232 interface with 9600baud for external control
- on/off remote can be uses by simple external switch/relay w/o RS232 counterpart station
- galvanic isolation of electrical interface (no hazard by touching the external interface!)
- interface PCB is looped just between the 4 wire connector between display PCB and the control knob PCB 
- mechanical power switch of control knob overrules every manipulation of electronic - Off state is as secure as w/o this electronic. This ensures a bug can not lead into unwanted heating.
- Analog value of control knob are passed through or modified by the firmware regarding selected operation mode.
- heating power is displayed on the common numeric display
- two LEDs indicate the operation state
- electronic is powered by the cooking plate itself

## Firmware
- electronic uses a simple AVR 8-bit microcontroller
- easy remote protocol uses messages
 - Messagetype: Preamble, length of Payload, Payload, checksum (simple xor), delimiter
- needs periodic messages (~2.5 sec) to ensure save state if electrical connection is lost

## Disclaimer

Everything you do, you do at your own risk.
Keep all safety rules in mind! Be aware of electrical shock or other hazards!