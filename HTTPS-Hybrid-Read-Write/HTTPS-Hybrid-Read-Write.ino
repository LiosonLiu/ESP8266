/*
Describtion:
Portal Alias: LDR, light
Write LDR data and read light data at same time
Use HTTPS port 443
*/
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
const int LDR 		= A0;
const int BUTTON 	= 4;
const int RED 		= 15;
const int GREEN 	= 12;
const int BLUE 		= 13;

//const char* ssid = "RD_Steve";
//const char* password = "RD_Steve";
const char* ssid = "exosite_spg_02";
const char* password = "ex0p@ssw0rd";

const char* host = "m2.exosite.com";
const int httpsPort = 443;
const char* cik  = "550e7c298eec90d77f98d448b8fb2fd4ccd7dc62";
const char* alias = "light";

const char* fingerprint = "AC 39 F8 6F EB 25 A7 4F E9 29 2B E0 E4 62 EA 5F 90 25 C3 07";

const long RPCinterval = 3000;//3sec
const long LEDinterval = 2000;//2sec
unsigned long RPCpreviousMillis,LEDpreviousMillis;
String light;

WiFiClientSecure Client;

//===========================
void connectWiFi()
//===========================
{
  byte ledStatus = LOW;
  WiFi.mode(WIFI_AP_STA);

  Serial.print("connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    analogWrite(RED, random(0,1023));
    analogWrite(GREEN, random(0,1023));
    analogWrite(BLUE, random(0,1023));
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//===========================
void getDataSourcesData(WiFiClientSecure& client)
//===========================
{
Serial.println("HTTP Call");
String url = "/onep:v1/stack/alias?" + String(alias);
String PostData = "LDR="+String(analogRead(LDR));
client.print("POST ");
client.print(url);
client.println(" HTTP/1.1");
client.print("Host: ");
client.println(host);
client.println("X-Exosite-CIK: " +String(cik));
client.println("Accept: application/x-www-form-urlencoded; charset=utf-8");
client.println("Content-Type: application/x-www-form-urlencoded; charset=utf-8");
client.print("Content-Length: ");
client.println(PostData.length());
client.println();
client.println(PostData);
//delay(500);
/*
while(client.available())
	{
  String line = client.readStringUntil('\r');
  Serial.print(line);
	}
*/	
while (client.connected()) 
 	{
  String line = client.readStringUntil('\n');
  Serial.println(line);
  if (line == "\r") 
    break;
	}
String line = client.readStringUntil('\n');
Serial.println("reply was:" + String(line));
/*
	if (line.startsWith("light=")) 
		{
    Serial.println("RPC call successfull!");
  	} 
  else 
  	{
    Serial.println("RPC call has failed");
    return;
  	}
int str_len = line.length() + 1;
light = line.substring(6);
Serial.println("light="+light);
*/
}
//=================
unsigned StartConnect(void)
//=================
{	
Serial.print("connecting to ");
Serial.println(host);
	if (Client.connect(host, httpsPort)) 
  	{
    Serial.println("connection success");
  	if (Client.verify(fingerprint, host)) 
  		{
    	Serial.println("certificate matches");
    	return true;
   		} 
  	else 
   		{
    	Serial.println("certificate fail");
    	return false;
     	}
    }
	else
		{
		Serial.print("connection fail");	
		return false;
		}	
return false;		
}

//===========================
void setup()
//===========================
{
Serial.begin(115200);
pinMode(LDR, INPUT);
pinMode(BUTTON, INPUT);
pinMode(RED, OUTPUT);
pinMode(GREEN, OUTPUT);
pinMode(BLUE, OUTPUT);
connectWiFi();
light = "0";
while(!StartConnect())
	{
	}
}
//===========================
void loop()
//===========================
{
unsigned long currentMillis = millis();
if(WiFi.status() != WL_CONNECTED) 
	connectWiFi();
if(!Client.connected())	
	StartConnect();

if (currentMillis - RPCpreviousMillis >= RPCinterval) 
	{
  RPCpreviousMillis = currentMillis; 	
  getDataSourcesData(Client);
 }
   
  //LED show
  if (currentMillis - LEDpreviousMillis >= LEDinterval) {
    LEDpreviousMillis = currentMillis;
    Serial.print("LDR: ");
    Serial.println(analogRead(LDR));
    Serial.print("BUTTON: ");
    Serial.println(digitalRead(BUTTON));

    if ((light == "off") || (light == "0")) {
        analogWrite(RED, 0);
        analogWrite(GREEN, 0);
        analogWrite(BLUE, 0);
    } else if ((light == "on") || (light == "1")) {
        analogWrite(RED, 1023);
        analogWrite(GREEN, 1023);
        analogWrite(BLUE, 1023);
    } else if (light == "red") {
        analogWrite(RED, 1023);
        analogWrite(GREEN, 0);
        analogWrite(BLUE, 0);
    } else if (light == "blue") {
        analogWrite(RED, 0);
        analogWrite(GREEN, 0);
        analogWrite(BLUE, 1023);
    } else if (light == "green") {
        analogWrite(RED, 0);
        analogWrite(GREEN, 1023);
        analogWrite(BLUE, 0);
    } else {
        analogWrite(RED, 0);
        analogWrite(GREEN, 0);
        analogWrite(BLUE, 0);
    }
  }
}





