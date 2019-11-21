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
}

String myID = System.deviceID();
SYSTEM_MODE(SEMI_AUTOMATIC); // Cloud functions disabled
// STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));
STARTUP(starttodo());

// disable system LED
void starttodo()
{
    RGB.control(true);
    RGB.brightness(0);
}

void mqtt_connect() {
    // connect to the server
    client.connect(myID, "<user>", "<pass>", "tele/switch/LWT", MQTT::QOS1, true, "sleep", true);
    waitFor(client.isConnected,3000);
}

void setup() {
    WiFi.connect();
    pinMode(D1, INPUT_PULLDOWN);
    pinMode(D2, INPUT_PULLDOWN);
    pinMode(D4, INPUT_PULLDOWN); 
    pinMode(D3, INPUT_PULLDOWN);
    pinMode(D5, INPUT_PULLDOWN);
}

void loop() {
    //  put the entire device into a stop mode with wakeup on interrupt
    // CHANGE to trigger the interrupt whenever the pin changes value,
    // RISING to trigger when the pin goes from low to high,
    // FALLING for when the pin goes from high to low.
    SleepResult result = System.sleepResult();
    pin_t pin = result.pin();
    if (result.wokenUpByPin()) {
        waitFor(WiFi.ready,3000);
        if (!client.isConnected()) mqtt_connect();
        client.publish("cmnd/photon/switch" + String(pin), "1", false);
        delay(50);
    }
    System.sleep({D1, D2, D3, D4, D5, D6, D7}, RISING);
}
