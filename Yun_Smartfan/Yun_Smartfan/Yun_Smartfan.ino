// Example testing sketch for various DHT humidity/temperature sensors
// DHT22 code originally from sample code written by ladyada, public domain from Adafruit
// Google App spreadsheet code from Temboo
// Code from both great sources mixed and matched by Jay Doscher www.polyideas.com

//This project uses the following components:
// Arduino Yun: http://www.amazon.com/gp/product/B00F6YJK3S/ref=as_li_ss_tl?ie=UTF8&camp=1789&creative=390957&creativeASIN=B00F6YJK3S&linkCode=as2&tag=86753090e-20
// Pololu Motor Control Shield (WARNING: Disable the power jumper on this board- if you don't this shield will fry the Yun! http://www.pololu.com/product/2503
// Adafruit DHT-22: http://www.adafruit.com/products/385
// Sparkfun Protoshield: https://www.sparkfun.com/products/11665
// Stackable headers: http://www.adafruit.com/products/85
// Breakaway headers: http://www.adafruit.com/products/392

#define TEMBOO_ACCOUNT "your_temboo_account"  // your Temboo account name 
#define TEMBOO_APP_KEY_NAME "your_app_key_name"  // your Temboo app key name
#define TEMBOO_APP_KEY  "your_app_key"  // your Temboo app key

/*
This sketch uses samples from the following sketches:
https://www.temboo.com/arduino/update-google-spreadsheet
https://github.com/adafruit/DHT-sensor-library

More information and projects can be found at:
http://polyideas.com
*/
#include "DualMC33926MotorShield.h"

#include <Bridge.h>
#include <Temboo.h>

/*** SUBSTITUTE YOUR VALUES BELOW: ***/

// Note that for additional security and reusability, you could
// use #define statements to specify these values in a .h file.

const String GOOGLE_USERNAME = "your_gmail_account@gmail.com";
const String GOOGLE_PASSWORD = "your_gmail_password";

// the title of the spreadsheet you want to send data to
// (Note that this must actually be the title of a Google spreadsheet
// that exists in your Google Drive/Docs account, and is configured
// as described above.)
const String SPREADSHEET_TITLE = "your_spreadsheet_name";

const unsigned long RUN_INTERVAL_MILLIS = 300000; // how often to run the Choreo (in milliseconds)

// the last time we ran the Choreo
// (initialized to 60 seconds ago so the
// Choreo is run immediately when we start up)
unsigned long lastRun = (unsigned long) - RUN_INTERVAL_MILLIS;


//Initialize the Adafruit DHT library, which you can get here:
// https://github.com/adafruit/DHT-sensor-library
#include "DHT.h"


#define DHTPIN 2     // what pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND

DHT dht(DHTPIN, DHTTYPE);

// Motor definitions to make life easier:
DualMC33926MotorShield md;
int motorAspeed;
int motorBspeed;
float tC;
float t;

void setup() {

  dht.begin();
  Bridge.begin();
  md.init(); // Motor control init

}


void loop() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float tC = dht.readTemperature();
  //convert from C to F
  t = ((tC * 1.8)+32);
  
  if (t < 70)
  {
    motorAspeed = 0;
    motorBspeed = 0;
    md.setM1Speed(motorAspeed);  // Set motor A
    md.setM2Speed(motorBspeed);  // Set motor B
  }
  if (t > 80)
  {
    motorAspeed = 100;
    motorBspeed = 0;
    md.setM1Speed(motorAspeed);  // Set motor A
    md.setM2Speed(motorBspeed);  // Set motor B
  }
  if (t > 90)
  {
    motorAspeed = 200;
    motorBspeed = 200;
    md.setM1Speed(motorAspeed);  // Set motor A
    md.setM2Speed(motorBspeed);  // Set motor B
  }

  if (t >= 100)
  {
    motorAspeed = 300;
    motorBspeed = 300;
    md.setM1Speed(motorAspeed);  // Set motor A
    md.setM2Speed(motorBspeed);  // Set motor B
  }

  if (t > 110)
  {
    motorAspeed = 400;
    motorBspeed = 400;
    md.setM1Speed(motorAspeed);  // Set motor A
    md.setM2Speed(motorBspeed);  // Set motor B
  }

  // get the number of milliseconds this sketch has been running
  unsigned long now = millis();

  // run again if it's been 60 seconds since we last ran
  if (now - lastRun >= RUN_INTERVAL_MILLIS) {

    // remember 'now' as the last time we ran the choreo
    lastRun = now;

    // get the value we want to append to our spreadsheet
    //    unsigned long sensorValue = getSensorValue();

    Serial.println("Appending value to spreadsheet...");

    // we need a Process object to send a Choreo request to Temboo
    TembooChoreo AppendRowChoreo;

    // invoke the Temboo client
    // NOTE that the client must be reinvoked and repopulated with
    // appropriate arguments each time its run() method is called.
    AppendRowChoreo.begin();

    // set Temboo account credentials
    AppendRowChoreo.setAccountName(TEMBOO_ACCOUNT);
    AppendRowChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    AppendRowChoreo.setAppKey(TEMBOO_APP_KEY);

    // identify the Temboo Library choreo to run (Google > Spreadsheets > AppendRow)
    AppendRowChoreo.setChoreo("/Library/Google/Spreadsheets/AppendRow");

    // set the required Choreo inputs
    // see https://www.temboo.com/library/Library/Google/Spreadsheets/AppendRow/
    // for complete details about the inputs for this Choreo

    // your Google username (usually your email address)
    AppendRowChoreo.addInput("Username", GOOGLE_USERNAME);

    // your Google account password
    AppendRowChoreo.addInput("Password", GOOGLE_PASSWORD);

    // the title of the spreadsheet you want to append to
    AppendRowChoreo.addInput("SpreadsheetTitle", SPREADSHEET_TITLE);


    Process time;
    time.runShellCommand("date");
    String timeString = "";
    while (time.available()) {
      char c = time.read();
      timeString += c;
    }


    // convert the time and sensor values to a comma separated string
    String rowData(timeString);
    rowData += ",";
    rowData += t;
    rowData += ",";
    rowData += h;
    rowData += ",";
    rowData += motorAspeed;
    rowData += ",";
    rowData += motorBspeed;

    // add the RowData input item
    AppendRowChoreo.addInput("RowData", rowData);

    // run the Choreo and wait for the results
    // The return code (returnCode) will indicate success or failure
    unsigned int returnCode = AppendRowChoreo.run();

    // return code of zero (0) means success
    if (returnCode == 0) {
      Serial.println("Success! Appended " + rowData);
      Serial.println("");
    } else {
      // return code of anything other than zero means failure
      // read and display any error messages
      while (AppendRowChoreo.available()) {
        char c = AppendRowChoreo.read();
        Serial.print(c);
      }
    }

    AppendRowChoreo.close();
  }
  delay(3000);
}
