# Password Protected Door Lock With Emergency Hydro-lock

An embedded access control system featuring a 4x4 matrix keypad input, automated door control using a shift register, and a prioritized, emergency escape routine triggered by liquid detection of the water sensor.

### System Demonstration

| Password Opening Door | Emergency State When Leak is Detected | Resetting Emergency State |
| :---: | :---: | :---: |
| ![Password Opening Door](assets/01_PasswordOpeningDoor.gif) | ![Emergency State](assets/02_EmergenceStateWhenLeak.gif) | ![Resetting Emergency State](assets/03_ResettingEmergencyState.gif) |

## Technical Stack & Hardware Architecture
* **Microcontroller:** Arduino Uno
* **Driver ICs:** 74HC595 (8-bit Latched Shift Register), ULN2003 Darlington Transistor Array
* **Actuator:** 28BYJ-48 Unipolar Stepper Motor (Configured for 4-step sequence)
* **Sensors & Inputs:** HW-038 Leak Sensor, 4x4 Matrix Keypad, Manual Push-Button
* **Firmware Language:** C++ / Arduino Wire

### Pin Configuration Matrix

| Component | Arduino Pin | Control Logic / Mode | Description |
| :--- | :--- | :--- | :--- |
| **Reset Button** | `A2` | `INPUT_PULLUP` (Active-Low) | Hardware emergency reset button |
| **Leak Sensor VCC** | `A0` | `OUTPUT` | Gated Power to minimize sensor oxidation |
| **Leak Sensor Signal** | `A1` | `INPUT_ANALOG` | Signal from water detection |
| **Danger Status LED** | `10` | `OUTPUT` (PWM Capable) | Red LED to indicate emergency state being on |
| **74HC595 Latch (ST_CP)** | `13` | `OUTPUT` | Latches data to storage register |
| **74HC595 Clock (SH_CP)** | `12` | `OUTPUT` | Shifts serial data into shift register |
| **74HC595 Data (DS)** | `11` | `OUTPUT` | Serial data input line |
| **Matrix Keypad Rows** | `5, 4, 3, 2` | `INPUT` / Scanning | Matrix scanning row pins |
| **Matrix Keypad Columns**| `6, 7, 8, 9` | `OUTPUT` / Scanning | Matrix scanning column pins |

---

## Core Engineering Principles Implemented

### 1. Life Safety Fail-Safe (Software Interlock)
The system shifts into a **Fail-Safe** operational profile upon hazard detection. The moment liquid is detected, the standard loop breaks, the entry keypad is physically locked out, and the system autonomously triggers an full opening loop to prevent occupants from being trapped in a flooded zone.

### 2. Shift Register & Stepper Control
Since there are not enough I/O pins on the microcontroller, the unipolar stepper motor coils along with the green LED are equenced using a 74HC595 8-bit Serial-In-Parallel-Out (SIPO) shift register, with the following sequence
```text
  Bit:   Q7   Q6   Q5   Q4   Q3   Q2   Q1   Q0
       [ X ][ X ][ X ][ L ][ M ][ M ][ M ][ M ]
                        |    |    |    |    |
                        |    +----+----+----+---> [Bits 0-3] Stepper Motor Coils (Pins 15, 1, 2, 3)
                        +-----------------------> [Bit 4]  Status Green LED (Pin 4)
```
* **Rotation of 90°:** The code executes a fixed 128-cycle iteration loop using a 4-step sequence array (`B00010001` to `B00011000`). This matches the step count required to rotate the motor exactly 90-degrees to open the door.
* **Default State after Emergency is Over:** To close the door again after emergency clearance, the phase commutation steps are shown in reverse order, which drives the motor back to its original position
* **Holding the Green LED ON** During the stationary open interval, we use B00010000 to completely de-energize the  motor coils while keeping the green LED latched high. This prevents overheating from the constant draw of current.
  
### 3. Active-Low Pull-Up Implementation
The physical manual reset button utilizes internal Arduino UNO pull-up structures (`INPUT_PULLUP`). Pulling the pin high internally avoids floating-state interference. This ensures the button press of a logic `LOW` indicates explicit human interaction.

### Schematics
The complete electrical logic design were mapped out using **AutoCAD Electrical**. 

* [View Schematic PDF](hardware/AutoCAD_Electrical_Schematic.pdf)

##  Project Directory Structure
```text
├── src/
│   └── Password_Protected_Door.ino                          # Main embedded C++ loop logic
├── hardware/
│   └── Hardware_Topview_74HC595 connections.pdf             # Photos of Hardware and 74HC595 Shift Register connections
│   └── AutoCAD_Electrical_Schematic.pdf                     # Wiring diagrams and I/O connections
├── README.md                                                # Project documentation and engineering report
