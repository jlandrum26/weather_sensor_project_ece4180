# Environment Monitor
![Environment Monitor](uLCD.heic)

Our Environment Monitor is a comprehensive system managed by mbed, designed to measure and display various environmental parameters. It provides real-time data on temperature, humidity, air pressure, ambient noise level, and air quality, all shown on a uLCD display.


Team Members

John Landrum, Nikola Porto, Zekai Wang



Components

mbed LPC1768: Main control unit

Adafruit MiCS 5524 Sensor: Gas sensor for air quality

μLCD-144-G2 Display: For data visualization

MPL115A2 Sensor: Measures barometric pressure & temperature

SPW2430 Sensor: MEMS microphone for noise level detection

Adafruit Si7021 Sensor: Measures temperature & humidity

![Environment Monitor](Overview.HEIC)
Features

Plug-and-play operation

Real-time environmental data monitoring

Color LCD for clear data display

Code Used: [main.cpp](Environment Monitor/main.cpp)
Library Used: 

[Si7012 Temperature & Humidity Sensor](https://os.mbed.com/users/mcm/code/SI7021/)
[MPL115A2 - Barometer and temperature sensor](https://os.mbed.com/components/MPL115A2/)
[uLCD-144-G2 128 by 128 Smart Color LCD](https://os.mbed.com/users/4180_1/notebook/ulcd-144-g2-128-by-128-color-lcd/)


Installation

(Here, provide detailed instructions on how to install and set up your monitor, including any necessary hardware and software configurations.)

Usage

(Include a brief guide on how to use the monitor, with examples or screenshots if possible.)

Future Improvements

Making the system more compact and portable
Adding a casing and internal battery
Integrating more sensors for diverse environmental data
Enabling internet connectivity and cloud data upload
Implementing remote warnings for environmental changes
Contributing

(Explain how others can contribute to your project, guidelines for pull requests, and issue reporting.)

License

(Indicate the license under which your project is released.)

Contact

(Provide contact information for support or inquiries.)


