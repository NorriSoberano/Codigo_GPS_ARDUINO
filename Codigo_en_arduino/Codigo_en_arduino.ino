#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Arduino_JSON.h>

static const int RXPin = 4, TXPin = 5;
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
SoftwareSerial mygps(RXPin, TXPin);

const char* ssid = "HALCONES_CAFE";
const char* password = "987654321";
const char* serverUrl = "https://bryangn.pythonanywhere.com/api/gpsdata/";

float latitude;
float longitude;
float velocity;
float sats;
//int id=500;
String bearing;
WiFiClientSecure client;

void setup() {
  // Configuración inicial del programa

  // Configura el cliente para conexiones inseguras (IMPORTANTE)
  client.setInsecure();
  
  // Inicia la comunicación serial a 9600 baudios
  Serial.begin(9600);

  // Inicia la comunicación serial para el módulo GPS
  mygps.begin(GPSBaud);

  // Conéctate a la red WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  delay(1000);
}

void loop() {
  // Bucle principal del programa

  // Verifica si hay datos disponibles en la comunicación serial del GPS
  while (mygps.available() > 0) {
    // Intenta decodificar un nuevo mensaje del GPS
    if (gps.encode(mygps.read()))
      // Si se obtiene una lectura válida, envía los datos al servidor
      sendDataToServer();
  }
}

void sendDataToServer() {
  // Función para enviar los datos del GPS al servidor

  if (gps.location.isValid()) {
    // Obtiene datos válidos de la ubicación del GPS

    // Lee datos del objeto GPS
    sats = gps.satellites.value();
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    velocity = gps.speed.kmph();
    bearing = TinyGPSPlus::cardinal(gps.course.value());

    // Imprime los datos en la consola serial
    Serial.print("SATS: ");
    Serial.println(sats);
    Serial.print("LATITUDE: ");
    Serial.println(latitude, 6);
    Serial.print("LONGITUDE: ");
    Serial.println(longitude, 6);
    Serial.print("SPEED: ");
    Serial.print(velocity);
    Serial.println("kmph");
    Serial.print("DIRECTION: ");
    Serial.println(bearing);

    // Construye la cadena JSON con los datos del GPS
    String jsonData = "{\"id\":"+String(id)+",\"latitude\":" 
                        + String(latitude, 6) 
                        +",\"longitude\":" 
                        + String(longitude, 6) 
                        +",\"velocity\":" 
                        + String(velocity) 
                        +",\"satellites\":" 
                        + String(sats) 
                        +",\"bearing\":\"" 
                        + bearing + "\"}";

    // Imprime la cadena JSON en la consola serial
    Serial.println(jsonData);

    // Configura la solicitud HTTP
    HTTPClient http;
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("User-Agent", "ESP8266");
    http.addHeader("Host", "https://bryangn.pythonanywhere.com/api/gpsdata/");
    http.addHeader("Content-Length", String(jsonData.length()));

    // Realiza la solicitud POST con los datos JSON
    int httpResponseCode = http.POST(jsonData);
    
    // Verifica si la solicitud fue exitosa
    if (httpResponseCode > 0){
      // Muestra el código de respuesta del servidor
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
      
      // Imprime la respuesta del servidor
      String payload = http.getString();
      Serial.println(payload);
    }

    // Cierra la conexión
    http.end();

    // Espera 5 segundos antes de enviar la próxima solicitud
    delay(5000);
  }
}

