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

    /* --- NEW: ALERT POPUP STYLES --- */
    #alertOverlay {
      display: none; /* Hidden by default */
      position: fixed; top: 0; left: 0; width: 100%; height: 100%;
      background: rgba(0,0,0,0.8); /* Dark transparent background */
      z-index: 1000;
    }
    .alert-box {
      background: white; width: 80%; max-width: 300px;
      margin: 100px auto; padding: 20px; border-radius: 10px; border: 4px solid #ff9800;
      text-align: center;
    }
    .alert-title { color: #d35400; font-size: 24px; margin: 0; }
    .btn-snooze { background-color: #f39c12; color: white; margin-bottom: 10px; }
    .btn-take { background-color: #27ae60; color: white; height: 50px; font-size: 18px; }

    /* --- NEW: HISTORY LIST STYLES --- */
    #historySection { margin-top: 30px; text-align: left; padding: 0 20px; }
    #historyList { list-style: none; padding: 0; }
    .status-dispensed { color: #d35400; font-weight: bold; } /* Orange for Machine */
    .status-taken { color: #27ae60; font-weight: bold; }     /* Green for User */
  
    .history-item { 
        background: white; border-bottom: 1px solid #ddd; 
        padding: 10px; margin-bottom: 5px; border-radius: 4px;
        font-size: 14px; display: flex; justify-content: space-between; align-items: center;
    }
    .history-details { display: flex; flex-direction: column; text-align: right; }
    .history-time { font-size: 11px; color: #999; }
</style>
  </style>
</head>
<body>



<div class="card">
  <h2>Pill Manager</h2>

  <label>Dispenser Slot (1-4):</label>
  <input type="number" id="slot" min="1" max="4" value="1" oninput="loadSlot()">

  <div id="formArea">
    <label>Pill Name:</label>
    <input type="text" id="name" placeholder="Empty Slot">

    <label>Schedule 1:</label>
    <input type="time" id="t1">
    
    <label>Schedule 2:</label>
    <input type="time" id="t2">

    <label>Schedule 3:</label>
    <input type="time" id="t3">

    <button class="btn-save" onclick="sendData()">SAVE / UPDATE</button>
    <button id="delBtn" class="btn-del" onclick="deleteSlot()">DELETE PILL</button>
  </div>
  
  <p id="status" style="color: grey;">Ready</p>
</div>

<div class="card" style="margin-top: 20px;">
  <h3>Adherence History</h3>
  <ul id="historyList">
    <li style="color:#aaa; text-align:center;">No pills taken yet</li>
  </ul>
</div>

<div id="alertOverlay">
  <div class="alert-box">
    <h1 class="alert-title">Alert</h1>
    <p id="alertMessage">Take your meds</p>
    
    <button class="btn-snooze" onclick="snoozeAlert()">SNOOZE</button>
    
    <button class="btn-take" onclick="confirmTaken()">TAKEN</button>
  </div>
</div>

<script>
  // --- SETTINGS ---
  var TEST_MODE = true; 

  // --- FAKE DATABASE ---
  var fakeDB = {
    1: { name: "", t1: "", t2: "", t3: "" },
    2: { name: "", t1: "", t2: "", t3: "" },
    3: { name: "", t1: "", t2: "", t3: "" },
    4: { name: "", t1: "", t2: "", t3: "" }
  };

  var historyLog = []; 
  var activePills = []; 

  // --- 1. LOAD & SAVE DATA (Standard Stuff) ---
  function loadSlot() {
    var slot = parseInt(document.getElementById("slot").value);
    if (isNaN(slot) || slot < 1 || slot > 4) return;

    if (TEST_MODE) {
      setTimeout(() => updateUI(fakeDB[slot]), 200);
    } else {
      fetch("/get?slot=" + slot).then(r => r.json()).then(d => updateUI(d));
    }
  }

  function updateUI(data) {
    if (!data) return;
    document.getElementById("name").value = data.name;
    document.getElementById("t1").value = data.t1;
    document.getElementById("t2").value = data.t2;
    document.getElementById("t3").value = data.t3;
    
    // Toggle Locking
    var isLocked = (data.name !== "");
    document.getElementById("delBtn").style.display = isLocked ? "inline-block" : "none";
    document.getElementById("name").disabled = isLocked;
    document.getElementById("name").style.backgroundColor = isLocked ? "#e9ecef" : "white";
  }

  function sendData() {
    var slot = parseInt(document.getElementById("slot").value);
    var name = document.getElementById("name").value;
    var t1 = document.getElementById("t1").value;
    var t2 = document.getElementById("t2").value;
    var t3 = document.getElementById("t3").value;

    if(name === "") { alert("Enter Name"); return; }

    if (TEST_MODE) {
       fakeDB[slot] = { name: name, t1: t1, t2: t2, t3: t3 };
       loadSlot();
       document.getElementById("status").innerHTML = "Saved";
    } else {
       var url = `/set?slot=${slot}&name=${name}&t1=${t1}&t2=${t2}&t3=${t3}`;
       fetch(url).then(() => { loadSlot(); document.getElementById("status").innerHTML = "Saved"; });
    }
  }

  function deleteSlot() {
    if(!confirm("Delete?")) return;
    var slot = parseInt(document.getElementById("slot").value);
    if(TEST_MODE) { fakeDB[slot] = {name:"",t1:"",t2:"",t3:""}; loadSlot(); }
    else { fetch("/delete?slot="+slot).then(() => loadSlot()); }
  }

  // ============================================
  // === UPDATED LOGIC FOR DISPENSED VS TAKEN ===
  // ============================================

 
  function triggerAlert(slotID) {
    var pillName = fakeDB[slotID].name;
    currentAlertSlot = slotID; 
    document.getElementById("alertMessage").innerHTML = pillName;
    document.getElementById("alertOverlay").style.display = "block";
  }

  
  function snoozeAlert() {
    document.getElementById("alertOverlay").style.display = "none"; 
    
    // Set a timer to bring the SAME list back in 5 seconds (testing)
    setTimeout(function() {
      // Only show if there are still pills pending
      if(activePills.length > 0) {
        updateAlertBox();
      }
    }, 300000); 
  }

  
  function confirmTaken() {
    document.getElementById("alertOverlay").style.display = "none"; 
    
    activePills.forEach(function(id) {
      addToHistory(id, "Taken");
    });

    
    activePills = [];
  }

  
  function addToHistory(slotID, actionType) {
    var pillName = fakeDB[slotID].name;
    var now = new Date();
    var timeString = now.toLocaleTimeString(); 

    // Add to Array
    historyLog.push({ name: pillName, time: timeString, type: actionType });

    // Render List
    var listHTML = "";
    for (var i = historyLog.length - 1; i >= 0; i--) {
      
      var cssClass = (historyLog[i].type === "Dispensed") ? "status-dispensed" : "status-taken";
      
      listHTML += `<li class="history-item">
                     <span><b>${historyLog[i].name}</b></span>
                     <div class="history-details">
                       <span class="${cssClass}">${historyLog[i].type}</span>
                       <span class="history-time">${historyLog[i].time}</span>
                     </div>
                   </li>`;
    }
    document.getElementById("historyList").innerHTML = listHTML;
  }

  
  setInterval(function() {
    var now = new Date();
    var currentHM = now.toTimeString().substring(0,5); // "08:00"
    var currentSec = now.getSeconds(); 

    if (currentSec !== 0) return; // Only run at :00 seconds

    var newPillsAdded = false;

    // Check every slot in the database
    for(var slot in fakeDB) {
      var p = fakeDB[slot];
      if(p.name !== "") {
        // If time matches...
        if(p.t1 === currentHM || p.t2 === currentHM || p.t3 === currentHM) {
          
          // Check if we already know about this pill (Prevent Duplicates)
          // We force 'slot' to be a number using parseInt
          var slotID = parseInt(slot);
          
          if(!activePills.includes(slotID)) {
             console.log("ALARM: " + p.name);
             
             // 1. Add to our active list
             activePills.push(slotID);
             
             // 2. Log "Dispensed" immediately
             addToHistory(slotID, "Dispensed");
             
             newPillsAdded = true;
          }
        }
      }
    }

    // If we found ANY new pills, update the screen immediately
    if (newPillsAdded) {
      updateAlertBox();
    }

  }, 1000);

  function updateAlertBox() {
    if (activePills.length === 0) return; // Nothing to show

    // 1. Build a combined string: "Advil & Tylenol"
    var names = activePills.map(function(id) {
      return fakeDB[id].name;
    });
    
    var msg = names.join(" & ");

    // 2. Show the box
    document.getElementById("alertMessage").innerHTML = msg;
    document.getElementById("alertOverlay").style.display = "block";
  }

  window.onload = loadSlot;
</script>

</body>
</html>
)=====";