
#include "mbed.h"
#include "rtos.h"
#include "uLCD_4DGL.h"
#include "MPL115A2.h"
#include "DHT.h"

uLCD_4DGL LCD(p9,p10,p11); // serial tx, serial rx, reset pin;
DHT tempsensor(p23,SEN11301P); // Use the SEN11301P sensor
I2C i2c(p28, p27);
AnalogIn gassensor(p15);
AnalogIn noisesensor(p16);

MPL115A2 p_sensor(&i2c);

Mutex lcd_mutex;

float tempval;
float humidval;
float dewpoint;
float pressureval;
float noiseval;
float airqval;

// Thread 1
// temperature/humidity/dewpoint reading and displaying

void thread1(void const *args)
{
    int err;
    wait(1);
    while(true) {       // thread loop
        //err = tempsensor.readData();
        //printf("Error = %i", err);

        //Here is where we read temperature
        tempval = tempsensor.ReadTemperature(FARENHEIT);
        humidval = tempsensor.ReadHumidity();
        dewpoint = tempsensor.CalcdewPointFast(tempsensor.ReadTemperature(CELCIUS), humidval);
        
        lcd_mutex.lock();
        printf("Temperature is %4.2f F \r\n", tempval);
        printf("Humidity is %4.2f  \r\n", humidval);
        printf("Dew Point is %4.2f  \r\n", dewpoint);
        lcd_mutex.unlock();
        Thread::wait(5000);
    }
}

// Thread 2
// pressure reading and displaying
void thread2(void const *args)
{
    p_sensor.begin();
    while(true) {         // thread loop
        p_sensor.ReadSensor();
        pressureval = p_sensor.GetPressure();
        tempval = p_sensor.GetTemperature();

        lcd_mutex.lock();
        printf("Pressure is %4.2f \r\n", pressureval);
        printf("Temperature is %4.2f \r\n", tempval);
        lcd_mutex.unlock();
        Thread::wait(5000);    // wait 1s
    }
}

// Thread 3
// RGB LED
void thread3(void const *args)
{
    while(true) {         // thread loop
        airqval = gassensor;

        lcd_mutex.lock();
        printf("AirQ is %f \r\n", airqval);
        lcd_mutex.unlock();
        Thread::wait(5000);    // wait 1s   
    }
}

int main()
{
    LCD.cls();
    printf("Start \r\n");
    //Thread t1(thread1); //start thread1
    //Thread t2(thread2);
    //Thread t3(thread3);

    while(1){
        noiseval = noisesensor;
        
        lcd_mutex.lock();
        printf("Noise Level is %f \r\n", noiseval);
        lcd_mutex.unlock();
        Thread::wait(5000);
    }
}
