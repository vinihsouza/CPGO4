#include <Arduino_LSM6DSOX.h>
#include <WiFiNINA.h>

float atualX, atualY, atualZ;
float posiX, posiY, posiZ;

char ssid[] = "MeuAP";
char pass[] = "12345678";
WiFiServer server(80);

void zeraLeds();
void mostraZ(float Zvalue, String &htmlContent, int color);

void setup() {
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  Serial.begin(9600);
  IMU.begin();

  delay(500);
  IMU.readAcceleration(posiX, posiY, posiZ);

  Serial.println("Configurando o ponto de acesso...");
  if (WiFi.beginAP(ssid, pass) != WL_AP_LISTENING) {
    Serial.println("Falha ao configurar o ponto de acesso.");
    while (true);
  }
  
  Serial.println("Ponto de acesso configurado!");
  Serial.print("IP do AP: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();  // Verifica se há um cliente conectado
  
  IMU.readAcceleration(atualX, atualY, atualZ);
  
  String htmlContent;
  
  if (atualZ < posiZ - 0.5) {
    mostraZ(atualZ, htmlContent, 1);  // Queda detectada
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDB, LOW);
  } else if (atualZ < posiZ - 0.2) {
    mostraZ(atualZ, htmlContent, 2);  // Instabilidade
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, LOW);
  } else if (atualZ < posiZ - 0.1 || atualZ > posiZ + 0.1) {
    mostraZ(atualZ, htmlContent, 3);  // Movimento detectado
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, LOW);
  } else {
    zeraLeds();
    htmlContent = "<html><body style='background-color:gray;'><h1 style='color:black;'>Normal</h1></body></html>";
  }

  if (client) {
    Serial.println("Novo cliente conectado!");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    
    client.print("<html><head>");
    client.print("<meta http-equiv='refresh' content='2'>");  // Atualiza a página a cada 2 segundos
    client.print("</head><body>");
    client.println(htmlContent);
    client.print("</body></html>");
    
    delay(10);
    client.stop();
  }
}

void zeraLeds() {
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);
}

void mostraZ(float Zvalue, String &htmlContent, int color) {
  zeraLeds();
  if (color == 1) {
    htmlContent = "<html><body style='background-color:red;'><h1 style='color:white;'>Queda Detectada!</h1></body></html>";
  } else if (color == 2) {
    htmlContent = "<html><body style='background-color:yellow;'><h1 style='color:black;'>Atencao: Instabilidade</h1></body></html>";
  } else if (color == 3) {
    htmlContent = "<html><body style='background-color:green;'><h1 style='color:white;'>Movimento Detectado</h1></body></html>";
  }
}
