#include <WiFi.h>
#include <PubSubClient.h> 
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// ----------------------------------------------------------------------------------------- Configurações Wi-Fi ------
const char* ssid = "&&&*&&&";
const char* password = "&&&*&&&";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* topico_temp = "baby/temperatura";
const char* topico_som  = "baby/som";
const char* topico_pressao = "baby/pressao";
const char* topico_mov     = "baby/movimento";

WiFiClient espClient;
PubSubClient client(espClient);

#define ARDUINO_NAME "MonitoradorDeBabys3000"

static BLEUUID serviceUUID("180C");
static BLEUUID charUUID_Temp("2A6E");
static BLEUUID charUUID_Pres("2A6D");
// UUIDs de movimento (opcionais se não for usar X,Y,Z brutos, mas mantive pra conexão)
static BLEUUID charUUID_MovX("2AF3");
static BLEUUID charUUID_MovY("2AF4");
static BLEUUID charUUID_MovZ("2AF5");
static BLEUUID charUUID_Som("2AF6");
// Novo UUID
static BLEUUID charUUID_Agito("2AF7");

static BLEAdvertisedDevice* myDevice;
static bool deviceFound = false;
static bool connecting = false;
static bool connected = false;

BLERemoteCharacteristic* pTempChar;
BLERemoteCharacteristic* pPresChar;
BLERemoteCharacteristic* pSomChar;
BLERemoteCharacteristic* pAgitoChar; // Novo ponteiro
BLEClient* pClient;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.getName() == ARDUINO_NAME) {
            advertisedDevice.getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            deviceFound = true;
        }
    }
};

// Funções Auxiliares (String convert para ESP32 v3.0)
float lerFloat(BLERemoteCharacteristic* pChar) {
    if(pChar->canRead()) {
        String value = pChar->readValue();
        if (value.length() >= 4) {
            float f;
            memcpy(&f, value.c_str(), 4);
            return f;
        }
    }
    return 0.0;
}

int lerByte(BLERemoteCharacteristic* pChar) {
    if(pChar->canRead()) {
        String value = pChar->readValue();
        if (value.length() >= 1) return (int)value[0];
    }
    return 0;
}

bool connectToBLE() {
    pClient = BLEDevice::createClient();
    if (!pClient->connect(myDevice)) return false;

    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) { pClient->disconnect(); return false; }

    pTempChar = pRemoteService->getCharacteristic(charUUID_Temp);
    pPresChar = pRemoteService->getCharacteristic(charUUID_Pres);
    pSomChar  = pRemoteService->getCharacteristic(charUUID_Som);
    pAgitoChar = pRemoteService->getCharacteristic(charUUID_Agito); // Pega o novo

    if (!pTempChar || !pPresChar || !pSomChar || !pAgitoChar) {
        pClient->disconnect(); return false;
    }
    connected = true;
    return true;
}

void reconnectMQTT() {
    while (!client.connected()) {
        String clientId = "ESP32Client-" + String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            // Conectado
        } else {
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) delay(500);

    client.setServer(mqtt_server, mqtt_port);

    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
}

void loop() {
    if (!client.connected()) reconnectMQTT();
    client.loop();

    if (deviceFound && !connected && !connecting) {
        connecting = true;
        if (connectToBLE()) connecting = false;
        else { deviceFound = false; connecting = false; BLEDevice::getScan()->start(5, false); }
    }

    if (connected) {
        if(!pClient->isConnected()){ connected = false; deviceFound = false; return; }

        // --- 1. ALERTAS RÁPIDOS (Som e Movimento) ---
        int som = lerByte(pSomChar);
        client.publish(topico_som, som == 1 ? "1" : "0");

        int agito = lerByte(pAgitoChar);
        if (agito == 1) Serial.println("AGITO DETECTADO!");
        client.publish(topico_mov, agito == 1 ? "1" : "0");

        // --- 2. SENSORES LENTOS ---
        // Ler temperatura e pressão
        float temp = lerFloat(pTempChar);
        float pres = lerFloat(pPresChar);
        
        char tempStr[8]; dtostrf(temp, 1, 2, tempStr);
        char presStr[8]; dtostrf(pres, 1, 2, presStr);
        
        client.publish(topico_temp, tempStr);
        client.publish(topico_pressao, presStr);

        Serial.print("Temp: "); Serial.print(tempStr);
        Serial.print(" | Press: "); Serial.println(presStr);

        delay(1000); // Loop de leitura
    }
}
