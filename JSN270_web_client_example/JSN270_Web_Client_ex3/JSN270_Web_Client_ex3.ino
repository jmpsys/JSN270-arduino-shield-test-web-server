#include <Debug.h>
#include <JSN270.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <math.h>

#define SSID      "JSN270_2G"    // your wifi network SSID
#define KEY       "12345678"    // your wifi network password
#define AUTH       "WPA2"     // your wifi network security (NONE, WEP, WPA, WPA2)

#define USE_DHCP_IP 1

#if !USE_DHCP_IP
#define MY_IP          "192.168.1.133"
#define SUBNET         "255.255.255.0"
#define GATEWAY        "192.168.1.254"
#endif

#define HOST_IP        "74,125,232,128" // www.google.com
#define REMOTE_PORT    8200
#define PROTOCOL       "TCP"

SoftwareSerial mySerial(3, 2); // RX, TX
 
JSN270 JSN270(&mySerial);

int sensorPin = A0; // select the input pin for the potentiometer
 
double Thermistor(int RawADC) {
  double Temp;
  Temp = log(10000.0*((1024.0/RawADC-1))); 
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
  Temp = Temp - 273.15;            // Convert Kelvin to Celcius
   //Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
   return Temp;
}

void setup() {
  char c;
  String hostname;
  char hostip[32];

  mySerial.begin(9600);
  Serial.begin(9600);

  Serial.println("--------- JSN270 Web Client Test --------");

  // wait for initilization of JSN270
  delay(5000);
  //JSN270.reset();
  delay(1000);

  //JSN270.prompt();
  JSN270.sendCommand("at+ver\r");
  delay(5);
  while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
    Serial.print((char)c);
  }
  delay(1000);

#if USE_DHCP_IP
  JSN270.dynamicIP();
#else
  JSN270.staticIP(MY_IP, SUBNET, GATEWAY);
#endif    
    
  if (JSN270.join(SSID, KEY, AUTH)) {
    Serial.println("WiFi connect to " SSID);
  }
  else {
    Serial.println("Failed WiFi connect to " SSID);
    Serial.println("Restart System");

    return;
  }
  delay(1000);

  JSN270.sendCommand("at+wstat\r");
  delay(5);
  while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
    Serial.print((char)c);
  }
  delay(1000);        

  JSN270.sendCommand("at+nstat\r");
  delay(5);
  while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
    Serial.print((char)c);
  }
  delay(1000);

  hostname = "192.168.43.87";
  
  Serial.print("Host IP is ");
  Serial.println(hostname);
  hostname.toCharArray(hostip, hostname.length()+1);  // convert string to char array
  
  if (!JSN270.client(hostip, REMOTE_PORT, PROTOCOL)) {
    Serial.print("Failed connect to ");
    Serial.println(hostip);
    Serial.println("Restart System");
  } else {
    Serial.print("Socket connect to ");
    Serial.println(hostip);
    //delay(2000);
    
    // Enter data mode
    JSN270.sendCommand("at+exit\r");
    delay(100);

    Serial.println("connected to server");
  }
}

unsigned long prev_time=0;

void loop() {

 unsigned long current_time = millis();
 int readVal=analogRead(sensorPin);
 double temp =  Thermistor(readVal);
 
 if(current_time - prev_time >10000){
  JSN270.print("GET /?temp=");
  JSN270.print(temp);
  JSN270.println("&humi=65 HTTP/1.1");
  JSN270.println("Host: 192.168.43.87");
  JSN270.println();
  prev_time = current_time;
  }
 
  while(JSN270.available()){
    Serial.write((char)JSN270.read());  
  } 
}
