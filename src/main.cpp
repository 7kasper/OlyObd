/*
 * OlyObd - OBD-II CAN-BUS Reader for Arduino UNO
 * 
 * Reads car diagnostic information using Seeedstudio CAN-BUS Shield V2
 * Based on MCP2515 CAN controller
 * 
 * Hardware Setup:
 * - Arduino UNO
 * - Seeedstudio CAN-BUS Shield V2
 * - OBD-II cable connected to car's OBD port
 * 
 * CAN-BUS Shield V2 uses:
 * - CS   -> Digital Pin 9 (default for Seeed V2)
 * - INT  -> Digital Pin 2
 */

#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>

// CAN-BUS Shield V2 CS pin
const int SPI_CS_PIN = 9;

// CAN controller object
MCP_CAN CAN(SPI_CS_PIN);

// OBD-II Standard PIDs (Mode 01 - Show current data)
#define PID_ENGINE_RPM              0x0C
#define PID_VEHICLE_SPEED           0x0D
#define PID_COOLANT_TEMP            0x05
#define PID_THROTTLE_POSITION       0x11
#define PID_ENGINE_LOAD             0x04
#define PID_INTAKE_TEMP             0x0F
#define PID_MAF_FLOW                0x10
#define PID_FUEL_PRESSURE           0x0A

// OBD-II Request structure
#define OBD_REQUEST_ID              0x7DF  // Functional broadcast address
#define OBD_RESPONSE_ID             0x7E8  // ECU response address (can be 7E8-7EF)

// Timing
#define OBD_REQUEST_INTERVAL        1000   // Request data every 1 second
#define CAN_TIMEOUT                 100    // Timeout for CAN response in ms

unsigned long lastRequestTime = 0;

/**
 * Send OBD-II request via CAN
 * @param pid Parameter ID to request
 */
void sendOBDRequest(byte pid) {
    byte request[8] = {0x02, 0x01, pid, 0x00, 0x00, 0x00, 0x00, 0x00};
    // request[0] = 0x02: Number of additional bytes
    // request[1] = 0x01: Mode 01 (Show current data)
    // request[2] = pid:  Parameter ID
    
    CAN.sendMsgBuf(OBD_REQUEST_ID, 0, 8, request);
}

/**
 * Read OBD-II response from CAN
 * @param pid Expected PID
 * @param response Buffer to store response data
 * @return true if response received and valid
 */
bool readOBDResponse(byte pid, byte* response) {
    unsigned long startTime = millis();
    
    while (millis() - startTime < CAN_TIMEOUT) {
        if (CAN_MSGAVAIL == CAN.checkReceive()) {
            byte len = 0;
            byte buf[8];
            unsigned long canId;
            
            CAN.readMsgBuf(&len, buf);
            canId = CAN.getCanId();
            
            // Check if it's an OBD response (0x7E8-0x7EF)
            if (canId >= 0x7E8 && canId <= 0x7EF) {
                // Check if it's a response to our PID
                // buf[0] = number of additional bytes
                // buf[1] = mode + 0x40 (0x41 for mode 01 response)
                // buf[2] = PID
                if (buf[1] == 0x41 && buf[2] == pid) {
                    // Copy response data (starts at buf[3])
                    for (int i = 0; i < 5; i++) {
                        response[i] = buf[3 + i];
                    }
                    return true;
                }
            }
        }
        delay(5);
    }
    return false;
}

/**
 * Get engine RPM
 * @return RPM value, or -1 if read failed
 */
int getEngineRPM() {
    sendOBDRequest(PID_ENGINE_RPM);
    byte response[5];
    
    if (readOBDResponse(PID_ENGINE_RPM, response)) {
        // RPM = ((A * 256) + B) / 4
        int rpm = ((response[0] * 256) + response[1]) / 4;
        return rpm;
    }
    return -1;
}

/**
 * Get vehicle speed
 * @return Speed in km/h, or -1 if read failed
 */
int getVehicleSpeed() {
    sendOBDRequest(PID_VEHICLE_SPEED);
    byte response[5];
    
    if (readOBDResponse(PID_VEHICLE_SPEED, response)) {
        // Speed = A (in km/h)
        return response[0];
    }
    return -1;
}

