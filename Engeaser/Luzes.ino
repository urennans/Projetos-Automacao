#define sala 5
#define quarto 18
#define cozinha 19

const char* ssid     = "brisa-Pichuxu";
const char* password = "noir1508";

WiFiServer server(80);

void setup()
{
  Serial.begin(115200);

  pinMode(sala, OUTPUT);
  pinMode(quarto, OUTPUT);
  pinMode(cozinha, OUTPUT);

  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Conectando à ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  IPAddress ip(192, 168, 0, 7); //endereço ip para conectar à página web
  IPAddress gateway(192, 168, 0, 1); //Gateway padrão
  IPAddress subnet(255, 255, 255, 0); //máscara de rede

  WiFi.config(ip, gateway, subnet);

  Serial.println("");
  Serial.println("O Wifi está conectado.");
  Serial.println(" Endereço de IP: ");
  Serial.println(ip);

  server.begin();

}

int value = 0;

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("<html>");
            client.println("<head>");
            client.println("<title> Automação Residencial </title>");
            client.println("<meta charset=\"utf-8\">");
            client.println("<meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\">");
            client.println("</head>");
            client.println("<body>");
            client.println("<center>");
            client.println("<br>");

            client.println("<a href=\"/SALA\"\"><button>Sala</button></a>");
            client.println("<br>");
            client.println("<br>");
            client.println("<a href=\"/QUARTO\"\"><button>Quarto</button></a>");
            client.println("<br>");
            client.println("<br>");
            client.println("<a href=\"/COZINHA\"\"><button>Cozinha</button></a>");
            client.println("<br>");
            client.println("<br>");
            client.println("</center>");
            client.println("</body>");
            client.println("</html>");


            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }


        if (currentLine.endsWith("GET /SALA")) {
          digitalWrite(sala, !digitalRead(sala));
        }
        
        if (currentLine.endsWith("GET /QUARTO")) {
          digitalWrite(quarto, !digitalRead(quarto));
        }
        
        if (currentLine.endsWith("GET /COZINHA")) {
          digitalWrite(cozinha, !digitalRead(cozinha));
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}