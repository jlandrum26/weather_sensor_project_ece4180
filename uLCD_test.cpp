#include "mbed.h"
#include "uLCD_4DGL.h"

uLCD_4DGL uLCD(p9, p10, p11); // Serial tx, Serial rx, Reset pin

// Define colors
const int COMFORT_COLOR = 0xADD8E6; // Light Blue for a comfortable, high-tech look
const int STATUS_BAR_COLOR = 0x008000; // Green for status bars
const int TEXT_COLOR = 0x000000; // Black for text
const int SEPARATOR_COLOR = 0xFFFFFF; // White for separating lines
const int HAZARD = 0x800000; // Maroon for hazardous air quality and painful noise
const int MODE = 0xFFFF00; // Yellow for moderate air quality
const int SENS = 0xFF8200; // Orange for unhealthy air quality for sensitive people
const int BAD = 0xBB42BB; // Purple for very unhealthy air quality 

// Sample values (replace with actual sensor readings)
float temperature = 23.5; // in Celsius (10-=cold, 15-19.9=cool, 20-25.9=warm, 26-37.9=hot, 38+=really hot)
float humidity = 45.0; // in percentage
float noiseLevel = 60; // in decibels (damage possible from 80+ dB, 120+ dB = pain & injury)
float airPressure = 1012; // in hPa
float airQuality = 80; // Air quality index
float overallCondition = 75; // Arbitrary value for overall condition

// Initialize uLCD display settings
void initDisplay() {
    uLCD.background_color(COMFORT_COLOR);
    uLCD.cls();
    uLCD.text_width(1); // Smaller font
    uLCD.text_height(1);
    uLCD.color(TEXT_COLOR);
    uLCD.textbackground_color(COMFORT_COLOR);
}

// Function to display a tile with numerical value
void displayTileWithNumericValue(float value, int x, int y, const char* label) {
    // Set text properties
    uLCD.text_width(1); // Smaller font
    uLCD.text_height(1);
    uLCD.color(TEXT_COLOR);

    // Calculate text positions (assuming 8x8 pixels per character)
    int label_pos_y = y; // Top of the tile
    int value_pos_y = y + 1; // Just below the label

    // Display the label at the top of the tile
    uLCD.locate(x, label_pos_y);
    uLCD.printf("%s", label);

    // Display the value in the middle of the tile
    uLCD.locate(x, value_pos_y);
    uLCD.printf("%.1f", value);
}

// Function to display a tile with a status bar
void displayTileWithStatusBar(float value, int x, int y, const char* label, float maxValue) {
    // Set text properties
    uLCD.text_width(1); // Smaller font
    uLCD.text_height(1);
    uLCD.color(TEXT_COLOR);

    // Calculate text positions (assuming 8x8 pixels per character)
    int label_pos_y = y; // Top of the tile
    int value_pos_y = y + 1; // Just below the label

    // Display the label at the top of the tile
    uLCD.locate(x, label_pos_y);
    uLCD.printf("%s", label);

    // Display the value above the status bar
    uLCD.locate(x, value_pos_y);
    uLCD.printf("%.0f", value);

    // Draw status bar
    int barLength = 50; // Length of the status bar
    int filledLength = (int)(barLength * (value / maxValue)); // Filled length of the bar
    uLCD.filled_rectangle(x*8+2, (y+3)*8, x*8+2+filledLength, (y+4)*8, STATUS_BAR_COLOR); // Adjusted for pixel position
    uLCD.rectangle(x*8+2, (y+3)*8, x*8+52, (y+4)*8, TEXT_COLOR); // Bar border
}

// Function to display a tile with a status bar
void displayAirQ(float value, int x, int y, const char* label, float maxValue) {
    // Set text properties
    uLCD.text_width(1); // Smaller font
    uLCD.text_height(1);
    uLCD.color(TEXT_COLOR);

    // Calculate text positions (assuming 8x8 pixels per character)
    int label_pos_y = y; // Top of the tile
    int value_pos_y = y + 1; // Just below the label

    // Display the label at the top of the tile
    uLCD.locate(x, label_pos_y);
    uLCD.printf("%s", label);

    // Display the value above the status bar
    uLCD.locate(x, value_pos_y);
    uLCD.printf("%.0f", value);

    int STATUS = 0x000000; // Default value

    if (value < 51) {
        STATUS = STATUS_BAR_COLOR;
    }
    if (value >= 51 && value < 101) {
        STATUS = MODE;
    }
    if (value >= 101 && value < 151) {
        STATUS = SENS;
    }
    if (value >= 151 && value < 201) {
        STATUS = RED;
    }
    if (value >= 201 && value < maxValue) {
        STATUS = BAD;
    }
    if (value >= maxValue) {
        STATUS = HAZARD;
    }

    // Draw colored square/rectangle
    uLCD.filled_rectangle(x*8+2, (y+3)*8, x*8+12, (y+4)*8, STATUS); // Adjusted for pixel position
}

int main() {
    // Initialize display
    initDisplay();

    // Draw separators first to avoid overwriting
    uLCD.line(0, 42, 127, 42, SEPARATOR_COLOR); // Horizontal separators
    uLCD.line(0, 85, 127, 85, SEPARATOR_COLOR);
    uLCD.line(64, 0, 64, 127, SEPARATOR_COLOR); // Vertical separator

    // Display tiles
    displayTileWithNumericValue(temperature, 1, 1, "Temp");//Celsius
    displayTileWithNumericValue(humidity, 10, 1, "Hum");//%
    displayTileWithStatusBar(noiseLevel, 1, 6, "Noise", 100.0);//dB
    displayTileWithNumericValue(airPressure, 10, 6, "Press");//hPa
    displayAirQ(airQuality, 1, 11, "Air Q", 301.0);//AQI
    displayTileWithStatusBar(overallCondition, 10, 11, "Cond", 100.0); //Why do we have this?

    while (1) {
        // Update display with sensor data in a loop or based on an event
        // Placeholder for sensor update code
    }
}
