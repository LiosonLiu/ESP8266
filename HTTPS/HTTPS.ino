/*
    HTTP over TLS (HTTPS) example sketch

    This example demonstrates how to use
    WiFiClientSecure class to access HTTPS API.
    We fetch and display the status of
    esp8266/Arduino project continous integration
    build.

    Created by Ivan Grokhotkov, 2015.
    This example is in public domain.
*/
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
//Port
const int LDR 		= A0;
const int BUTTON 	= 4;
const int LED_RED 	= 15;
const int LED_GREEN = 12;
const int LED_BLUE 	= 13;
//Value
const char* ssid 	= "ExoMobile";
const char* password = "Colfax227";

const char* host 	= "m2.exosite.com";
const int httpsPort = 443;
const char* cik  	= "78a23b8c723d5dc5942bd8ce9c2a2b71412de9f9";
const char* aliasA 	= "led_data";
const char* aliasB 	= "light_sensor";
const char* aliasC 	= "button";
const char* fingerprint = "1abbf683b33fb3e4ab4dd829e26608884e61e61d";

String LedData;
const long RPCinterval = 1200;//1.2sec
unsigned long RPCpreviousTimeStamp;
unsigned char ButtonData,LightSensor;
//=================
void initHardware(void)
//=================
{
  Serial.begin(115200);
  Serial.println();
  pinMode(LDR, INPUT);
  pinMode(BUTTON, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);    
  digitalWrite(LED_BLUE, LOW);
}

//=================
void connectWiFi(void)
//=================
{
  byte ledStatus = LOW;
  WiFi.mode(WIFI_AP_STA);
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  	{
    // Blink the LED
    digitalWrite(LED_BLUE, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    Serial.print(".");
    delay(500);
  	}
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BLUE, HIGH);
}


//=================
void getRequestJson(JsonObject& root) 
//=================
//Code checked via http://www.jsoneditoronline.org/
{
  JsonObject& auth = root.createNestedObject("auth");			//"auth":{"cik": "e469e336ff9c8ed9176bc05ed7fa40daaaaaaaaa"}
  auth["cik"] = cik;											//
  JsonArray& calls = root.createNestedArray("calls");			//"calls": [
  JsonObject& call1 = calls.createNestedObject();				//			{
  call1["id"] = 0; 												//			"id": 0,
  call1["procedure"] = "read";									//			"procedure": "read",
  JsonArray& arguments1 = call1.createNestedArray("arguments");	//			"arguments": [
  JsonObject& aliasname1 = arguments1.createNestedObject();		//
  aliasname1["alias"] = aliasA;								//						"alias":"led_data",
  JsonObject& sortlmit = arguments1.createNestedObject();		//							{
  sortlmit["sort"] = "desc";									//							"sort":"desc",
  sortlmit["limit"] = 1;										//							"limit":1			
																//							}]		
  JsonObject& call2 = calls.createNestedObject();				//			{
  call2["id"] = 1; 												//			"id": 1,
  call2["procedure"] = "write";									//			"procedure": "write",
  JsonArray& arguments2 = call2.createNestedArray("arguments");	//			"arguments": [
  JsonObject& aliasname2 = arguments2.createNestedObject();		//						 	{"alias":"light_sensor",
  aliasname2["alias"] = aliasB;									//						 	 LDR}]}
  arguments2.add(LightSensor);
  
  
  JsonObject& call3 = calls.createNestedObject();				//			{
  call3["id"] = 2; 												//			"id": 2,
  call3["procedure"] = "write";									//			"procedure": "write",
  JsonArray& arguments3 = call3.createNestedArray("arguments");	//			"arguments": [  
  JsonObject& aliasname3 = arguments3.createNestedObject();		//						 	{"alias":"button",
  aliasname3["alias"] = aliasC;									//						 	 Button}]}
  arguments3.add(ButtonData);																						
}				
																
