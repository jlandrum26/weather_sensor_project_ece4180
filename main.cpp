
#include "mbed.h"
#include "MPL115A2.h"
#include "SI7021.h"
#include "uLCD_4DGL.h"

Serial pc(USBTX, USBRX);
uLCD_4DGL uLCD(p13,p14,p11); // serial tx, serial rx, reset pin;
//DHT tempsensor(p23,SEN11301P); // Use the SEN11301P sensor
I2C i2c(p28, p27);
AnalogIn gassensor(p15);
AnalogIn noisesensor(p16);

MPL115A2 p_sensor(&i2c);
SI7021 myTempRHsensor(p9, p10, SI7021::SI7021_ADDRESS, 400000);
//Mutex lcd_mutex;

float tempval;
float humidval;
float dewpoint;
float pressureval;
float noiseval;
float airqval;

// Thread 1
// temperature/humidity/dewpoint reading and displaying

SI7021::SI7021_vector_data_t myData;
SI7021::SI7021_status_t mySI7021status;
uint32_t myState;

// Thread 2
// pressure reading and displaying
const int COMFORT_COLOR = 0xADD8E6; // Light Blue for a comfortable, high-tech look
const int STATUS_BAR_COLOR = 0x008000; // Green for status bars
const int TEXT_COLOR = 0x000000; // Black for text
const int SEPARATOR_COLOR = 0xFFFFFF; // White for separating lines
const int HAZARD = 0x800000; // Maroon for hazardous air quality and painful noise
const int MODE = 0xFFFF00; // Yellow for moderate air quality
const int SENS = 0xFF8200; // Orange for unhealthy air quality for sensitive people
const int BAD = 0xBB42BB; // Purple for very unhealthy air quality 

// Sample values (replace with actual sensor readings)
float temp; // in Celsius (10-=cold, 15-19.9=cool, 20-25.9=warm, 26-37.9=hot, 38+=really hot)
float humidity = 45.0; // in percentage
float noiseLevel = 60; // in decibels (damage possible from 80+ dB, 120+ dB = pain & injury)
float pressure; // in hPa
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


int main()
{
    //LCD.cls();
    initDisplay();

    // Draw separators first to avoid overwriting
    uLCD.line(0, 42, 127, 42, SEPARATOR_COLOR); // Horizontal separators
    uLCD.line(0, 85, 127, 85, SEPARATOR_COLOR);
    uLCD.line(64, 0, 64, 127, SEPARATOR_COLOR); // Vertical separator
    pc.printf("Start \r\n");
    //Thread t1(thread1); //start thread1
    //Thread t2(thread2);
    //Thread t3(thread3);
    pc.baud(115200);
    mySI7021status = myTempRHsensor.SI7021_SoftReset();
    wait_ms(15);
    mySI7021status = myTempRHsensor.SI7021_Conf(SI7021::SI7021_RESOLUTION_RH_12_TEMP_14, SI7021::SI7021_HTRE_DISABLED);

    
    p_sensor.begin();  
    while(1){
        noiseval = noisesensor;
        pc.printf("Noise Level is %f \r\n", noiseval);
        p_sensor.ReadSensor();
        pressure = p_sensor.GetPressure();
        pc.printf("Pressure    = %f\n", pressure);
        wait(1);
        temp = p_sensor.GetTemperature();
        pc.printf("Temperature = %f\n", temp);
        wait(1);
        airqval = gassensor;
        pc.printf("AirQ is %f \r\n", airqval);
        mySI7021status = myTempRHsensor.SI7021_TriggerHumidity(SI7021::SI7021_NO_HOLD_MASTER_MODE);

        // Short delay to allow the sensor to complete the measurement
        wait_ms(500); 

        // Read Humidity result
        mySI7021status = myTempRHsensor.SI7021_ReadHumidity(&myData);

        // Send it through the UART
        pc.printf("RH: %0.5f%%\r\n", myData.RelativeHumidity);

        // Short delay before the next reading
        

        displayTileWithNumericValue(temp, 1, 1, "Temp");//Celsius
        displayTileWithNumericValue(myData.RelativeHumidity, 10, 1, "Humidty");//%
        displayTileWithStatusBar(noiseval, 1, 6, "Noise", 100.0);//dB
        displayTileWithNumericValue(pressure, 10, 6, "Pressure");//hPa
        displayAirQ(airqval, 1, 11, "Air Qual", airqval);//AQI
        displayTileWithStatusBar(overallCondition, 10, 11, "Cond", 100.0); //Why do we have this?

        wait_ms(5000);
    }
}
