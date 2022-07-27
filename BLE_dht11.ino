#include "config.h"
#include "secret.h"
const char* ssid = WIFI;
const char* password = PASSWORD;

DHT dht(DHTPIN, DHTTYPE);


/* Define our custom characteristic along with it's properties */

BLECharacteristic dhtbhCharacteristic(
  BLEUUID((uint16_t)0x2A6E), 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY
);

BLECharacteristic humidityCharacteristic(
  BLEUUID((uint16_t)0x2A6F), 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY
);

/* This function handles the server callbacks */
bool deviceConnected = false;
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* MyServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* MyServer) {
      deviceConnected = false;
      BLEDevice::startAdvertising();
    }
};

void setup() {
  Serial.begin(9600);
  
  config_BLE();
  start_OTA();
  dht.begin();
}
void config_BLE(){
  // Create and name the BLE Device
  BLEDevice::init("ESP32C3-Temperature Sensor");

  /* Create the BLE Server */
  BLEServer *MyServer = BLEDevice::createServer();
  MyServer->setCallbacks(new ServerCallbacks());  // Set the function that handles Server Callbacks

  /* Add a service to our server */
  BLEService *customService = MyServer->createService((uint16_t)0x181A); //  A random ID has been selected
  /* Add a characteristic to the service */
  customService->addCharacteristic(&dhtbhCharacteristic);
  customService->addCharacteristic(&humidityCharacteristic);
  /* Add Descriptors to the Characteristic*/
  dhtbhCharacteristic.addDescriptor(new BLE2902());
  humidityCharacteristic.addDescriptor(new BLE2902());

  /* Configure Advertising with the Services to be advertised */
  MyServer->getAdvertising()->addServiceUUID((uint16_t)0x181A);

  // Start the service
  customService->start();

  // Start the Server/Advertising
  MyServer->getAdvertising()->start();
}
void start_OTA(){
  Serial.println("Waiting for a Client to connect...");
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
void loop() {
  dhtRead_t = dht.readTemperature();
  dhtRead_h = dht.readHumidity();
  dhtRead_t = dhtRead_t * 100;
  dhtRead_h = dhtRead_h * 100;
  if (deviceConnected) {
    /* Set the value */
    dhtbhCharacteristic.setValue((uint8_t*)&dhtRead_t, 2);  // This is a value of 4 single bytes
    dhtbhCharacteristic.notify();
    humidityCharacteristic.setValue((uint8_t*)&dhtRead_h, 2);
    humidityCharacteristic.notify();
  } else {
    
  }
  delay(100);
  ArduinoOTA.handle();
}
