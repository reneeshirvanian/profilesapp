const char page_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; text-align: center; background: #f4f4f4; margin: 0; padding-bottom: 50px; }
    
    /* Main Card Style */
    .card { background: white; padding: 20px; display: inline-block; border-radius: 8px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); max-width: 400px; width: 90%; margin-top: 20px; }
    
    /* Input & Button Styles */
    input, button { padding: 10px; margin: 5px 0; width: 90%; box-sizing: border-box; }
    button { cursor: pointer; border: none; border-radius: 4px; color: white; font-weight: bold; font-size: 16px; }
    .btn-save { background-color: #28a745; }
    .btn-del { background-color: #dc3545; display: none; } 
    label { font-weight: bold; display: block; margin-top: 10px; text-align: left; margin-left: 5%; }

    /* WebSocket Status */
    #ws-status { font-size: 12px; color: red; margin-bottom: 10px; font-weight: bold; }

    /* --- ALERT POPUP STYLES --- */
    #alertOverlay {
      display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%;
      background: rgba(0,0,0,0.8); z-index: 1000;
    }
    .alert-box {
      background: white; width: 80%; max-width: 300px;
      margin: 100px auto; padding: 20px; border-radius: 10px; border: 4px solid #ff9800;
      text-align: center;
    }
    .alert-title { color: #d35400; font-size: 24px; margin: 0; }
    .btn-snooze { background-color: #f39c12; color: white; margin-bottom: 10px; }
    .btn-take { background-color: #27ae60; color: white; height: 50px; font-size: 18px; }

    /* --- HISTORY LIST STYLES --- */
    #historySection { margin-top: 30px; text-align: left; padding: 0 20px; }
    #historyList { list-style: none; padding: 0; }
    .status-dispensed { color: #d35400; font-weight: bold; }
    .status-taken { color: #27ae60; font-weight: bold; }
  
    .history-item { 
        background: white; border-bottom: 1px solid #ddd; 
        padding: 10px; margin-bottom: 5px; border-radius: 4px;
        font-size: 14px; display: flex; justify-content: space-between; align-items: center;
    }
    .history-details { display: flex; flex-direction: column; text-align: right; }
    .history-time { font-size: 11px; color: #999; }
  </style>
</head>
<body>

<div class="card">
  <h2>Pill Manager</h2>
  <div id="ws-status">Disconnected</div>

  <label>Dispenser Slot (1-4):</label>
  <input type="number" id="slot" min="1" max="4" value="1" oninput="requestSlot()">

  <div id="formArea">
    <label>Pill Name:</label>
    <input type="text" id="name" placeholder="Empty Slot">

    <label>Schedule 1:</label>
    <input type="time" id="t1">
    
    <label>Schedule 2:</label>
    <input type="time" id="t2">

    <label>Schedule 3:</label>
    <input type="time" id="t3">

    <button class="btn-save" onclick="sendSave()">SAVE / UPDATE</button>
    <button id="delBtn" class="btn-del" onclick="sendDelete()">DELETE PILL</button>
  </div>
</div>

<div class="card" style="margin-top: 20px;">
  <h3>Adherence History</h3>
  <ul id="historyList">
    <li style="color:#aaa; text-align:center;">No pills taken yet</li>
  </ul>
</div>

<div id="alertOverlay">
  <div class="alert-box">
    <h1 class="alert-title">IT'S TIME!</h1>
    <p id="alertMessage">Take your meds</p>
    
    <button class="btn-snooze" onclick="snoozeAlert()">SNOOZE</button>
    <button class="btn-take" onclick="confirmTaken()">TAKEN</button>
  </div>
</div>

<script>
  // --- WEBSOCKET SETUP ---
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  var historyLog = []; 
  var activePills = []; // Stores names of pills currently ringing

  window.addEventListener('load', onLoad);

  function onLoad(event) { initWebSocket(); }

  function initWebSocket() {
    console.log('Opening WebSocket...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
  }

  function onOpen(event) {
    document.getElementById('ws-status').innerHTML = "Connected";
    document.getElementById('ws-status').style.color = "green";
    requestSlot(); // Load Slot 1 data immediately
  }

  function onClose(event) {
    document.getElementById('ws-status').innerHTML = "Disconnected";
    document.getElementById('ws-status').style.color = "red";
    setTimeout(initWebSocket, 2000); // Retry every 2s
  }

  // --- HANDLING MESSAGES FROM ESP32 ---
  function onMessage(event) {
    console.log("RX: " + event.data);
    var data = JSON.parse(event.data);

    // 1. DATA LOAD (Populate inputs)
    if (data.type === "slot_data") {
        document.getElementById("name").value = data.name;
        document.getElementById("t1").value = data.t1;
        document.getElementById("t2").value = data.t2;
        document.getElementById("t3").value = data.t3;

        // UI Locking Logic
        var isLocked = (data.name !== "");
        document.getElementById("delBtn").style.display = isLocked ? "inline-block" : "none";
        document.getElementById("name").disabled = isLocked;
        document.getElementById("name").style.backgroundColor = isLocked ? "#e9ecef" : "white";
    }

    // 2. ALERT TRIGGER (ESP32 says "Dispense Now!")
    else if (data.type === "alert") {
        // data.msg contains the pill name, e.g. "Advil"
        handleIncomingAlert(data.msg);
    }
    else if (data.type === "refresh_history") {
        // ESP32 says: "History changed, please ask for the new list"
        websocket.send(JSON.stringify({type: "get_history"}));
    }

    // --- NEW: RECEIVE FULL HISTORY LIST ---
    else if (data.type === "history_full") {
        renderHistory(data.data);
    }
  }

  // --- SENDING COMMANDS TO ESP32 ---
  
  function requestSlot() {
    var slot = document.getElementById("slot").value;
    // Debounce or basic check could go here
    websocket.send(JSON.stringify({ type: "get", slot: parseInt(slot) }));
  }

  function sendSave() {
    var slot = parseInt(document.getElementById("slot").value);
    var name = document.getElementById("name").value;
    if(name === "") { alert("Enter Name"); return; }

    var msg = {
        type: "save",
        slot: slot,
        name: name,
        t1: document.getElementById("t1").value,
        t2: document.getElementById("t2").value,
        t3: document.getElementById("t3").value
    };
    websocket.send(JSON.stringify(msg));
  }

  function sendDelete() {
    if(!confirm("Delete this pill?")) return;
    var msg = {
        type: "delete",
        slot: parseInt(document.getElementById("slot").value)
    };
    websocket.send(JSON.stringify(msg));
  }

  // --- ALERT & HISTORY LOGIC ---

  function handleIncomingAlert(pillName) {
    // 1. Add to active list
    if(!activePills.includes(pillName)) {
        activePills.push(pillName);
        
        //addToHistory(pillName, "Dispensed");
    }
    updateAlertBox();
  }

  function updateAlertBox() {
    if (activePills.length === 0) return;
    var msg = "Time to take:<br><b>" + activePills.join(" & ") + "</b>";
    document.getElementById("alertMessage").innerHTML = msg;
    document.getElementById("alertOverlay").style.display = "block";
  }

  function snoozeAlert() {
    document.getElementById("alertOverlay").style.display = "none";
    
    setTimeout(function() {
      
      // 3. After time is up, check if pills are still pending
      if(activePills.length > 0) {
        // Show the alert again
        updateAlertBox();
      }
      
    }, 5000);
  }

  function confirmTaken() {
    document.getElementById("alertOverlay").style.display = "none";
    
    // Log "Taken" for all active pills
    activePills.forEach(function(name) {
      websocket.send(JSON.stringify({ type: "log_taken", name: name }));
    });
    
    activePills = []; // Clear list
  }

  function renderHistory(historyArray) {
    var listHTML = "";
    // Loop through data sent by ESP32
    for (var i = 0; i < historyArray.length; i++) {
      var item = historyArray[i];
      var cssClass = (item.type === "Dispensed") ? "status-dispensed" : "status-taken";

      listHTML += `<li class="history-item">
                     <span><b>${item.name}</b></span>
                     <div class="history-details">
                       <span class="${cssClass}">${item.type}</span>
                       <span class="history-time">${item.time}</span>
                     </div>
                   </li>`;
    }
    
    if(historyArray.length === 0) {
        listHTML = `<li style="color:#aaa; text-align:center;">No pills taken yet</li>`;
    }
    
    document.getElementById("historyList").innerHTML = listHTML;
  }

</script>
</body>
</html>
)=====";
