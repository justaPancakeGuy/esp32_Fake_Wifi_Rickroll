#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"  // Include SPIFFS library

DNSServer dnsServer;
AsyncWebServer server(80);

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
        audio {
            width: 100%;
            margin-top: 20px;
            display: none; /* Hide the audio initially */
        }
        .rickroll-text {
            display: none;
            font-size: 48px;
            font-weight: bold;
            color: red;
            text-align: center;
            margin-top: 20px;
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
        <audio id="audioPlayer" controls>
            <source src="/audio" type="audio/mpeg">
            Your browser does not support the audio tag.
        </audio>
        <div id="rickrollText" class="rickroll-text">You just got rickrolled!</div>
    </div>
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            var acceptButton = document.getElementById('acceptButton');
            var agreement = document.getElementById('agreement');
            var audio = document.getElementById('audioPlayer');
            var rickrollText = document.getElementById('rickrollText');
            
            acceptButton.addEventListener('click', function() {
                agreement.style.display = 'none'; // Hide the agreement page
                audio.style.display = 'block'; // Show the audio player
                audio.play(); // Start playing the audio
            });

            // Show the rickroll text when the audio starts playing
            audio.addEventListener('play', function() {
                rickrollText.style.display = 'block'; // Show the rickroll text
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

void setupServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html); 
  });

  // Serve the audio file from SPIFFS
  server.on("/audio", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (SPIFFS.exists("/your-audio-file.mp3")) {
      request->send(SPIFFS, "/your-audio-file.mp3", "audio/mpeg");
    } else {
      request->send(404, "text/plain", "Audio file not found");
    }
  });
}

void setup() {
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");

  WiFi.mode(WIFI_AP); 
  WiFi.softAP("Free Wifi");
  Serial.println("AP IP address: " + WiFi.softAPIP().toString());

  setupServer();

  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // Only when requested from AP
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
}
