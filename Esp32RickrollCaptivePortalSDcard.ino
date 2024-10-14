#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <SD.h> // Include SD card library
#include <SPI.h> // Include SPI library for SD card

DNSServer dnsServer;
AsyncWebServer server(80);

const int sd_cs_pin = 5; // GPIO 5 (adjust if necessary)
File logFile; // File object for logging

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Free Wi-Fi Agreement</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            background-color: #f4f4f4;
            font-family: Arial, sans-serif;
        }
        .container {
            text-align: center;
        }
        .agreement {
            background: white;
            border: 1px solid #ccc;
            border-radius: 8px;
            padding: 20px;
            max-width: 600px;
            margin: 0 auto;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }
        .agreement h1 {
            font-size: 24px;
            margin-bottom: 20px;
        }
        .agreement p {
            font-size: 16px;
            margin-bottom: 20px;
        }
        .agreement a {
            color: #008CBA;
            text-decoration: none;
        }
        .agreement button {
            background-color: #008CBA;
            border: none;
            color: white;
            padding: 15px 32px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            margin: 4px 2px;
            cursor: pointer;
            border-radius: 4px;
        }
        video {
            width: 100vw;
            height: 100vh;
            object-fit: cover;
            display: none; /* Hide the video initially */
        }
    </style>
</head>
<body>
    <div class="container">
        <div id="agreement" class="agreement">
            <h1>Free Wi-Fi Access</h1>
            <p>You must agree to the <a href="#">Terms of Service</a> to access this free Wi-Fi.</p>
            <p>By clicking "Accept", you agree to abide by the terms stated.</p>
            <button id="acceptButton">Accept</button>
        </div>
        <video id="videoPlayer" controls>
            <source src="/video" type="video/mp4">
            Your browser does not support the video tag.
        </video>
    </div>
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            var acceptButton = document.getElementById('acceptButton');
            var agreement = document.getElementById('agreement');
            var video = document.getElementById('videoPlayer');
            
            acceptButton.addEventListener('click', function() {
                agreement.style.display = 'none'; // Hide the agreement page
                video.style.display = 'block'; // Show the video
                video.play(); // Start playing the video
            });
        });
    </script>
</body>
</html>
)rawliteral";

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request) {
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html); 
  }
};

void logMessage(String message) {
  Serial.println(message); // Print to Serial
  if (logFile) {
    logFile.println(message); // Log to SD card
    logFile.flush(); // Ensure the data is written
  }
}

void setupServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html); 
    logMessage("Client Connected");
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
  
    if (request->hasParam("name")) {
      inputMessage = request->getParam("name")->value();
      inputParam = "name";
      logMessage("Name: " + inputMessage);
    }

    if (request->hasParam("proficiency")) {
      inputMessage = request->getParam("proficiency")->value();
      inputParam = "proficiency";
      logMessage("Proficiency: " + inputMessage);
    }
    request->send(200, "text/html", "The values entered by you have been successfully sent to the device <br><a href=\"/\">Return to Home Page</a>");
  });

  // Serve the video file with streaming
  server.on("/video", HTTP_GET, [](AsyncWebServerRequest *request) {
    logMessage("Another victim rickrolled :)");
    if (SD.exists("/your-video-file.mp4")) {
      File videoFile = SD.open("/your-video-file.mp4");
      if (videoFile) {
        request->send(SD, "/your-video-file.mp4", "video/mp4");
        videoFile.close();
      } else {
        logMessage("Failed to open video file");
        request->send(500, "text/plain", "Failed to open video file");
      }
    } else {
      logMessage("Video file not found");
      request->send(404, "text/plain", "Video file not found");
    }
  });
}

void setup() {
  Serial.begin(115200);

  // Initialize SD card
  if (!SD.begin(sd_cs_pin)) {
    logMessage("SD card initialization failed");
    return;
  }
  logMessage("SD card initialized");

  // Open log file
  logFile = SD.open("/log.txt", FILE_APPEND);
  if (!logFile) {
    logMessage("Failed to open log file");
    return;
  }
  
  logMessage("Listing files on SD card:");
  File root = SD.open("/");
  File file = root.openNextFile();
  while (file) {
    logMessage("FILE: " + String(file.name()));
    file = root.openNextFile();
  }

  logMessage("Setting up AP Mode");
  WiFi.mode(WIFI_AP); 
  WiFi.softAP("Free Wifi");
  logMessage("AP IP address: " + WiFi.softAPIP().toString());
  logMessage("Setting up Async WebServer");
  setupServer();
  logMessage("Starting DNS Server");
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // Only when requested from AP
  server.begin();
  logMessage("All Done!");
}

void loop() {
  dnsServer.processNextRequest();
}
