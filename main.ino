#include <WiFi.h>
//#include <WebServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "time.h"
#include "webpage.h"

#define RXD2 7
#define TXD2 8

const char* ssid = "Maddy";
const char* password = "Mads2939hotspotyo";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

// --- TELEGRAM CONFIG ---
// user is @MaddyPill_bot, name is MaddyPillBot
const char* botToken = "8516743744:AAFUVvnr7LvQV9efkYH3mAgJU1jbDVmibCo"; 
const char* chatID   = "8532923835";

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
PillSlot slots[4];

struct HistoryItem { String name; String time; String type; };
HistoryItem historyLog[10]; // Store last 10 events
int historyCount = 0;       // How many we have stored so far

void sendToSTM32(String command) {
  Serial.println("Sent to STM32: " + command); // Show on Computer
  Serial2.println(command + "\n");                    // Send to STM32
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
       
       Serial.println("Saved Slot " + String(slot) + String(slots[slot].t1 + slots[slot].t2 + slots[slot].t3));
       sendToSTM32("SCHED:" + String(slot) + ":" + String(slots[slot].t1) + ":1");
       delay(50); 
       sendToSTM32("SCHED:" + String(slot) + ":" + String(slots[slot].t2) + ":1");
       delay(50); 
       sendToSTM32("SCHED:" + String(slot) + ":" + String(slots[slot].t3) + ":1");
       delay(50); 

       StaticJsonDocument<200> response;
       response["type"] = "slot_data";
       response["slot"] = slot;
       response["name"] = slots[slot].name;
       response["t1"] = slots[slot].t1;
       response["t2"] = slots[slot].t2;
       response["t3"] = slots[slot].t3;

       String output;
       serializeJson(response, output);
       ws.textAll(output);
       
    }

    // 5. Handle "DELETE" Request
    else if (type == "delete") {
       slots[slot].name = "";
       slots[slot].t1 = "";
       slots[slot].t2 = "";
       slots[slot].t3 = "";
       Serial.println("Deleted Slot " + String(slot));

       sendToSTM32("DELETE:" + String(slot));
       delay(50);
      
       
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
    else if (type == "snooze_alert") {
       String names = doc["names"].as<String>();
       
       // Create the message and encode spaces for the URL
       String telegramMsg = "Snooze over! Time to take: " + names;
       telegramMsg.replace(" ", "+"); 
       
       sendTelegramMessage(telegramMsg);
       Serial.println("Sent snooze text for: " + names);
    }
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

  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial2 Started for STM32");

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // <--- Type this IP into your browser!

  sendTelegramMessage("Pill+Dispenser+Online");

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

void sendTelegramMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Skip certificate validation (easiest for hobby projects)
    
    HTTPClient https;
    
    // Construct the URL
    String url = "https://api.telegram.org/bot" + String(botToken) + 
                 "/sendMessage?chat_id=" + String(chatID) + 
                 "&text=" + message;
                 
    // Initialize connection
    if (https.begin(client, url)) {
      int httpCode = https.GET();
      
      if (httpCode > 0) {
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    } else {
      Serial.println("[HTTPS] Unable to connect");
    }
  } else {
    Serial.println("Error: WiFi not connected");
  }
}


void loop() {
  ws.cleanupClients();
  //server.handleClient(); // Listen for incoming web requests
  if (Serial2.available()) {
    String msg = Serial2.readStringUntil('\n');
    msg.trim(); // Clean up hidden characters like \r
    Serial.println("RX from STM32: " + msg);

    // --- A. LOAD CELL DETECTS PILL DROPPED ---
    // Expected format from STM32: "DROPPED:0" (or 1, 2, 3)
    if (msg.startsWith("DROPPED:")) {
       int slotNum = msg.substring(8).toInt();
       if (slotNum >= 0 && slotNum <= 3 && slots[slotNum].name != "") {
           String pillName = slots[slotNum].name;
           
           // 1. Log to History
           addHistory(pillName, "Dispensed");
           
           // 2. Alert Webpage
           StaticJsonDocument<200> alertDoc;
           alertDoc["type"] = "alert";
           alertDoc["msg"] = pillName; 
           String output; serializeJson(alertDoc, output);
           ws.textAll(output);

           // 3. Send Telegram Alert
           String telegramMsg = "Time to take: " + pillName;
           telegramMsg.replace(" ", "+"); 
           sendTelegramMessage(telegramMsg);
           
           // 4. Update Web History UI
           ws.textAll("{\"type\":\"refresh_history\"}"); 
       }
    }
    else if (msg.startsWith("TAKEN:")) {
       int slotNum = msg.substring(6).toInt();
       if (slotNum >= 0 && slotNum <= 3 && slots[slotNum].name != "") {
           String pillName = slots[slotNum].name;
           
           // Log as taken and refresh web history
           addHistory(pillName, "Taken");
           ws.textAll("{\"type\":\"refresh_history\"}"); 
           
           // Tell the webpage to turn off the alert popup
           ws.textAll("{\"type\":\"close_alert\"}");
       }
    }
    else if (msg == "GET_TIME") {
      // Send initialization time to STM32
      struct tm timeinfo;
      while(!getLocalTime(&timeinfo)){ Serial.print("."); delay(500); }
      uint8_t timeData[3];
      timeData[0] = (uint8_t)timeinfo.tm_hour;
      timeData[1] = (uint8_t)timeinfo.tm_min;
      timeData[2] = (uint8_t)timeinfo.tm_sec;
      
      delay(50);
      Serial2.write(timeData, 3);
      delay(50);
    }
  }
  
  // --- TEST CODE END ---
  // Add your Pill Dispenser Logic / Time check here...
}
