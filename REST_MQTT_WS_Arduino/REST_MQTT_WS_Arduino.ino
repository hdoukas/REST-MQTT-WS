
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <cc3000_PubSubClient.h>
#include <utility/netapp.h>
#include <ChainableLED.h>

#include <avr/pgmspace.h>

#define aref_voltage 3.3

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!

// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER);

#define WLAN_SSID       "IOTLBAB"
#define WLAN_PASS       ""

// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_UNSEC

unsigned long aucDHCP = 14400;
unsigned long aucARP = 3600;
unsigned long aucKeepalive = 10;
unsigned long aucInactivity = 0;

const unsigned long
  connectTimeout  = 15L * 1000L, // Max time to wait for server connection
  responseTimeout = 15L * 1000L; // Max time to wait for data from server

Adafruit_CC3000_Client client;
   
union ArrayToIp {
  byte array[4];
  uint32_t ip;
};

uint32_t ip;
//uint32_t broker_ip = {83, 212, 96, 61};
ArrayToIp server = { 61, 96, 212, 83 };

#define sensor_pin 6


cc3000_PubSubClient mqttclient(server.ip, 1883, callback, client, cc3000);

ChainableLED leds(7, 8, 1);

int in_counter = 0;

void callback (char* topic, byte* payload, unsigned int length) {
 
 //check for payload and set LED color accordingly 
  if(payload[0]=='1')
        leds.setColorRGB(0, 255, 0, 0);
      if(payload[0]=='2')
        leds.setColorRGB(0, 0, 255, 0);
      if(payload[0]=='3')
        leds.setColorRGB(0, 0, 0, 255);
  
}

void setup(void)
{
  Serial.begin(9600);
  Serial.println(F("Hello, CC3000!\n"));
  
  
  // If you want to set the aref to something other than 5v
  analogReference(EXTERNAL);
  
  pinMode(sensor_pin,INPUT);
  
  Serial.println(F("\nInitialising the CC3000 ..."));
  if (!cc3000.begin()) {
    Serial.println(F("Unable to initialise the CC3000! Check your wiring?"));
    for(;;);
  }

 
   if (netapp_timeout_values(&aucDHCP, &aucARP, &aucKeepalive, &aucInactivity) != 0) {
      Serial.println(F("Error setting inactivity timeout!"));
    }


  /* Attempt to connect to an access point */
  char *ssid = WLAN_SSID;             /* Max 32 chars */
  Serial.print(F("\nAttempting to connect to ")); Serial.println(ssid);
  
  /* NOTE: Secure connections are not available in 'Tiny' mode! */
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()) {
    delay(100); // ToDo: Insert a DHCP timeout!
  }
  
  //green colour
  leds.setColorRGB(0, 0, 255, 0);
  
  delay(1000);

 
  
   // connect to the broker
   while (!client.connected()) {
     Serial.println(F("connecting to broker..."));
     client = cc3000.connectTCP(server.ip, 1883);
   }
   
   // did that last thing work? sweet, let's do something
   if(client.connected()) {
     Serial.println("Connected to broker");
    if (mqttclient.connect("CC3000")) {
      //subscribe to  topic
      bool check  = mqttclient.subscribe("meesage_in");
      //set LED color to blue 
      leds.setColorRGB(0, 0, 0, 255);  
      Serial.println(check);
     
    }
   } 
   
}

boolean error = false;

long time = millis();

int sense_counter  = 0;

void loop(void) {
 
  //check if sensor is triggered...to remove noise don't send message until
   if(digitalRead(sensor_pin)==LOW)  {
     sense_counter++;
      if(sense_counter > 3) {
        sense_counter = 0;
        mqttclient.publish("message_in", "1");
        delay(100);
      }
    }
    
  mqttclient.loop();
}

