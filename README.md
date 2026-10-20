# Arduino Drill Controller

An Arduino Mega/Wokwi project exploring joystick control of a simulated drilling arm, vibration-triggered interlocks, and an LED-matrix hazard display. Originally developed as the **Thalassic Excavator** embedded-systems exercise.

**Repository status: documentation and submitted evidence.** The available material contains three reports, circuit images and links to recorded simulations. The original Arduino sketch and editable Wokwi project have not yet been located. This repository is not currently a self-contained buildable or runnable project, and no firmware has been recreated from screenshots.

## Documented development stages

| Stage | Features described in the report | Report |
| --- | --- | --- |
| 0: interface and motion | OLED menu, joystick input, select/back buttons, stepper depth control and servo tilt | [Level 0](docs/submission/level-0-report.docx) |
| 1: vibration feedback | MPU6050-based vibration metric, RGB feedback, buzzer and a retract-only response after a hard-rock event | [Level 1](docs/submission/level-1-report.docx) |
| 2: mapping and readings | Drill animation, hazard-depth logging, a readings menu and LED-matrix graph display | [Level 2](docs/submission/level-2-report.docx) |

The reports describe the implementation approach and debugging history. Their requirements include responsive scheduling without `delay()`, interrupt-driven select/back inputs and bounded hazard storage. Compliance and complete runtime behavior have not been independently tested here.

## Hardware shown in the submitted diagrams

- Arduino Mega.
- Analog joystick and an additional push button.
- I2C OLED display.
- Stepper motor with an A4988 driver and a servo motor.
- MPU6050 motion sensor in the feedback stages.
- RGB LED and buzzer for status/interlock feedback.
- MAX7219-style 8×8 LED-matrix module in the mapping stage.

The stage diagrams are historical Wokwi snapshots. Firmware is needed to establish the exact pin assignments, timing assumptions and active component configuration.

## Circuit diagrams

### Interface and motion

![Stage 0 circuit diagram](docs/circuits/level-0.png)

### Vibration feedback and interlock

![Stage 1 circuit diagram](docs/circuits/level-1.png)

### Hazard mapping and display

![Stage 2 circuit diagram](docs/circuits/level-2.png)

## Submitted recordings

- [Stage 0 recording](https://drive.google.com/file/d/1DPsZVRSRvrXB0LwuJ75AtvsVDSbADhUV/view) — `Level_0.mkv`, approximately 68.1 MB.
- [Stage 1 recording](https://drive.google.com/file/d/1VMMDrOzgt0JvMdCT57NRFqtiZ7Od7ujr/view) — `Level_1.mkv`, approximately 29.8 MB.
- [Stage 2 recording](https://drive.google.com/file/d/1LcFLCSrh0hfGbNf_W9HodWCG2g3zlqwF/view) — `Level_2.mp4`, approximately 319.9 MB.

These are existing submission recordings, retained on Drive rather than copied into Git history. Drive's existing access settings apply. They have not been independently reviewed frame by frame, and no new simulation run is claimed.

## Development notes

The mapping-stage report records several iterations of drill animation: removing trailing pixels, avoiding blocking delays, constraining matrix addressing and simplifying the animation logic. It also describes a bounded hazard graph with up to eight display columns.

The vibration calculation is treated here as the exercise's feedback metric. The reports and diagrams alone do not establish calibrated material-hardness measurement or persistent storage across a power cycle.

## Files and provenance

| Folder | Contents |
| --- | --- |
| `docs/submission/` | Reports retrieved from the supplied Drive submission |
| `docs/circuits/` | The three submitted PNG circuit diagrams |
| `archive/local/` | Earlier local report copies, preserved separately |

[Source Drive folder](https://drive.google.com/drive/folders/1tcv4Tossfqg1NjhqLnVZBN4WMbcAC4-o). The available files were collected on 20 October 2026. Original reports and images are unchanged; their repository copies were checked with SHA-256 hashes, and the Word document archives passed integrity checks.

## Restoring a runnable project

The next required source material is the original Wokwi project URL/export or Arduino sketch, together with its circuit definition and library dependencies. Once located, it can be added alongside these reports, with each firmware version matched to its stage. Build and simulation checks should follow that recovery rather than reconstructing an unverified implementation from screenshots.
