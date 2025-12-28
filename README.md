# OlyObd

OBD-II CAN-BUS reader for Arduino UNO using Seeedstudio CAN-BUS Shield V2. This tool reads real-time diagnostic data from your car's ECU (Engine Control Unit) via the OBD-II port.

## Features

- ✅ Engine RPM monitoring
- ✅ Vehicle speed tracking
- ✅ Engine coolant temperature
- ✅ Throttle position
- ✅ Engine load percentage
- ✅ Serial output for real-time monitoring

## Hardware Requirements

- **Arduino UNO** (or compatible board)
- **Seeedstudio CAN-BUS Shield V2** (based on MCP2515 CAN controller)
- **OBD-II to DB9 cable** (to connect shield to car)
- **USB cable** (for Arduino programming and serial monitoring)

## Hardware Setup

1. **Mount the CAN-BUS Shield on Arduino UNO**
   - Stack the Seeedstudio CAN-BUS Shield V2 on top of Arduino UNO
   - Ensure all pins are properly aligned

2. **Connect to Car's OBD-II Port**
   - Locate your car's OBD-II port (usually under the dashboard, driver's side)
   - Connect the DB9 cable from the CAN-BUS Shield to the OBD-II port
   - Turn on your car's ignition (engine can be off for initial testing)

3. **CAN-BUS Shield V2 Pin Configuration**
   - CS (Chip Select): Digital Pin 9
   - INT (Interrupt): Digital Pin 2
   - SPI: MISO (Pin 12), MOSI (Pin 11), SCK (Pin 13)

## Software Setup

### Prerequisites

- [PlatformIO](https://platformio.org/) installed (via VSCode extension or CLI)
- Or [Arduino IDE](https://www.arduino.cc/en/software) with required libraries

### Installation with PlatformIO

1. Clone this repository:
   ```bash
   git clone https://github.com/7kasper/OlyObd.git
   cd OlyObd
   ```

2. Open the project in PlatformIO (VSCode):
   ```bash
   code .
   ```

3. Build the project:
   ```bash
   pio run
   ```

4. Upload to Arduino:
   ```bash
   pio run --target upload
   ```

5. Open serial monitor to view data:
   ```bash
   pio device monitor
   ```

### Installation with Arduino IDE

1. Install required libraries:
   - Open Arduino IDE
   - Go to **Sketch → Include Library → Manage Libraries**
   - Search and install:
     - "MCP_CAN_lib" by coryjfowler
     - "Seeed Arduino CAN" by Seeed Studio

2. Open `src/main.cpp` in Arduino IDE (rename to `.ino` if needed)

3. Select **Tools → Board → Arduino UNO**

4. Select your COM port under **Tools → Port**

5. Click **Upload**

6. Open **Tools → Serial Monitor** (set baud rate to 115200)

## Usage

Once uploaded and connected to your car:

1. Turn on your car's ignition
2. Open the serial monitor at 115200 baud
3. The device will automatically start reading OBD-II data every second
4. View real-time data in the serial monitor

### Example Output

```
OlyObd - OBD-II CAN-BUS Reader
===============================

Initializing CAN-BUS Shield... OK
CAN-BUS Shield initialized successfully
Waiting for OBD-II data...

--- Reading OBD-II Data ---
Engine RPM: 1850 RPM
Vehicle Speed: 45 km/h
Coolant Temp: 85 °C
Throttle Position: 23 %
Engine Load: 45 %

--- Reading OBD-II Data ---
Engine RPM: 2100 RPM
Vehicle Speed: 52 km/h
Coolant Temp: 86 °C
Throttle Position: 31 %
Engine Load: 52 %
```

## Troubleshooting

### CAN-BUS Shield initialization fails
- Check that the shield is properly seated on the Arduino
- Verify the CS pin (default: Pin 9)
- Try resetting the Arduino

### No data received from car
- Ensure car ignition is ON
- Check OBD-II cable connections
- Verify your car supports OBD-II (most cars from 1996+ in US, 2001+ in EU)
- Some cars may use different CAN speeds (try 250kbps if 500kbps doesn't work)

### "READ FAILED" messages
- Car may not support specific PIDs
- ECU might be busy or not responding
- Check cable connections
- Some parameters only work when engine is running

## Technical Details

### OBD-II Protocol
- **Standard**: ISO 15765-4 (CAN)
- **CAN Speed**: 500 kbps (standard for most cars)
- **Request ID**: 0x7DF (functional broadcast)
- **Response IDs**: 0x7E8 - 0x7EF

### Supported PIDs (Mode 01)
- `0x0C`: Engine RPM
- `0x0D`: Vehicle Speed
- `0x05`: Engine Coolant Temperature
- `0x11`: Throttle Position
- `0x04`: Engine Load

## Extending Functionality

To add more OBD-II parameters, define additional PIDs and create getter functions following the existing pattern:

```cpp
#define PID_FUEL_LEVEL 0x2F

int getFuelLevel() {
    sendOBDRequest(PID_FUEL_LEVEL);
    byte response[5];
    if (readOBDResponse(PID_FUEL_LEVEL, response)) {
        return (response[0] * 100) / 255;
    }
    return -1;
}
```

## License

This project is open source. Feel free to modify and distribute.

## References

- [OBD-II PIDs - Wikipedia](https://en.wikipedia.org/wiki/OBD-II_PIDs)
- [Seeedstudio CAN-BUS Shield V2](https://wiki.seeedstudio.com/CAN-BUS_Shield_V2.0/)
- [MCP2515 CAN Controller](https://www.microchip.com/wwwproducts/en/MCP2515)
