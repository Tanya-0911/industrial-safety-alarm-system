# Industrial Safety Alarm System

An **AT89S52 (8051)-based industrial safety monitoring prototype** that reads temperature and digital hazard inputs, calculates a real-time **Safety Index (0–100)**, classifies alarms independently of the score, and shows status and hazard information on a single **16×2 LCD**.

Developed in **Embedded C (Keil C51)** for simulation with **Proteus**.

> **Safety notice:** This is an educational prototype, not a certified industrial safety system. Its thresholds and weights are illustrative. Do not use it as a substitute for independently engineered emergency-stop, fire, gas, or machinery-protection equipment.

## Features

- LM35 temperature measurement through an ADC0808.
- Active-low digital inputs for flame, gas, vibration, and emergency switch.
- Real-time numerical Safety Index with per-sensor risk deductions.
- Separate **SAFE / PRE-WARNING / WARNING / DANGER / EMERGENCY** alarm classification.
- Pre-warning at 30–39 °C; warning at 40–49 °C; critical temperature at 50 °C and above.
- Green, yellow, and red LEDs plus a buzzer for visual/audible indications.
- Two LCD views on **one 16×2 display**: index/status and sensor/hazard details.
- **Timer 0** requests an LCD view change approximately every 2 seconds, without a 2-second delay in the main monitoring loop.
- Newly detected critical hazards can immediately switch the display to hazard details.

## Hardware and software

| Component | Role |
| --- | --- |
| AT89S52 | Main 8051 microcontroller |
| LM35 | Temperature sensing |
| ADC0808 | Analog-to-digital conversion for LM35 |
| Flame sensor/module | Digital flame detection |
| Gas sensor/module | Digital gas-threshold indication |
| Vibration sensor/module | Digital vibration-threshold indication |
| Emergency switch | Manual emergency input |
| 16×2 LCD | Index, status, temperature, and hazard messages |
| 3 LEDs and buzzer | Status and alarm indication |
| 11.0592 MHz clock | Timing assumed by the source |
| Keil µVision / C51 | Build firmware and generate HEX |
| Proteus | Simulate the circuit |

## Pin mapping

The pin assignments below are taken from the firmware and should match the Proteus schematic.

| Signal | AT89S52 pin/port bit |
| --- | --- |
| ADC0808 8-bit data bus | P0 (all 8 bits) |
| ADC ALE | P3.0 |
| ADC START | P3.1 |
| ADC EOC | P3.2 |
| ADC OE | P3.3 |
| LCD RS / EN | P2.0 / P2.1 |
| LCD D4–D7 | P2.4–P2.7 |
| Flame / Gas / Vibration / Emergency | P1.0 / P1.1 / P1.2 / P1.3 |
| Green / Yellow / Red LEDs / Buzzer | P1.4 / P1.5 / P1.6 / P1.7 |

**Input logic:** the four digital safety inputs are **active low** (`0` = detected/pressed, `1` = inactive). Output polarity follows the existing circuit and firmware; check your Proteus LED and buzzer wiring.

## Safety Index calculation

The score starts at 100 and deducts points for observed conditions:

| Condition | Points deducted |
| --- | ---: |
| Temperature below 30 °C | 0 |
| Temperature 30–39 °C | 10 |
| Temperature 40–49 °C | 20 |
| Temperature 50 °C or higher | 30 |
| Flame detected | 30 |
| Gas detected | 25 |
| Abnormal vibration detected | 15 |
| Emergency switch pressed | **Force score to 0** |

```text
Safety Index = max(0, 100 - TemperatureRisk - FlameRisk - GasRisk - VibrationRisk)
```

For example, at **45 °C with gas detected and no other hazards**, the index is `100 - 20 - 25 = 55`.

The Safety Index is **not** itself the alarm category. A single flame detection, for example, deducts 30 points (score 70) but still triggers **DANGER**.

## Alarm classification

The firmware applies these rules in priority order:

| Priority | Condition | Classification |
| --- | --- | --- |
| 1 | Emergency switch active | EMERGENCY |
| 2 | Temperature ≥ 50 °C, flame detected, or gas detected | DANGER |
| 3 | Temperature ≥ 40 °C or vibration detected | WARNING |
| 4 | Temperature ≥ 30 °C | PRE-WARNING |
| 5 | None of the above | SAFE |

