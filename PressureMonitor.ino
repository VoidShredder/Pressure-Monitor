/*
 * PressureMonitor code that detects whether an LED light is on (or off), and sends the information , along with a timestamp via Ethernet
 * Draft and concept by Rick Momoi
 * 11/21/2016
 * 
 * 
 * Currently needs: A way to synchornize with the server clock (probably once every day is good enough)
 * A better way to double check that the interrupt didn't die on us, and probably a manual override system.
 * Maybe more efficient coding that would reduce memory allocation.
 * The entire send_data section. Right now it's pretty much pseudocode.
 */


#include <Time.h>
#include <TimeLib.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h>  


//Pin 4 acts as a 5V power supply for the circuit
//Pin 3 reads the digital signal and determines whether the light is on or off
//Pin 5 acts as the 0V ground for the circuit.
#define CIRCUIT_POWER 4
#define READING_INPUT 3
#define CIRCUIT_GROUND 5

//Tells if the interrupt on Pin 3 is activated and no one has yet sent the data
volatile bool state_changed = false;

//Timestamp that tells us when the last signal was sent to the DAQ. We only need precision to the nearest second.
time_t last_check = now();


//The structure that we will actually send via Ethernet.
struct PressureMonitor{
  //If the light is on, notVacuum should be set to TRUE
  volatile bool notVacuum = false;
  //The timestamp.
  unsigned long timestamp = millis();
  
  //Since the millis restarts after about 50 days, we add in a secondary time counter that records to the nearest second. Additionally, in the case of power outages, this would retain
  //the timestamp somewhat, whereas any other method would fuck up all the data.
  float actual_time = (float) now();
  //Indicates whether this is a test signal, just to make sure that the Arduino didn't die.
  bool isTest = false;
};

volatile struct PressureMonitor pm;
//Setup the Ethernet connection. Will edit as soon as we find out these details.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0x00, 0xC4};

//Subject to change
IPAddress ip(192, 168, 1, 254);
IPAddress remoteip(192, 168, 1, 222);
unsigned int localPort = 8888;
unsigned int remoteP = 8742;
EthernetUDP Udp;

void setup() {
  //Nothing unintuitive here
  pinMode(CIRCUIT_POWER, OUTPUT);
  pinMode(READING_INPUT, INPUT);
  pinMode(CIRCUIT_GROUND, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(READING_INPUT), Pressure_alert,CHANGE);
  digitalWrite(CIRCUIT_POWER, HIGH);
  digitalWrite(CIRCUIT_GROUND, LOW);
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
}


void loop() {
 //If there is a delay here, please remove it. I may have glossed over it. FYI this code is detecting a signal that is on the order of 1 milliseconds. 
 delay(100);
 
 //In essence, if the interrupt has been triggered and we haven't sent this information over Ethernet, do so.
 if (state_changed){
  pm.timestamp = millis();
  pm.actual_time = (float) now();
  pm.isTest = false;
  send_data();
  //We want to indicate that we sent the information, so we set this to false afterwards. We should probably make sure that the sending succeeded.
  state_changed = false;
 }
 //If there has been no recorded activity since the last minute, we would like to send information about the pin, just to let the people know that the Arduino isn't broken.
 else if ((abs(now() - last_check)) > 10){
  //We want to make sure the trigger didn't malfunction. This is a cautionary step. I should probably rewrite this later on to be extra careful.
  pm.notVacuum = digitalRead(READING_INPUT);
  pm.timestamp = millis();
  pm.actual_time = (float) now();
  last_check = now();
  pm.isTest = true;
  send_data();
 }
}

void send_data()
{
  Udp.beginPacket(remoteip, remoteP);
  Udp.write(((byte*)&pm), sizeof(struct PressureMonitor));
  Udp.endPacket();  
}
void Pressure_alert(){
  pm.notVacuum = !pm.notVacuum;
  state_changed = true;
  pm.timestamp = millis();
  pm.actual_time = (float) now();  
  last_check = now();
 }

