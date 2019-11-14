// This #include statement use MQTT Libary from https://github.com/hirotakaster/MQTT 
// You have to install it manually
#include <MQTT.h>
// This #include statement use I2CSoilMoistureSensor Libary from https://github.com/VintageGeek/I2CSoilMoistureSensor
// You have to install it manually
#include <I2CSoilMoistureSensor.h>

#define SOIL_SENSOR_ADDRESS1 0x20
#define SOIL_SENSOR_ADDRESS2 0x21
#define SOIL_SENSOR_ADDRESS3 0x22

// TSL45315 I2C address is 0x29(41)
#define TSL45315_Addr 0x29

/**
 * if want to use IP address,
 * byte server[] = { XXX,XXX,XXX,XXX };
 * MQTT client(server, 1883, callback);
 * want to use domain name,
 * MQTT client("www.sample.com", 1883, callback);
 **/
byte server[] = { 192,168,178,20 };
MQTT client(server, 1883, callback);
// recieve message
void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    
    if(strcmp(topic,"cmnd/iqplants/run")==0) {
        every_minute();
    }
    else if (strcmp(p,"connect")==0) {
        RGB.brightness(64);
        RGB.control(false);
        if(!Particle.connected()) {
            Particle.connect();
            waitFor(Particle.connected,1000);
        }
        client.publish("tele/iqplants/LWT", "Cloud", true);
    }
    else if (strcmp(p,"disconnect")==0) {
        if(Particle.connected()) {
            Particle.disconnect();
        }
        RGB.control(true);
        RGB.brightness(0);
        client.publish("tele/iqplants/LWT", "Online", true);
    }
    delay(1000);
}

I2CSoilMoistureSensor sensor1(SOIL_SENSOR_ADDRESS1);
I2CSoilMoistureSensor sensor2(SOIL_SENSOR_ADDRESS2);
I2CSoilMoistureSensor sensor3(SOIL_SENSOR_ADDRESS3);

String myID = System.deviceID();
Timer timer(300000, every_minute);
ApplicationWatchdog wd(360000, System.reset);

SYSTEM_MODE(SEMI_AUTOMATIC); // Cloud functions disabled
STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));
STARTUP(starttodo());

// disable system LED
void starttodo()
{
    RGB.control(true);
    RGB.brightness(0);

}

void inittls() {
  // Start I2C Transmission
  Wire.beginTransmission(TSL45315_Addr);
  // Select control register
  Wire.write(0x80);
  // Normal operation
  Wire.write(0x03);
  Wire.endTransmission();
  // Start I2C Transmission
  Wire.beginTransmission(TSL45315_Addr);
  // Select configuration register
  Wire.write(0x81);
  // Multiplier 1x, Tint : 400ms
  Wire.write(0x00);
  Wire.endTransmission();
  delay(300);
}

int getlux() {
  int luminance;
  unsigned int data[2];
  // Start I2C Transmission
  Wire.beginTransmission(TSL45315_Addr);
  // Select data register
  Wire.write(0x84);
  Wire.endTransmission();
  // Request 2 bytes of data
  Wire.requestFrom(TSL45315_Addr, 2);
  // Read 2 bytes of data
  // luminance lsb, luminance msb
  if(Wire.available() == 2)
    {
      data[0] = Wire.read();
      data[1] = Wire.read();
      delay(300);
    }
  // Convert the data
  luminance = (data[1] * 256) + data[0];
  // while(Wire.available()){ Wire.read(); } //received more bytes?
  return luminance;
}

void every_minute() {
    waitFor(WiFi.ready,1000);
    
    // Read moisture value
    waitFor(sensor1.isBusy,1000);
    int soilMoisture1 = sensor1.getCapacitance();
    delay(1000);
    
    waitFor(sensor2.isBusy,1000);
    int soilMoisture2 = sensor2.getCapacitance();
    delay(1000);

    waitFor(sensor3.isBusy,1000);
    int soilMoisture3 = sensor3.getCapacitance();
    delay(1000);

    waitFor(sensor1.isBusy,1000);
    int temperature = sensor1.getTemperature();
    delay(1000);
    
    //divide raw temperature reading by 10 to get celsius
    float tempInC=(temperature/(float)10);

    client.publish("sensor/double/wohnzimmer/pflanze1", "{\"Name\":\"Wohnzimmer\",\"Typ\":\"I2C Soil moisture sensor\",\"data\":{\"key\":\"Bodenfeuchte Grünlilie\",\"value\":\"" + String(soilMoisture1) + "\",\"unit\":\"\"}}", true);
    client.publish("sensor/double/wohnzimmer/pflanze2", "{\"Name\":\"Wohnzimmer\",\"Typ\":\"I2C Soil moisture sensor\",\"data\":{\"key\":\"Bodenfeuchte Ficus\",\"value\":\"" + String(soilMoisture2) + "\",\"unit\":\"\"}}", true);
    client.publish("sensor/double/wohnzimmer/pflanze3", "{\"Name\":\"Wohnzimmer\",\"Typ\":\"I2C Soil moisture sensor\",\"data\":{\"key\":\"Bodenfeuchte Geldbaum\",\"value\":\"" + String(soilMoisture3) + "\",\"unit\":\"\"}}", true);
    client.publish("sensor/double/wohnzimmer/temp_p1", "{\"Name\":\"Wohnzimmer\",\"Typ\":\"Temperatur Soil moisture sensor\",\"data\":{\"key\":\"Temperatur Grünlilie\",\"value\":\"" + String(tempInC) + "\",\"unit\":\"°C\"}}", true);
    client.publish("sensor/double/wohnzimmer/lux", "{\"Name\":\"Wohnzimmer\",\"Typ\":\"TSL45315\",\"data\":{\"key\":\"Lux Wohnzimmer\",\"value\":\"" + String(getlux()) + "\",\"unit\":\"LUX\"}}", true);
    
    wd.checkin();  
}

void mqtt_connect() {
    // connect to the server
    client.connect(myID, "<user>", "<pass>", "tele/iqplants/LWT", MQTT::QOS0, true, "Offline", true);
    waitFor(client.isConnected,3000);
    // MQTT subscribe endpoint could have the QoS
    client.subscribe("cmnd/iqplants/particle");
    client.subscribe("cmnd/iqplants/run");
    client.publish("tele/iqplants/LWT", "Online", true);
}

void setup() {
    //Init Wire for I2C Communication
    Wire.begin();

    sensor1.begin();
    sensor2.begin();
    sensor3.begin();
    
    // Init TSL45315
    inittls();
    delay(1000); // give some time to boot up

    WiFi.connect();
    waitUntil(WiFi.ready);
    mqtt_connect();
    // Trigger first time
    every_minute();
    // Trigger every x seconds
    timer.start();
}


void loop() {
    if (!client.isConnected()) mqtt_connect();
    client.loop();
    delay(50);
}
