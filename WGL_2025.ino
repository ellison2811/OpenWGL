//SD card
#include <SPI.h>  //This library allows you to communicate with SPI devices, with the Arduino as the controller device. (SD CARD)
#include <SD.h>   //Enables reading and writing on SD cards.


//depth
#include <KellerLD.h>
#include <Wire.h>
KellerLD DepthSensor;

//pH
#include <SoftwareSerial.h>
SoftwareSerial pHserial(4, 5);

//Eh
SoftwareSerial Ehserial(6, 7);

//Cond
SoftwareSerial Condserial(2, 3);



void setup() {
  //Computer/USB serial
  Serial.begin(9600);

  // SD setup
  init_SD();

  //Depth setup
  init_depth();

  //pH setup
  init_pH();

  //Eh setup
  init_Eh();

  // EC sensor setup
  init_Cond();
}

void loop() {
  //Read Depth
  DepthSensor.read();

  // //Measure pH
  float pH = read_pH();

  //Measure Eh
  float Eh = read_Eh();

  //Measure Cond
  float EC = read_Cond();

  SD.begin(14);
  File dataFile = SD.open("WGL_LOG.TXT", FILE_WRITE);
  delay(30);
  if (!dataFile) {
    File dataFile = SD.open("WGL_LOG.TXT", FILE_WRITE);
    Serial.println(F("try again."));
  } else {
    dataFile.print(DepthSensor.pressure());
    dataFile.print(F(", "));
    dataFile.print(DepthSensor.depth());
    dataFile.print(F(", "));
    dataFile.print(DepthSensor.temperature());
    dataFile.print(F(", "));
    dataFile.print(pH);
    dataFile.print(F(", "));
    dataFile.print(Eh);
    dataFile.print(F(", "));
    dataFile.print(EC);
    dataFile.println();

    Serial.print(DepthSensor.pressure());
    Serial.print(F(", "));
    Serial.print(DepthSensor.depth());
    Serial.print(F(", "));
    Serial.print(DepthSensor.temperature());
    Serial.print(F(", "));
    Serial.print(pH);
    Serial.print(F(", "));
    Serial.print(Eh);
    Serial.print(F(", "));
    Serial.print(EC);
    Serial.println();
  }

  dataFile.close();
}

void init_SD() {
  const int chipSelect = 14;
  if (!SD.begin(chipSelect)) {  // if it doesnt begin
    Serial.println(F("Card failed, or not present"));
  } else {
    File dataFile = SD.open("wgl_log.txt", FILE_WRITE);  // open the file. note that only one file can be open at a time
    delay(30);
    if (dataFile) {
      dataFile.println(F("pressure_mbar, depth_m, temp_C, pH_mV, Eh_mV, cond_uscm"));
      dataFile.close();
      Serial.println(F("SD initialized."));
    } else {
      Serial.println(F("Error initializing log file"));
    }
  }
}

void init_depth() {
  Wire.begin();                  //This function initializes the Wire library and join the I2C bus as a controller or a peripheral. This function should normally be called only once.
  DepthSensor.init();
  while (!DepthSensor.isInitialized()) {  // Returns true if initialization was successful
    Serial.println(F("Init failed!"));
    Serial.println(F("Are SDA/SCL connected correctly?"));
    Serial.println(F("Blue Robotics Bar30: White=SDA, Green=SCL"));
    Serial.println(F("\n\n\n"));
    delay(1000);
    DepthSensor.init();
  }
  Serial.println(F("Bar100 init successful!"));
  // DepthSensor.setModel(MS5837::MS5837_30BA);
  DepthSensor.setFluidDensity(997);  // kg/m^3 (freshwater, 1029 for seawater)
  Serial.println(F("Bar100 setup complete"));
}

void init_pH() {
  pHserial.begin(9600);
  pHserial.listen();
  pHserial.println(F("Baud,38400"));
  pHserial.begin(38400);  
  pHserial.println("Cal,clear");  //clear any calibration
  pHserial.println("C,0");        // Enter single reading mode
  pHserial.println("*OK,0");
}

void init_Eh() {
  Ehserial.begin(9600);
  Ehserial.listen();
  Ehserial.println(F("Baud,38400"));
  Ehserial.begin(38400);
  Ehserial.println(F("Cal,clear"));  //clear any calibration
  Ehserial.println(F("C,0"));        // Enter single reading mode
  Ehserial.println(F("*OK,0"));
}

void init_Cond() {
  Condserial.begin(9600);  //set baud rate for the software serial port to 9600
  Condserial.listen();
  Condserial.println(F("Baud,38400"));
  Condserial.begin(38400);
  Condserial.println(F("K,0.1"));
  Condserial.println(F("Cal,clear"));  //clear any calibration
  Condserial.println(F("C,0"));        // Enter single reading mode
  Condserial.println(F("*OK,0"));
}

float read_pH() {
  pHserial.listen();
  pHserial.println(F("R,25.0"));
  delay(20);
  String pHstring = "";
  boolean pH_string_complete = false;
  int iter = 0;
  while (!pH_string_complete) {
    if (pHserial.available() > 0) {         //if we see that the Atlas Scientific product has sent a character
      char inchar = (char)pHserial.read();  //get the char we just received
      pHstring += inchar;                   //add the char to the var called sensorstring
      if (inchar == '\r') {                 //if the incoming character is a <CR>
        pH_string_complete = true;          //set the flag
      }
    }
    iter += 1;
    if (iter >= 50) {
      pHstring = "-999";
      break;
    }
    delay(20);
  }
  return (pHstring.toFloat()-7) * -59.16;
}

float read_Eh() {
  Ehserial.listen();
  Ehserial.println(F("R"));
  delay(20);
  int iter = 0;
  boolean Eh_string_complete = false;
  String Ehstring = "";

  while (!Eh_string_complete) {
    if (Ehserial.available() > 0) {                                  //if we see that the Atlas Scientific product has sent a character
      char inchar = (char)Ehserial.read();                           //get the char we just received
      Ehstring += inchar;  //add the char to the var called sensorstring
      if (inchar == '\r') {                                          //if the incoming character is a <CR>
        Eh_string_complete = true;                                   //set the flag
      }
    }
    iter += 1;
    if (iter >= 50) {
      Ehstring = "-999";
      break;
    }
    delay(20);
  }
  return Ehstring.toFloat();
}

float read_Cond() {
  Condserial.listen();
  Condserial.println(F("R"));
  delay(20);
  int iter = 0;
  boolean Cond_string_complete = false;
  String Condstring = "";
  while (!Cond_string_complete) {
    if (Condserial.available() > 0) {         //if we see that the Atlas Scientific product has sent a character
      char inchar = (char)Condserial.read();  //get the char we just received
                                              // if(isdigit(inchar) || inchar == ".") {
      Condstring += inchar;
      // }                           //add the char to the var called sensorstring
      if (inchar == '\r') {           //if the incoming character is a <CR>
        Cond_string_complete = true;  //set the flag
      }
    }
    iter += 1;
    if (iter >= 50) {
      Condstring = "-999";
      break;
    }
    delay(20);
  }

  char condstring_array[30];  //we make a char array
  char *EC;                   //char pointer used in string parsing

  Condstring.toCharArray(condstring_array, 30);  //convert the string to a char array
  EC = strtok(condstring_array, ",");            //let's pars the array at each comma

  return float(Condstring.toFloat());
}