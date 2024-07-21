#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Configuración del WiFi
const char* ssid = "Megacable_2.4G_265E";
const char* password = "";

// URL del servidor donde enviar los datos
const char* serverName = "http://192.168.1.8:8080/infoUltrasonico"; // Cambia por tu dirección

#define TRIG_PIN 14
#define ECHO_PIN 12

void setup() {
  // Inicializar el puerto serial
  Serial.begin(9600);
  
  // Configurar los pines del sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Conectar al WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado al WiFi");
}

void loop() {
  // Variables para la medición de distancia
  long duration;
  int distance;
  // Generar un pulso en el pin Trig
  digitalWrite(TRIG_PIN, LOW);  //para generar un pulso limpio ponemos a LOW 4us
  delayMicroseconds(4);
  digitalWrite(TRIG_PIN, HIGH);  //generamos Trigger (disparo) de 10us
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);


  // Medir el tiempo del pulso en el pin Echo
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calcular la distancia en cm
  distance = duration * 10 / 292/ 2;   //convertimos a distancia, en cm
  
  Serial.print("distance: ");
  Serial.print(distance);
  Serial.println("cm");
  
  
  // Enviar los datos a la API
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Preparar la URL para la solicitud HTTP POST
    String url = serverName;

    // Iniciar la conexión HTTP con WiFiClient
    WiFiClient client; // Crear instancia de WiFiClient
    http.begin(client, url); // Pasar WiFiClient como referencia

    // Especificar el tipo de contenido
    http.addHeader("Content-Type", "application/json");

    // Crear el JSON para enviar
    String httpRequestData = "{\"distance\":\"" + String(distance) + "\"}";

    // Enviar solicitud POST
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("Código de respuesta HTTP: ");
      Serial.println(httpResponseCode);
      //Serial.println(response);
    } else {
      Serial.print("Error en la solicitud HTTP: ");
      Serial.println(httpResponseCode);
    }

    // Finalizar la conexión
    http.end();
  } else {
    Serial.println("Error en la conexión WiFi");
  }

  // Esperar antes de la próxima medición
  delay(2000); // 2 segundos
}
