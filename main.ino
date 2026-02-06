#include <WiFi.h>
#include <WebServer.h>
#include "webpage.h" // Import the HTML file we just made

const char* ssid = "mason iphone 13";
const char* password = "7paf0osloro0";

WebServer server(80);

// Variables to store schedule
String pillName = "";
String pillTime = "";
int pillSlot = 1;

// 1. Serve the HTML Page
void handleRoot() {
  server.send(200, "text/html", page_html);
}

// 2. Handle the "Set Schedule" Button
void handleSet() {
  // Check if arguments exist (slot, name, time)
  if (server.hasArg("slot") && server.hasArg("name") && server.hasArg("time")) {
    
    pillSlot = server.arg("slot").toInt();
    pillName = server.arg("name");
    pillTime = server.arg("time");
    
    Serial.println("New Schedule Received:");
    Serial.print("Slot: "); Serial.println(pillSlot);
    Serial.print("Name: "); Serial.println(pillName);
    Serial.print("Time: "); Serial.println(pillTime);
    
    // Save to variables or Struct here...
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing Arguments");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // <--- Type this IP into your browser!

  // Define URLs
  server.on("/", handleRoot);      // Load the webpage
  server.on("/set", handleSet);    // Catch the button click
  
  server.begin();
}

void loop() {
  server.handleClient(); // Listen for incoming web requests
  
  // Add your Pill Dispenser Logic / Time check here...
}