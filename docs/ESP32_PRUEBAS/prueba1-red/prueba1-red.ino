#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

const char* ssid = "chat";
const char* password = "";
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);
WebSocketsServer webSocket(81);

String chatMessages = "";
int messageCooldown = 0;
bool networkVisible = true;
const String adminUser = "trolladorxs";
const String adminPass = "Troll@dorxs21";
unsigned long lastMessageTime = 0;
int timerDelays[] = {0, 10000, 20000};

void handleRoot() {
    server.send(200, "text/html", R"rawliteral(
        <html>
        <head>
            <style>
                body {
                    font-family: Arial, sans-serif;
                    text-align: center;
                    background: #f4f4f4;
                }
                .container {
                    background: white;
                    padding: 20px;
                    border-radius: 10px;
                    display: inline-block;
                }
                input, button {
                    padding: 10px;
                    margin: 5px;
                    border: none;
                    border-radius: 5px;
                    font-size: 16px;
                }
                button {
                    background: #007BFF;
                    color: white;
                    cursor: pointer;
                }
                button:hover {
                    background: #0056b3;
                }
            </style>
            <script>
                function login() {
                    var role = document.querySelector('input[name=role]:checked').value;
                    var color = document.getElementById('colorPicker').value;
                    if (role == 'admin') {
                        var user = prompt('Usuario:');
                        var pass = prompt('Contraseña:');
                        if (user == 'trolladorxs' && pass == 'Troll@dorxs21') {
                            window.location = '/chat?admin=1&color=' + encodeURIComponent(color);
                        } else {
                            alert('Credenciales incorrectas');
                        }
                    } else {
                        var name = prompt('Nombre:');
                        window.location = '/chat?user=' + encodeURIComponent(name) + '&color=' + encodeURIComponent(color);
                    }
                }
            </script>
        </head>
        <body>
            <div class="container">
                <h2>Bienvenido</h2>
                <input type='radio' name='role' value='admin'> Admin
                <input type='radio' name='role' value='user' checked> Usuario
                <br><br>
                <label>Elige tu color:</label>
                <input type="color" id="colorPicker">
                <br><br>
                <button onclick='login()'>Entrar</button>
            </div>
        </body>
        </html>
    )rawliteral");
}

void handleChat() {
    String username = server.arg("user");
    String color = server.arg("color");
    bool isAdmin = server.hasArg("admin");

    if (isAdmin) {
        username = "Admin";
    }

    String page = R"rawliteral(
        <html>
        <head>
            <style>
                body {
                    font-family: Arial, sans-serif;
                    text-align: center;
                    background: #ffffff;
                }
                #chat {
                    width: 90%;
                    max-width: 500px;
                    height: 300px;
                    overflow-y: auto;
                    border: 2px solid #444;
                    background: white;
                    padding: 10px;
                    border-radius: 10px;
                    margin: auto;
                }
                input, button {
                    padding: 10px;
                    margin: 5px;
                    border: none;
                    border-radius: 5px;
                    font-size: 16px;
                }
                button {
                    background: #007BFF;
                    color: white;
                    cursor: pointer;
                }
                button:hover {
                    background: #0056b3;
                }
            </style>
            <script>
                var socket = new WebSocket('ws://' + location.host + ':81/');
                socket.onmessage = function(event) {
                    document.getElementById('chat').innerHTML = event.data;
                };
                function sendMessage() {
                    var msg = document.getElementById('msg').value;
                    socket.send(JSON.stringify({user: ')rawliteral" + username + R"rawliteral(', color: ')rawliteral" + color + R"rawliteral(', msg: msg}));
                    document.getElementById('msg').value = '';
                }
                function changeBackground() {
                    var bgColor = document.getElementById('bgColorPicker').value;
                    document.body.style.backgroundColor = bgColor;
                }
            </script>
        </head>
        <body>
            <h2>Chat</h2>
            <div id='chat'></div>
            <input type='text' id='msg' placeholder='Escribe tu mensaje...'>
            <button onclick='sendMessage()'>Enviar</button>
            <br><br>
            <label>Cambiar fondo:</label>
            <input type='color' id='bgColorPicker' onchange='changeBackground()'>
    )rawliteral";

    if (isAdmin) {
        page += R"rawliteral(
            <button onclick='socket.send(JSON.stringify({clear: true}))'>Borrar Chat</button>
            <button onclick='socket.send(JSON.stringify({setTimer: true}))'>Configurar Temporizador</button>
            <button onclick='socket.send(JSON.stringify({toggleNetwork: true}))'>Ocultar Red</button>
        )rawliteral";
    }

    page += "</body></html>";
    server.send(200, "text/html", page);
}

void setup() {
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    server.on("/", handleRoot);
    server.on("/chat", handleChat);
    server.begin();
    webSocket.begin();
}

void loop() {
    server.handleClient();
    webSocket.loop();
}
