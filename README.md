Industrial Safety & Temperature Alarm System

Overview

An 8051-based industrial safety monitoring and temperature alarm system
designed and simulated in Proteus. The system uses an AT89C52,
ADC0808, LM35, 16×2 LCD, flame/gas/vibration/emergency
inputs, LEDs, and a transistor-driven buzzer.

Objectives

Measure temperature using an LM35.

Convert the LM35 analog output using ADC0808.

Display temperature and system status on a 16×2 LCD.

Monitor flame, gas, vibration, and emergency inputs.

Provide green, yellow, and red status indication.

Activate an audible alarm during critical conditions.

Implement a priority-based HIGH ALERT system.

Hardware

Component               Purpose

AT89C52                 Main microcontroller
ADC0808                 Analog-to-digital converter
LM35                    Temperature sensor
LM016L 16×2 LCD         Display
Flame input             Flame detection
Gas input               Gas detection
Vibration input         Vibration detection
Emergency input         Emergency detection
Green LED               Normal status
Yellow LED              Warning status
Red LED                 Alarm status
5 V active buzzer       Audible alarm
BC547                   Buzzer driver
R9 = 1 kΩ               BC547 base resistor
R10 = 10 kΩ             Base pull-down
P0 pull-ups = 10 kΩ     Port 0 pull-ups
Crystal = 11.0592 MHz   AT89C52 clock

Pin Mapping

Port 0 --- ADC0808 Data

P0.0 → D0

P0.1 → D1

P0.2 → D2

P0.3 → D3

P0.4 → D4

P0.5 → D5

P0.6 → D6

P0.7 → D7

Port 0 requires external pull-up resistors.

Port 1 --- Inputs and Outputs

P1.0 → Flame

P1.1 → Gas

P1.2 → Vibration

P1.3 → Emergency

P1.4 → Green LED

P1.5 → Yellow LED

P1.6 → Red LED

P1.7 → Buzzer driver

Port 2 --- LCD

P2.0 → RS

P2.1 → E

P2.4 → D4

P2.5 → D5

P2.6 → D6

P2.7 → D7

LCD is operated in 4-bit mode.

Port 3 --- ADC0808 Control

P3.0 → ALE

P3.1 → START

P3.2 → EOC

P3.3 → OE

Temperature Measurement

The LM35 provides approximately 10 mV/°C.

With ADC0808:

VREF+ = 5 V

VREF− = 0 V

8-bit resolution

The firmware calculates:

Temperature = ADC_Value × 500 / 256

Alarm Priority

The system uses the following priority:

HIGH ALERT

Flame alert

Gas alert

Vibration alert

Emergency alert

Temperature warning

Normal operation

HIGH ALERT

HIGH ALERT is activated when:

Temperature is 50°C or higher, OR

Two or more safety inputs are active.

Output:

Green LED OFF

Yellow LED OFF

Red LED ON

Buzzer ON

LCD examples:

HIGH ALERT
TEMP CRITICAL

or:

HIGH ALERT
MULTI SENSOR

Single Sensor Alerts

For one active sensor:

ALERT: FLAME!
FIRE DETECTED

ALERT: GAS!
GAS DETECTED

ALERT: VIBRATION
VIBRATION HIGH

EMERGENCY
BUTTON ACTIVE

Temperature Warning

For 30°C--49°C:

TEMP WARNING
TEMP: XX C

Yellow LED ON, buzzer OFF.

Normal

Below 30°C with no active safety input:

SYSTEM NORMAL
TEMP: XX C

Green LED ON, buzzer OFF.

Safety Input Logic

The current Proteus switches are active LOW:

Released = 1
Pressed  = 0

Therefore the firmware checks:

if(FLAME == 0)
if(GAS == 0)
if(VIBRATION == 0)
if(EMERGENCY == 0)

If actual sensor modules are connected later, verify their output
polarity because some modules are active HIGH.

Buzzer Circuit

The buzzer is driven through a BC547 NPN transistor:

AT89C52 P1.7 (Pin 8)
        |
       R9
      1 kΩ
        |
     BC547 Base
        |
       R10
      10 kΩ
        |
       GND

BC547 Emitter  → GND
BC547 Collector → Buzzer (-)
Buzzer (+)      → +5 V

Use a 5 V active buzzer.

P1.7 should drive the transistor through R9; the buzzer should not be
driven directly from the microcontroller pin.

Clock and ADC

AT89C52 clock:

11.0592 MHz

Crystal capacitors:

33 pF + 33 pF

The ADC0808 requires a clock on its CLOCK input; approximately 1 MHz
is suitable for the Proteus design.

Software

The firmware is written in C for the 8051 family and can be compiled
with Keil µVision.

Main software modules:

LCD initialization and display

ADC0808 conversion

LM35 temperature calculation

Safety input monitoring

LED control

Buzzer control

HIGH ALERT priority logic

Running the Project

Open the Proteus project.

Open the firmware in Keil.

Build/compile the project.

Load the generated HEX file into the AT89C52 in Proteus.

Set the AT89C52 clock to 11.0592 MHz.

Start the simulation.

Test each safety input individually.

Test multiple safety inputs together.

Test the LM35 temperature response.

Expected Results

Condition                   LCD              LED      Buzzer

No alarm, <30°C            SYSTEM NORMAL    Green    OFF
30--49°C                    TEMP WARNING     Yellow   OFF
One safety input            Specific alert   Red      ON
Two or more safety inputs   HIGH ALERT       Red      ON
All safety inputs           HIGH ALERT       Red      ON
Temperature ≥50°C           HIGH ALERT       Red      ON

Important Checks

Before simulation, verify:

AT89C52 VCC → +5 V

AT89C52 GND → GND

EA → +5 V

Crystal → 11.0592 MHz

P0 pull-up network is installed

ADC0808 VREF+ → +5 V

ADC0808 VREF− → GND

ADC0808 IN0 → LM35 output

ADC0808 clock is approximately 1 MHz

LCD RW → GND

LCD uses 4-bit mode

Safety switches are active LOW

BC547 emitter → GND

BC547 collector → buzzer negative

Buzzer positive → +5 V

R9 = 1 kΩ

R10 = 10 kΩ

Buzzer driver is connected to P1.7, not the reset circuit

Project Structure

Industrial-Safety-Alarm/
├── README.md
├── main.c
├── Proteus/
│   └── industrial_alarm_system.pdsprj
└── Firmware/
    └── alarm_system.hex

Applications

This project demonstrates concepts used in:

Industrial safety monitoring

Temperature monitoring

Embedded systems

8051 microcontroller interfacing

Analog sensor interfacing

ADC interfacing

LCD interfacing

Alarm and warning systems

License

This project is intended for educational and academic purposes.
