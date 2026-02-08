#include <WiFi.h>
//#include <WebServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "time.h"
#include "webpage.h" // Import the HTML file we just made

#define RXD2 7
#define TXD2 8

const char* ssid = "Maddy";
const char* password = "Mads2939hotspotyo";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;      // Adjust for your timezone (e.g., -21600 for CST)
const int   daylightOffset_sec = 3600;

String lastCheckedTime = "";

//WebServer server(80);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Variables to store schedule
struct PillSlot {
  String name = ""; 
  String t1 = "";
  String t2 = "";
  String t3 = "";
};
PillSlot slots[5];

struct HistoryItem { String name; String time; String type; };
HistoryItem historyLog[10]; // Store last 10 events
int historyCount = 0;       // How many we have stored so far

void sendToSTM32(String command) {
  Serial.println("Sent to STM32: " + command); // Show on Computer
  Serial2.println(command);                    // Send to STM32
}

void addHistory(String name, String type) {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) return;
  char timeBuff[10];
  strftime(timeBuff, sizeof(timeBuff), "%I:%M:%S", &timeinfo); // e.g. 08:00:05
  
  // Shift existing items to make room at the top (Index 0 is newest)
  for (int i = 9; i > 0; i--) {
      historyLog[i] = historyLog[i - 1];
  }
  
  // Add new item at top
  historyLog[0] = {name, String(timeBuff), type};
  if (historyCount < 10) historyCount++;
}

// Helper: Send History to Web Client
void sendHistory(AsyncWebSocketClient *client) {
  StaticJsonDocument<1024> doc;
  doc["type"] = "history_full";
  
  JsonArray data = doc.createNestedArray("data");
  for(int i=0; i<historyCount; i++) {
     JsonObject item = data.createNestedObject();
     item["name"] = historyLog[i].name;
     item["time"] = historyLog[i].time;
     item["type"] = historyLog[i].type;
  }
  
  String output;
  serializeJson(doc, output);
  client->text(output);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len, AsyncWebSocketClient *client) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    
    // 1. Convert received bytes to String
    data[len] = 0;
    String message = (char*)data;
    Serial.println("Received: " + message);

    // 2. Parse JSON
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) { Serial.print("JSON Error"); return; }

    String type = doc["type"];
    int slot = doc["slot"];

    // 3. Handle "GET" Request (User switched slot number)
    if (type == "get") {
       StaticJsonDocument<200> response;
       response["type"] = "slot_data";
       response["slot"] = slot;
       response["name"] = slots[slot].name;
       response["t1"] = slots[slot].t1;
       response["t2"] = slots[slot].t2;
       response["t3"] = slots[slot].t3;

       String output;
       serializeJson(response, output);
       ws.textAll(output); // Send back to website
    }

    // 4. Handle "SAVE" Request
    else if (type == "save") {
       slots[slot].name = doc["name"].as<String>();
       slots[slot].t1 = doc["t1"].as<String>();
       slots[slot].t2 = doc["t2"].as<String>();
       slots[slot].t3 = doc["t3"].as<String>();
       
       Serial.println("Saved Slot " + String(slot));
       sendToSTM32("SAVE:" + String(slot) + String(slots[slot].t1 + slots[slot].t2 + slots[slot].t3));
    }

    // 5. Handle "DELETE" Request
    else if (type == "delete") {
       slots[slot].name = "";
       slots[slot].t1 = "";
       slots[slot].t2 = "";
       slots[slot].t3 = "";
       Serial.println("Deleted Slot " + String(slot));

       sendToSTM32("DELETE:" + String(slot) + String(slots[slot].t1 + slots[slot].t2 + slots[slot].t3));
       
       // Force refresh on client
       StaticJsonDocument<200> response;
       response["type"] = "slot_data";
       response["name"] = "";
       response["t1"] = ""; response["t2"] = ""; response["t3"] = "";
       String output;
       serializeJson(response, output);
       ws.textAll(output);
    }
    else if (type == "log_taken") {
       addHistory(doc["name"].as<String>(), "Taken");
       // Broadcast update to everyone
       ws.textAll("{\"type\":\"refresh_history\"}"); 

       sendToSTM32("TAKEN:" + doc["name"].as<String>()); // prob is handled w/ load cell
    }
    // --- NEW: REQUEST HISTORY ---
    else if (type == "get_history") {
       sendHistory(client);
    }
  }
}


void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if(type == WS_EVT_DATA) handleWebSocketMessage(arg, data, len, client);
  if(type == WS_EVT_CONNECT) {
     // Send history immediately upon connection
     sendHistory(client);
  }
}


void setup() {
  Serial.begin(115200);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial2 Started for STM32");

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // <--- Type this IP into your browser!


  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Waiting for time...");
  struct tm timeinfo;
  while(!getLocalTime(&timeinfo)){ Serial.print("."); delay(500); }
  Serial.println("\nTime Synchronized!");

  // Define URLs
  // Setup WebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Serve Webpage
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", page_html);
  });

  server.begin();
}

void loop() {
  ws.cleanupClients();
  //server.handleClient(); // Listen for incoming web requests
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)){
    
    // Create a string like "08:30"
    char timeStringBuff[6];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M", &timeinfo);
    String currentTime = String(timeStringBuff);
    
    // Only check once per minute (when the minute changes)
    if(currentTime != lastCheckedTime) {
       lastCheckedTime = currentTime;
       Serial.println("Checking Schedule for: " + currentTime);

       // Check all 4 slots
       for(int i=1; i<=4; i++) {
         if(slots[i].name != "") {
            // Check if ANY of the 3 times match
            if(slots[i].t1 == currentTime || slots[i].t2 == currentTime || slots[i].t3 == currentTime) {
                // 1. Log to History
                addHistory(slots[i].name, "Dispensed");
                
                // 2. Alert Webpage
                StaticJsonDocument<200> alertDoc;
                alertDoc["type"] = "alert";
                alertDoc["msg"] = slots[i].name; 
                String output; serializeJson(alertDoc, output);
                ws.textAll(output);
                
                // 3. Update History List on Webpage
                ws.textAll("{\"type\":\"refresh_history\"}"); 

                sendToSTM32("DISPENSE:" + String(i)); // optional bc STM has RTC
            }
         }
       }
    }
  }
  // --- TEST CODE END ---
  // Add your Pill Dispenser Logic / Time check here...
}