/**
 * Get coolant temperature
 * @return Temperature in Celsius, or -999 if read failed
 */
int getCoolantTemp() {
    sendOBDRequest(PID_COOLANT_TEMP);
    byte response[5];
    
    if (readOBDResponse(PID_COOLANT_TEMP, response)) {
        // Temperature = A - 40 (in Celsius)
        return response[0] - 40;
    }
    return -999;
}

/**
 * Get throttle position
 * @return Throttle position in percentage (0-100), or -1 if read failed
 */
int getThrottlePosition() {
    sendOBDRequest(PID_THROTTLE_POSITION);
    byte response[5];
    
    if (readOBDResponse(PID_THROTTLE_POSITION, response)) {
        // Throttle = (A * 100) / 255
        return (response[0] * 100) / 255;
    }
    return -1;
}

/**
 * Get engine load
 * @return Engine load in percentage (0-100), or -1 if read failed
 */
int getEngineLoad() {
    sendOBDRequest(PID_ENGINE_LOAD);
    byte response[5];
    
    if (readOBDResponse(PID_ENGINE_LOAD, response)) {
        // Engine Load = (A * 100) / 255
        return (response[0] * 100) / 255;
    }
    return -1;
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("OlyObd - OBD-II CAN-BUS Reader"));
    Serial.println(F("==============================="));
    Serial.println();
    
    // Initialize CAN-BUS at 500kbps (OBD-II standard)
    Serial.print(F("Initializing CAN-BUS Shield..."));
    
    // Try to initialize CAN at 500kbps (standard OBD-II speed)
    if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
        Serial.println(F(" OK"));
    } else {
        Serial.println(F(" FAILED"));
        Serial.println(F("Check connections and reset Arduino"));
        while (1) {
            delay(1000);
        }
    }
    
    // Set CAN to normal mode
    CAN.setMode(MCP_NORMAL);
    
    Serial.println(F("CAN-BUS Shield initialized successfully"));
    Serial.println(F("Waiting for OBD-II data..."));
    Serial.println();
    
    delay(1000);
}

void loop() {
    unsigned long currentTime = millis();
    
    // Request data at regular intervals
    if (currentTime - lastRequestTime >= OBD_REQUEST_INTERVAL) {
        lastRequestTime = currentTime;
        
        Serial.println(F("--- Reading OBD-II Data ---"));
        
        // Read Engine RPM
        int rpm = getEngineRPM();
        if (rpm >= 0) {
            Serial.print(F("Engine RPM: "));
            Serial.print(rpm);
            Serial.println(F(" RPM"));
        } else {
            Serial.println(F("Engine RPM: READ FAILED"));
        }
        
        // Read Vehicle Speed
        int speed = getVehicleSpeed();
        if (speed >= 0) {
            Serial.print(F("Vehicle Speed: "));
            Serial.print(speed);
            Serial.println(F(" km/h"));
        } else {
            Serial.println(F("Vehicle Speed: READ FAILED"));
        }
        
        // Read Coolant Temperature
        int coolant = getCoolantTemp();
        if (coolant != -999) {
            Serial.print(F("Coolant Temp: "));
            Serial.print(coolant);
            Serial.println(F(" °C"));
        } else {
            Serial.println(F("Coolant Temp: READ FAILED"));
        }
        
        // Read Throttle Position
        int throttle = getThrottlePosition();
        if (throttle >= 0) {
            Serial.print(F("Throttle Position: "));
            Serial.print(throttle);
            Serial.println(F(" %"));
        } else {
            Serial.println(F("Throttle Position: READ FAILED"));
        }
        
        // Read Engine Load
        int load = getEngineLoad();
        if (load >= 0) {
            Serial.print(F("Engine Load: "));
            Serial.print(load);
            Serial.println(F(" %"));
        } else {
            Serial.println(F("Engine Load: READ FAILED"));
        }
        
        Serial.println();
    }
    
    // Small delay to prevent overwhelming the CAN bus
    delay(10);
}
