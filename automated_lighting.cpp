#include <iostream>
#include <wiringPi.h>
#include <signal.h>
#include <unistd.h>

// GPIO Pin Definitions
#define PIR_ROOM 17
#define PIR_BATHROOM 27
#define RELAY_LIGHT1 22
#define RELAY_LIGHT2 23
#define RELAY_LIGHT3 24

// Light control variables
volatile bool roomOccupied = false;
volatile bool bathroomOccupied = false;
unsigned long lastMotionTimeRoom = 0;
unsigned long lastMotionTimeBathroom = 0;
const unsigned long LIGHT_TIMEOUT = 300000; // 5 minutes in milliseconds

// Function prototypes
void roomMotionDetected();
void bathroomMotionDetected();
void controlLights();
void cleanup(int signal);

int main() {
    std::cout << "Initializing Automated Lighting System..." << std::endl;
    
    // Initialize WiringPi
    if (wiringPiSetupGpio() == -1) {
        std::cerr << "Failed to initialize WiringPi" << std::endl;
        return 1;
    }
    
    // Set up PIR sensors as inputs
    pinMode(PIR_ROOM, INPUT);
    pinMode(PIR_BATHROOM, INPUT);
    
    // Set up relays as outputs
    pinMode(RELAY_LIGHT1, OUTPUT);
    pinMode(RELAY_LIGHT2, OUTPUT);
    pinMode(RELAY_LIGHT3, OUTPUT);
    
    // Initialize relays to off state
    digitalWrite(RELAY_LIGHT1, HIGH); // Relay modules are often active LOW
    digitalWrite(RELAY_LIGHT2, HIGH);
    digitalWrite(RELAY_LIGHT3, HIGH);
    
    // Set up interrupt handlers for PIR sensors
    if (wiringPiISR(PIR_ROOM, INT_EDGE_RISING, &roomMotionDetected) < 0) {
        std::cerr << "Unable to setup ISR for room PIR" << std::endl;
        return 1;
    }
    
    if (wiringPiISR(PIR_BATHROOM, INT_EDGE_RISING, &bathroomMotionDetected) < 0) {
        std::cerr << "Unable to setup ISR for bathroom PIR" << std::endl;
        return 1;
    }
    
    // Set up signal handler for clean exit
    signal(SIGINT, cleanup);
    
    std::cout << "System ready. Monitoring for motion..." << std::endl;
    
    // Main control loop
    while (true) {
        controlLights();
        delay(1000); // Check every second
    }
    
    return 0;
}

// Interrupt Service Routine for Room PIR
void roomMotionDetected() {
    roomOccupied = true;
    lastMotionTimeRoom = millis();
    std::cout << "Motion detected in room" << std::endl;
}

// Interrupt Service Routine for Bathroom PIR
void bathroomMotionDetected() {
    bathroomOccupied = true;
    lastMotionTimeBathroom = millis();
    std::cout << "Motion detected in bathroom" << std::endl;
}

// Control lights based on occupancy and timeout
void controlLights() {
    unsigned long currentTime = millis();
    
    // Room lights control
    if (roomOccupied) {
        // Turn on both room lights when motion is detected
        digitalWrite(RELAY_LIGHT1, LOW);
        digitalWrite(RELAY_LIGHT2, LOW);
        
        // Check if timeout has elapsed since last motion
        if (currentTime - lastMotionTimeRoom > LIGHT_TIMEOUT) {
            roomOccupied = false;
            std::cout << "Room lights timeout - turning off" << std::endl;
        }
    } else {
        // Turn off room lights when no motion
        digitalWrite(RELAY_LIGHT1, HIGH);
        digitalWrite(RELAY_LIGHT2, HIGH);
    }
    
    // Bathroom light control
    if (bathroomOccupied) {
        // Turn on bathroom light when motion is detected
        digitalWrite(RELAY_LIGHT3, LOW);
        
        // Check if timeout has elapsed since last motion
        if (currentTime - lastMotionTimeBathroom > LIGHT_TIMEOUT) {
            bathroomOccupied = false;
            std::cout << "Bathroom light timeout - turning off" << std::endl;
        }
    } else {
        // Turn off bathroom light when no motion
        digitalWrite(RELAY_LIGHT3, HIGH);
    }
}

// Cleanup function for graceful exit
void cleanup(int signal) {
    std::cout << "\nShutting down lighting system..." << std::endl;
    
    // Turn off all lights
    digitalWrite(RELAY_LIGHT1, HIGH);
    digitalWrite(RELAY_LIGHT2, HIGH);
    digitalWrite(RELAY_LIGHT3, HIGH);
    
    exit(0);
}