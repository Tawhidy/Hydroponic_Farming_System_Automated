
# Project Manual: Automated Hydroponic Farming System

# 1. Introduction

This manual provides comprehensive documentation for the Automated Hydroponic Farming System hardware project. It includes step-by-step assembly instructions, a complete bill of materials (BOM), circuit diagrams, and contributor information. This document serves as both a build guide and reference for future modifications.

## 1.1 Project Overview

The Automated Hydroponic Farming System is a self-contained solution that monitors and controls water, nutrients, and lighting for soil-free plant cultivation. This project was designed to allow anyone to grow plants indoors efficiently without requiring extensive agricultural knowledge.

## 1.2 Repository Structure

The GitHub repository is organized as follows:

- /docs - Documentation files including this manual
- /hardware - Circuit diagrams, PCB designs, and component specifications
- /firmware - Software required for operation
- /images - Photos and diagrams used throughout documentation

# 2. Bill of Materials (BOM)

Below is the complete list of components required for this project:

| **Component**           | **Quantity** | **Reference** | **Description**                                       | **Product Link**                               |
| ----------------------- | ------------ | ------------- | ----------------------------------------------------- | ---------------------------------------------- |
| Arduino uno/nano        | 2            | U1            | Microcontroller board based on ATmega328P             | https://store.arduino.cc/products/arduino-nano |
| pH Sensor               | 1            | S1            | Analog pH sensor with probe, 0-14 pH range            | https://www.dfrobot.com/product-1025.html      |
| 12V Water Pump          | 4            | P1            | 12V DC submersible pump, 240L/H flow rate             | https://www.adafruit.com/product/1150          |
| LED Grow Lights         | 2            | L1, L2        | Full spectrum grow lights, 15W, 12V DC                | https://www.sparkfun.com/products/14590        |
| mist maker              | 1            | Various       | Power supply, resistors, capacitors, connectors, etc. | See detailed BOM document                      |
| relay                   | 3            |               |                                                       |                                                |
| 220V - 12V power supply | 1            |               |                                                       |                                                |
| 12V - 5V buck converter | 1            |               |                                                       |                                                |
| 120mm PC fan            | 2            |               |                                                       |                                                |
| glue gun                | 1            |               |                                                       |                                                |
| glue stick              | 8-10         |               |                                                       |                                                |

## 2.1 Tools Required

- Soldering iron, solder & solder wick
- Wire strippers
- Multimeter
- Screwdriver set
- glue gun

# 3. Circuit Diagram

![alt text](https://github.com/Tawhidy/Hydroponic_Farming_System_Automated/blob/main/Electrical/Hydroponic_schematics.png)

## 3.1 Schematic Explanation

The circuit consists of the following main sections:

- **Power Supply**: 220V-12V power supply to feed into the motors using relays & 12V is then converted into 5V using the buck converter and feed power to the microcontroller and mist maker.
- **Microcontroller**: we used an Arduino UNO and one Nano. The Uno handles the display and Ph control. and the rest of the control is done by the nano ie. temperature and humidity control.
- **Sensor Circuit**: pH, temperature and humidity sensors for controlling the necessary environment for the system.
- **Output Circuit**: Relays controlling nutrient+water pumps, mist maker, LED lighting and the the fans are directly controlled by arduino.

# 4. Assembly Instructions

## 4.1 Electrical Assembly

Follow these steps to assemble the PCB:

1. Begin by soldering the smallest components first (resistors, capacitors)
2. Solder the IC sockets (do not insert the ICs yet)
3. Solder the larger components (connectors, power regulators)
4. Inspect all solder joints for quality and potential shorts
5. Clean the PCB with isopropyl alcohol to remove flux residue
6. Insert ICs into their sockets, ensuring correct orientation

## 4.2 Enclosure Assembly

If your project includes an enclosure, follow these steps:

1. Prepare the enclosure by drilling/cutting necessary openings
2. Mount the PCB using the provided standoffs and screws
3. Install any external components (switches, displays, etc.)
4. Connect any required wiring between the PCB and external components
5. Secure the enclosure with the provided fasteners

# 5. Testing and Verification

After assembly, follow these steps to verify correct operation:

1. Visual inspection of all connections
2. Continuity testing of power rails
3. Initial power-up test (without connected peripherals)
4. Functional testing of each subsystem
5. Integration testing of the complete system

## 5.1 Troubleshooting Guide

If you encounter issues, refer to this troubleshooting guide:

| **Symptom** | **Possible Cause** | **Solution** |
| --- | --- | --- |
| No power | Incorrect power connections | Check power supply connections and polarity |
| Erratic behavior | Cold solder joints | Inspect and reflow suspicious solder joints |
| Sensor readings incorrect | Improper calibration | Recalibrate sensors using known reference values |

# 6. Firmware Installation

To install the firmware:

1. Download the latest firmware from the /firmware directory
2. Connect the Arduino Nano to your computer via USB
3. Use the Arduino IDE to flash the firmware
4. Verify successful firmware installation by checking the serial monitor output

# 7. Usage Instructions

Once assembled and programmed, your hydroponic system will automatically monitor and adjust growing conditions. Use the companion smartphone app to check status and customize settings for your specific plants.

# 8. Contributors

This project was made possible by the contributions of the following individuals:

## 8.1 Core Team

- **Tawhid Alam** - Project Lead
    - GitHub: [https://github.com/tawhidalam](https://github.com/tawhidalam)
    - Email: [email address]
- **[Contributor Name]** - [Role]
    - GitHub: [GitHub profile link]
    - Website: [Personal website if available]
- **[Contributor Name]** - [Role]
    - GitHub: [GitHub profile link]
    - LinkedIn: [LinkedIn profile link]

## 8.2 Contributors

- **[Contributor Name]** - [Contribution]
- **[Contributor Name]** - [Contribution]

# 9. License Information

This project is licensed under the MIT License. See the LICENSE file in the repository for full details.

# 10. Appendix

## 10.1 Additional Resources

- Datasheets for key components
- Reference designs
- Additional documentation

## 10.2 Version History

| **Version** | **Date** | **Changes** |
| --- | --- | --- |
| 1.0 | 2025-08-05 | Initial release |

The appendix also includes detailed visual aids (component photos, assembly steps)

[^1]: 

 
