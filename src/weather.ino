// This #include statement use MQTT Libary from https://github.com/hirotakaster/MQTT 
// You have to install it manually
#include <MQTT.h>


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
    
    if(strcmp(topic,"cmnd/iqwather/run")==0) {
        every_minute();
    }
    else if (strcmp(p,"connect")==0) {
        RGB.brightness(64);
        RGB.control(false);
        if(!Particle.connected()) {
            Particle.connect();
            waitFor(Particle.connected,1000);
        }
        client.publish("tele/iqwather/LWT", "Cloud", true);
    }
    else if (strcmp(p,"disconnect")==0) {
        if(Particle.connected()) {
            Particle.disconnect();
        }
        RGB.control(true);
        RGB.brightness(0);
        client.publish("tele/iqwather/LWT", "Online", true);
    }
    delay(1000);
}


String myID = System.deviceID();
Timer timer(60000, every_minute);
ApplicationWatchdog wd(360000, System.reset);
SYSTEM_MODE(SEMI_AUTOMATIC); // Cloud functions disabled
STARTUP(starttodo());

// disable system LED
void starttodo()
{
    RGB.control(true);
    RGB.brightness(0);
}

void every_minute() {
    wd.checkin();  
}

void mqtt_connect() {
    // connect to the server
    client.connect(myID, "<user>", "<pass>", "tele/iqwather/LWT", MQTT::QOS0, true, "Offline", true);
    waitFor(client.isConnected,3000);
    client.subscribe("cmnd/iqwather/particle");
    client.subscribe("cmnd/iqwather/run");
    client.publish("tele/iqwather/LWT", "Online", true);
}

void setup() {
    if(!WiFi.ready()) WiFi.connect();
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