//=================
void ReadDataFromCloud(WiFiClientSecure& client)
//=================
{
  String url = "/onep:v1/rpc/process";	
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();					//{
  getRequestJson(root);
  String requestJson;
  root.printTo(requestJson);

  Serial.print("JSON Data	: ");
  root.printTo(Serial);
  Serial.print("\r\n");
  Serial.print("Requesting URL	: ");
  Serial.println(url);
  Serial.print("HTTP Request......\r\n");
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-type: application/json; charset=utf-8\r\n" +
               "Content-Length: " + requestJson.length() + "\r\n" +
               "Connection: close\r\n\r\n" + requestJson
              );

  while (client.available()) 
  	{
    String line = client.readStringUntil('\r');
    Serial.print(line);
  	}
  while (client.connected()) 
  	{
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("[{\"id\":0,\"status\":\"ok\"")) 
  	{
    Serial.print("RPC call successfull!\r\n");
  	} 
  else 
  	{
    Serial.print("RPC call failed\r\n");
    return;
  	}

  Serial.print("Reply info:");
  Serial.println(line);
  int str_len = line.length() + 1;
  StaticJsonBuffer<1200> jsonBuffer2;
//  Serial.println(line.substring(1, str_len - 2));
  JsonObject& root2 = jsonBuffer2.parseObject(line.substring(1, str_len - 2));
  if (!root2.success())
  {
    Serial.println("parseObject() failed");
    return;
  }
  if (root2["result"][0].is<JsonArray&>()) 
  	{
    String mesg1 = root2["result"][0][1];
    LedData = mesg1;
    Serial.print("LedData=");
    Serial.println(LedData);    
  	}
}

//=================
int	ASCIItoHex(int temp1,int temp2)
//=================
{
if(temp1<58)	
	{
	if(temp1>47)	temp1=temp1-48;
	else			temp1=0;
	}
else if(temp1<71)
		{
		if(temp1>64)	temp1=temp1-55;
		else			temp1=0;
		}
else if(temp1<103)
		{	
		if(temp1>96)	temp1=temp1-87;
		else			temp1=0;
		}
else	
		temp1=0;	
	
if(temp2<58)	
	{
	if(temp2>47)	temp2=temp2-48;
	else			temp2=0;
	}
else if(temp2<71)
		{
		if(temp2>64)	temp2=temp2-55;
		else			temp2=0;
		}
else if(temp2<103)
		{	
		if(temp2>96)	temp2=temp2-87;
		else			temp2=0;
		}
else	
		temp2=0;
	
				
return (temp1<<4)+temp2;
}

//===========================
void setup() 
//===========================
{
  initHardware();
  connectWiFi();
}


//===========================
void loop() 
//===========================
{
int red,green,blue;
unsigned long CurrentTimeStamp = millis();			
WiFiClientSecure client;
  
if (WiFi.status() != WL_CONNECTED) 
	connectWiFi();
  
if (CurrentTimeStamp - RPCpreviousTimeStamp >= RPCinterval) 
	{
	Serial.print("=============================Connection START=============================\r\n");
    RPCpreviousTimeStamp = CurrentTimeStamp;    
    Serial.print("connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) 
    	{
      	Serial.println("connection failed \r\n");
      	return;
    	}

 	if (client.verify(fingerprint, host)) 
    	Serial.println("certificate matches \r\n");
	else 
   		{
    	Serial.println("certificate doesn't match \r\n");
    	return;
    	}
	ReadDataFromCloud(client);
	
	red 	= ASCIItoHex(LedData[0],LedData[1]);
	green	= ASCIItoHex(LedData[2],LedData[3]);
	blue 	= ASCIItoHex(LedData[4],LedData[5]);
	}
analogWrite(LED_RED, random(0,red<<2));
analogWrite(LED_GREEN, random(0,green<<2));
analogWrite(LED_BLUE, random(0,blue<<2));
LightSensor=0xff-analogRead(LDR);
ButtonData=digitalRead(BUTTON);
}


/*


*/