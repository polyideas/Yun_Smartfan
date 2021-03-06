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

// Sketch Includes
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

// Our interval duration that matches the delay at the end of the script
int intervalInt;
// How often we update Temboo in seconds
int triggerInt = 600;
// Interval for counting time since we renewed our DHCP lease. This is important for most cases when you get an
// automatic IP address- when the address lease expires, the sketch will stop updating Temboo unless we renew the lease
int dhcpcount;
// This is the lease duration in seconds, or ideally a number lower than your lease duration.
// 10800 is 3 hours, so the networking will be restarted every three hours to keep getting a fresh IP.
int dhcpInt = 10800;

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
// Temperature variables
float tC;
float t;

void setup() {
  // Initialize the Adafruit DHT library
  dht.begin();

  // Initialize the Yun Bridge library
  Bridge.begin();

  // Initialize the Pololu MD motor library
  md.init(); // Motor control init

}


void loop() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float tC = dht.readTemperature();

  //convert from Celsius to Farenheit
  t = ((tC * 1.8) + 32);

  // Lowest temperature threshold means the fans are off
  if (t <= 80)
  {
    motorAspeed = 0;
    motorBspeed = 0;
    md.setM1Speed(motorAspeed);  // Set motor A
    md.setM2Speed(motorBspeed);  // Set motor B
  }

  // Turn on the upper cabinet fan on a low speed
  if (t > 80)
  {
    motorAspeed = 100;
    motorBspeed = 0;
    md.setM1Speed(motorAspeed);  // Set motor A
    md.setM2Speed(motorBspeed);  // Set motor B
  }

  // Turn on both input and exhaust fans on 50% speed
  if (t > 90)
  {
    motorAspeed = 200;
    motorBspeed = 200;
    md.setM1Speed(motorAspeed);  // Set motor A
    md.setM2Speed(motorBspeed);  // Set motor B
  }

  // Turn on both fans at 75% speed
  if (t > 100)
  {
    motorAspeed = 300;
    motorBspeed = 300;
    md.setM1Speed(motorAspeed);  // Set motor A
    md.setM2Speed(motorBspeed);  // Set motor B
  }

  // Turn on both fans at full speed
  if (t > 110)
  {
    motorAspeed = 400;
    motorBspeed = 400;
    md.setM1Speed(motorAspeed);  // Set motor A
    md.setM2Speed(motorBspeed);  // Set motor B
  }

// Let's define a process() so we can call the Linux command to restart the network stack
  Process p;

  // run again if it's been 60 seconds since we last ran
  if (dhcpcount > dhcpInt) {
    // reset the interval to 0
    dhcpcount = 0;
    // Restart the network stack to renew our lease.
    p.runShellCommand("/etc/init.d/network  restart");
    // Don't do anything while we are waiting for the command to complete.
    while (p.running());
  }

  // run again if it's been 60 seconds since we last ran
  if (intervalInt == triggerInt) {
    // reset the interval to 0
    intervalInt = 0;

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
      Serial.println("Appended " + rowData);
      Serial.println("");
      Serial.println(intervalInt);
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
  intervalInt = (intervalInt + 3);
  dhcpcount = (dhcpcount + 3);
  Serial.println(intervalInt);
}