The existing firmware calls `alarm_condition()` for DANGER/EMERGENCY, `warning_condition()` for WARNING/PRE-WARNING, and `normal_condition()` for SAFE. The exact LED output levels are defined by those functions in the source.

## LCD display and Timer 0

**View 1 — Index and classification**

```text
SAFETY IDX: 090
STATUS: PREWARN
```

**View 2 — Hazard details**

```text
TEMP PRE-WARN
TEMP: 35 C
```

Timer 0 is configured in 16-bit Mode 1 for an approximately 10 ms interrupt at **11.0592 MHz** (`TH0 = 0xDC`, `TL0 = 0x00`). After 200 interrupts, the interrupt service routine sets a flag. The **main loop** handles the LCD change; LCD operations are not performed inside the ISR. Actual timing is approximate due to interrupt/reload overhead.

When a new critical condition is detected, the firmware prioritizes showing the hazard view instead of waiting for the next normal screen change.

## How the firmware works

```text
Initialize ports, LCD, and Timer 0
                |
                v
      Read ADC + digital inputs
                |
                v
       Calculate Safety Index
                |
                v
     Classify alarm separately
                |
                v
        Update LEDs + buzzer
                |
                v
    Update LCD when data changes,
  screen timer expires, or new danger
                |
                +---- repeat ----+
```

Main functions include `get_temperature()`, `calculate_index()`, `classify_alarm()`, `timer0_init()`, `timer0_isr()`, `display_index_status()`, and `display_sensor_details()`.

## Build and run

1. Open your **Keil µVision** project for the **AT89S52**.
2. Add the source file (for example, `alarm_system.c`) to the project, or replace the previous main source with the updated version. **Do not include two source files defining `main()`.**
3. Set the target clock to **11.0592 MHz** and configure an appropriate program memory size. If a 2 KB code-size setting causes a linker/vector issue, use a target configuration that supports the program's actual size.
4. In **Options for Target → Output**, enable **Create HEX File**.
5. Select **Project → Rebuild all target files** and verify a successful build.
6. In **Proteus**, load the generated HEX file into the AT89S52 and use the matching clock setting.
7. Run the simulation and vary temperature or toggle the active-low hazard inputs to observe the index, alarm outputs, and LCD messages.

## Suggested tests

| Test | Expected result |
| --- | --- |
| Temperature below 30 °C; no active sensors | Index 100, SAFE |
| Temperature 35 °C | Index 90, PRE-WARNING |
| Temperature 45 °C | Index 80, WARNING |
| Temperature 50 °C | Index 70, DANGER |
| Flame active at normal temperature | Index 70, DANGER |
| Gas active at normal temperature | Index 75, DANGER |
| Vibration active at normal temperature | Index 85, WARNING |
| Emergency active | Index 0, EMERGENCY |
| Normal operation for ~2 seconds | LCD view changes |

These are **expected firmware behaviors**, not a record of completed physical validation.

## Limitations and future work

- Current pre-warning uses a **temperature threshold**, not prediction from a rising-temperature trend.
- The gas and vibration inputs are **binary threshold signals**, not calibrated ppm or vibration amplitude measurements.
- ADC conversion waits for EOC; consider adding a timeout and sensor-fault handling.
- Emergency behavior is implemented in firmware; real emergency-stop protection requires independent, fail-safe hardware and appropriate latching/reset behavior.
- Add sensor filtering, hysteresis, event logging, trend-based warnings, and calibration for a more robust future version.
- Consider a serial dashboard or data logger for safety history and trend visualization.

## Repository organization (suggested)

```text
industrial-safety-alarm/
├── README.md
├── alarm_system.c        # Keil C51 firmware
├── proteus/              # Optional schematic/simulation files
└── screenshots/           # Optional circuit and LCD screenshots
```

## Project status

The firmware has been reported as **building and working in the user's Keil/Proteus setup**. No independent hardware certification or comprehensive validation is claimed.

## License

No license has been specified yet. Add a `LICENSE` file if you want to explicitly define reuse permissions.
