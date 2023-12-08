
#include "mbed.h"
#include "MPL115A2.h"
#include "SI7021.h"
#include "uLCD_4DGL.h"

Serial pc(USBTX, USBRX);
uLCD_4DGL uLCD(p13,p14,p11); // serial tx, serial rx, reset pin;
//DHT tempsensor(p23,SEN11301P); // Use the SEN11301P sensor
I2C i2c(p28, p27);
AnalogIn gassensor(p15);
AnalogIn noisesensor(p20);

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
float humidity; // in percentage
float noiseLevel = 60; // in decibels (damage possible from 80+ dB, 120+ dB = pain & injury)
float pressure; // in hPa
float overallCondition = 75; // Arbitrary value for overall condition
float tempgrade;
float humiditygrade;
float pressuregrade;
float noisegrade;
float totalgrade;
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
void displayTileWithNumericValuePressure(float value, int x, int y, const char* label) {
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
    uLCD.locate(x, value_pos_y+1);
    uLCD.printf("%.1fhPa", value);
}

void displayTileWithNumericValueHumidity(float value, int x, int y, const char* label) {
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
    uLCD.locate(x+1, value_pos_y+1);
    uLCD.printf("%.1f%%", value);
}

void displayTileWithNumericValueTemp(float value, int x, int y, const char* label) {
    // Set text properties
    uLCD.text_width(1); // Smaller font
    uLCD.text_height(1);
    uLCD.color(TEXT_COLOR);

    // Calculate text positions (assuming 8x8 pixels per character)
    int label_pos_y = y; // Top of the tile
    int value_pos_y = y + 1; // Just below the label

    // Display the label at the top of the tile
    uLCD.locate(x+2, label_pos_y);
    uLCD.printf("%s", label);

    // Display the value in the middle of the tile
    uLCD.locate(x+1, value_pos_y+1);
    uLCD.printf("%.1f F", value);
}

// Function to display a tile with a status bar
void displayTileWithStatusBarNoise(float value, int x, int y, const char* label, float maxValue) {
    // Set text properties
    uLCD.text_width(1); // Smaller font
    uLCD.text_height(1);
    uLCD.color(TEXT_COLOR);

    // Calculate text positions (assuming 8x8 pixels per character)
    int label_pos_y = y; // Top of the tile
    int value_pos_y = y + 1; // Just below the label

    // Display the label at the top of the tile
    uLCD.locate(x+1, label_pos_y);
    uLCD.printf("%s", label);

    // Display the value above the status bar
    uLCD.locate(x+2, value_pos_y+1);
    uLCD.printf("%.0f%%", value);
    int STATUS = 0x000000; // Default value

    if (value < 50) {
        STATUS = STATUS_BAR_COLOR;
    }
    if (value >= 50 && value < 60) {
        STATUS = MODE;
    }
    if (value >= 60 && value < 70) {
        STATUS = SENS;
    }
    if (value >= 70 && value < 80) {
        STATUS = RED;
    }
    if (value >= 80 && value < 90) {
        STATUS = BAD;
    }
    if (value >= 90) {
        STATUS = HAZARD;
    }
    // Draw status bar
    int barLength = 56; // Length of the status bar
    int filledLength = (int)(barLength * (value / maxValue)); // Filled length of the bar
    uLCD.filled_rectangle(x*8-4, (y+3)*8+2, x*8-4+filledLength, (y+4)*8+2, STATUS); // Adjusted for pixel position
    uLCD.rectangle(x*8-4, (y+3)*8+2, x*8+52, (y+4)*8+2, TEXT_COLOR); // Bar border
}

void displayTileWithStatusBarCondition(float value, int x, int y, const char* label, float maxValue) {
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
    uLCD.locate(x+2, value_pos_y+1);
    uLCD.printf("%.0f%%", value);
    int STATUS = 0x000000; // Default value

    if (value > 90) {
        STATUS = STATUS_BAR_COLOR;
    }
    if (value >= 80 && value < 90) {
        STATUS = MODE;
    }
    if (value >= 70 && value < 80) {
        STATUS = SENS;
    }
    if (value >= 60 && value < 70) {
        STATUS = RED;
    }
    if (value >= 50 && value < 60) {
        STATUS = BAD;
    }
    if (value < 50) {
        STATUS = HAZARD;
    }
    // Draw status bar
    int barLength = 56; // Length of the status bar
    int filledLength = (int)(barLength * (value / maxValue)); // Filled length of the bar
    uLCD.filled_rectangle(x*8-12, (y+3)*8+2, x*8+-12+filledLength, (y+4)*8+2, STATUS); // Adjusted for pixel position
    uLCD.rectangle(x*8-12, (y+3)*8+2, x*8+44, (y+4)*8+2, TEXT_COLOR); // Bar border
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
    uLCD.locate(x+2, label_pos_y);
    uLCD.printf("%s", "Air");
    uLCD.locate(x, label_pos_y+1);
    uLCD.printf("%s", "Quality");
    // Display the value above the status bar
    uLCD.locate(x+1, value_pos_y+2);
    uLCD.printf("%.0f%%", value);

    int STATUS = 0x000000; // Default value

    if (value > 90) {
        STATUS = STATUS_BAR_COLOR;
    }
    if (value >= 80 && value < 90) {
        STATUS = MODE;
    }
    if (value >= 70 && value < 80) {
        STATUS = SENS;
    }
    if (value >= 60 && value < 70) {
        STATUS = RED;
    }
    if (value >= 50 && value < 60) {
        STATUS = BAD;
    }
    if (value < 50) {
        STATUS = HAZARD;
    }

    // Draw colored square/rectangle
    uLCD.filled_rectangle((x+4)*8+2, (y+3)*8, (x+4)*8+12, (y+4)*8, STATUS); // Adjusted for pixel position
}


