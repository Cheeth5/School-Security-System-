#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "ESP8266_AP";    // Change to your desired SSID for the ESP8266's access point
const char* password = "password";  // Change to your desired password for the ESP8266's access point

const int ledPin = 16;  // Use built-in LED pin (usually GPIO2 on NodeMCU)

ESP8266WebServer server(80);

String detectedName = "";
String previousName = "";  // Example detected name

const char htmlLoginPage[] PROGMEM = R"(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Login Page</title>
</head>
<body>
  <h1>Login</h1>
  <form action="/login" method="post">
    <label for="username">Username:</label><br>
    <input type="text" id="username" name="username"><br>
    <label for="password">Password:</label><br>
    <input type="password" id="password" name="password"><br><br>
    <input type="submit" value="Login">
  </form>
</body>
</html>
)";

const char htmlContent[] PROGMEM = R"(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP8266 Access Point</title>
  <script>
    function updateNames(name) {
      const namesDiv = document.getElementById('detectedNames');
      namesDiv.innerHTML += name + "<br>"; // Append name with line break
    }
  </script>
</head>
<body>
  <h1>lycée Mahmoud Messadi ,Bardo </h1>
  <p>School Security System.</p>
  <p>Detected Names:</p>
  <div id="detectedNames"></div>
  <script>
    // Update detected names
    function updateDetectedName() {
      fetch('/getname')
        .then(response => response.text())
        .then(data => updateNames(data))
        .catch(error => console.error('Error fetching name:', error));
    }
    setInterval(updateDetectedName, 5000); // Update every 5 seconds
    updateDetectedName(); // Initial update
  </script>
</body>
</html>
)";

bool isAuthenticated = false;  // Flag to track authentication status

void handleRoot() {
  if (!isAuthenticated) {
    String page = FPSTR(htmlLoginPage);
    server.send(200, "text/html", page);
    return;
  }

  String page = FPSTR(htmlContent);
  page.replace("{DETECTED_NAME}", detectedName);
  server.send(200, "text/html", page);
}

void handleLogin() {
  if (!server.hasArg("username") || !server.hasArg("password")) {
    server.send(400, "text/plain", "Missing username or password");
    return;
  }

  if (server.arg("username") == "user" && server.arg("password") == "user") {
    isAuthenticated = true;  // Set authentication flag to true
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  } else {
    isAuthenticated = false;  // Set authentication flag to false
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
  }
}

void handleGetName() {
  server.send(200, "text/plain", detectedName);
}

void handleSetName() {
  Serial.println("Received arguments:");
  for (int i = 0; i < server.args(); i++) {
    Serial.print("Argument name: ");
    Serial.print(server.argName(i));
    Serial.print(", Value: ");
    Serial.println(server.arg(i));
  }

  if (server.hasArg("name")) {
    if (server.arg("name") != previousName) {
      detectedName = server.arg("name");
      previousName = detectedName;
      server.send(200, "text/plain", "Name updated successfully");
    } else {
      server.send(200, "text/plain", "Name already set");
    }
  } else {
    server.send(400, "text/plain", "Missing 'name' parameter in the request");
  }
}

void setup() {
  Serial.begin(115200);

  // Configure ESP8266 as Access Point
  WiFi.softAP(ssid, password);

  // Print IP address of the Access Point
  Serial.println("Access Point IP address:");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/getname", HTTP_GET, handleGetName);
  server.on("/setname", HTTP_GET, handleSetName);

  // Set LED pin as output and turn it on
  pinMode(ledPin, OUTPUT);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  digitalWrite(ledPin, LOW);
}