int main()
{
    //LCD.cls();
    initDisplay();

    // Draw separators first to avoid overwriting
    uLCD.line(0, 42, 127, 42, SEPARATOR_COLOR); // Horizontal separators
    uLCD.line(0, 85, 127, 85, SEPARATOR_COLOR);
    uLCD.line(64, 0, 64, 127, SEPARATOR_COLOR); // Vertical separator
    //pc.printf("Start \r\n");
    //Thread t1(thread1); //start thread1
    //Thread t2(thread2);
    //Thread t3(thread3);
    pc.baud(115200);
    mySI7021status = myTempRHsensor.SI7021_SoftReset();
    wait_ms(15);
    mySI7021status = myTempRHsensor.SI7021_Conf(SI7021::SI7021_RESOLUTION_RH_12_TEMP_14, SI7021::SI7021_HTRE_DISABLED);

    
    p_sensor.begin();  
    while(1){
        noiseval = noisesensor * 100;
        //pc.printf("Noise Level is %f \r\n", noiseval);
        p_sensor.ReadSensor();
        pressure = p_sensor.GetPressure();
        //pc.printf("Pressure    = %f\n", pressure);
        wait(1);
        temp = p_sensor.GetTemperature();
        //pc.printf("Temperature = %f\n", temp);
        wait(1);
        airqval = (100 - gassensor*100);
        //pc.printf("AirQ is %f \r\n", airqval);
        mySI7021status = myTempRHsensor.SI7021_TriggerHumidity(SI7021::SI7021_NO_HOLD_MASTER_MODE);

        // Short delay to allow the sensor to complete the measurement
        wait_ms(500); 

        // Read Humidity result
        mySI7021status = myTempRHsensor.SI7021_ReadHumidity(&myData);
        humidity = myData.RelativeHumidity;
        // Send it through the UART
        //pc.printf("RH: %0.5f%%\r\n", humidity);

        // Short delay before the next reading
    
        //Total Grade
        //if 
        //overallCondition
        //Grade Calc
        if ((1.8 * temp) + 32 > 75){
            tempgrade = 100 - ((1.8 * temp) + 32 - 75)*4;
        }
        else if (tempval < 65){
            tempgrade = 100 - (65 - (1.8 * temp) - 32)*4;
        }
        else {
            tempgrade = 100;
        }
        
        if (humidity > 45){
            humiditygrade = 100 - (humidity - 45)*6;
        }
        else if (humidity < 35){
            humiditygrade = 100 - (35 - humidity)*6;
        }
        else {
            humiditygrade = 100;
        }
        
        if (pressure > 1025){
            pressuregrade = 100 - (pressure - 1025)*3;
        }
        else if (pressure < 995){
            pressuregrade = 100 - (995 - pressure)*3;
        }
        else {
            pressuregrade = 100;
        }
        
        noisegrade = 100 - noiseval;
        
        totalgrade = 0.15*tempgrade + 0.4*airqval + 0.15*pressuregrade + 0.15*humiditygrade + 0.15*noisegrade;
       

        displayTileWithNumericValueTemp((1.8 * temp) + 32, 1, 1, "Temp");//Celsius
        displayTileWithNumericValueHumidity(myData.RelativeHumidity, 10, 1, "Humidty");//%
        displayTileWithStatusBarNoise(noiseval, 1, 6, "Noise", 100.0);//dB
        displayTileWithNumericValuePressure(pressure, 10, 6, "Pressure");//hPa
        displayAirQ(airqval, 1, 11, "Air Qual", airqval);//AQI
        displayTileWithStatusBarCondition(totalgrade, 10, 11, "Overall", 100.0); //Why do we have this?

        wait_ms(5000);
    }
}