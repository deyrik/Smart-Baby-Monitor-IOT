/*
 * MONITOR DE BEBÊ 3000 - VERSÃO "RAIO-X" (DEBUG DA IA)
 */

#include <Gabriel_SF_Padua-project-1_inferencing.h> 
// ----------------------------------------------------------------

#include <ArduinoBLE.h>
#include <Arduino_LPS22HB.h> 
#include <Arduino_LSM9DS1.h> 
#include <PDM.h> 

// --- Configurações BLE ---
BLEService babyService("180C");
BLEFloatCharacteristic tempChar("2A6E", BLERead | BLENotify);
BLEFloatCharacteristic presChar("2A6D", BLERead | BLENotify);
BLEByteCharacteristic somChar("2AF6", BLERead | BLENotify);   
BLEByteCharacteristic agitoChar("2AF7", BLERead | BLENotify); 

struct DadosSensor {
  float temperatura; float pressao; float ax, ay, az;
};
DadosSensor dados;

// Variáveis de Controle
unsigned long tempoAnterior = 0;
const long intervaloEnvio = 2000; 

// --- Configurações de Áudio ---
#define SAMPLE_BUFFER_SIZE EI_CLASSIFIER_RAW_SAMPLE_COUNT
static signed short sampleBuffer[SAMPLE_BUFFER_SIZE];
static volatile bool record_ready = false;
static volatile int sampleIndex = 0;

// Callback do Microfone
void onPDMdata() {
    // 1. Pergunta quantos bytes tem pra ler
    int bytesAvailable = PDM.available();

    // 2. Lê TUDO para um buffer temporário (para limpar o hardware)
    // Isso evita que o microfone trave!
    short tempBuffer[512]; 
    int samplesRead = bytesAvailable / 2;
    if (samplesRead > 512) samplesRead = 512;
    
    PDM.read(tempBuffer, bytesAvailable); // <--- AQUI ESTÁ A CORREÇÃO: LÊ SEMPRE!

    // 3. Só grava no nosso buffer principal se a IA estiver livre
    if (record_ready) {
        return; // Se já estamos cheios, ignoramos o que acabamos de ler (mas o hardware tá limpo!)
    }

    // 4. Se a IA está esperando dados, guarda no buffer principal
    for (int i = 0; i < samplesRead; i++) {
        if (sampleIndex < SAMPLE_BUFFER_SIZE) {
            sampleBuffer[sampleIndex] = tempBuffer[i];
            sampleIndex++;
        } else {
            record_ready = true; // Buffer encheu! Avisa o loop.
        }
    }
}

static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
    numpy::int16_to_float(&sampleBuffer[offset], out_ptr, length);
    return 0;
}

// --- Leitura e Envio de Sensores ---
void lerSensores() {
  if (BLE.central().connected()) {
     dados.temperatura = BARO.readTemperature();
     dados.pressao = BARO.readPressure();
     
     tempChar.writeValue(dados.temperatura);
     presChar.writeValue(dados.pressao);
     
     int agitoDetectado = 0;
     if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(dados.ax, dados.ay, dados.az);
        float mag = sqrt(pow(dados.ax, 2) + pow(dados.ay, 2) + pow(dados.az, 2));
        if (abs(mag - 1.0) > 0.5) agitoDetectado = 1;
        agitoChar.writeValue(agitoDetectado);
     }
     // Não vou imprimir sensores agora para limpar o Serial para a IA
  }
}

void setup() {
  Serial.begin(115200);
  // while(!Serial); // Descomente se quiser esperar abrir o monitor para começar

  Serial.println("--- INICIANDO DEBUGGER DE IA ---");

  if (!BARO.begin()) Serial.println("Erro Baro");
  if (!IMU.begin())  Serial.println("Erro IMU");
  if (!BLE.begin()) { Serial.println("Erro BLE"); while(1); }

  BLE.setLocalName("MonitoradorDeBabys3000");
  BLE.setAdvertisedService(babyService);
  babyService.addCharacteristic(tempChar);
  babyService.addCharacteristic(presChar);
  babyService.addCharacteristic(somChar);
  babyService.addCharacteristic(agitoChar);
  BLE.addService(babyService);
  BLE.advertise();
  
  Serial.println("Bluetooth OK. Iniciando Microfone...");

  PDM.onReceive(onPDMdata);
  // GANHO ALTO PARA TESTE (Se ficar muito chiado, baixe para 80 ou 64)
  PDM.setGain(100); 
  
  if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) {
      Serial.println("ERRO FATAL: Microfone não iniciou!"); while(1);
  } else {
      Serial.println("Microfone OK (Ganho: 100)");
  }
  Serial.println("--- AGUARDANDO BUFFER DE ÁUDIO ENCHER (FALE ALGO!) ---");
}

void loop() {
  BLE.poll(); 

  // 1. SENSORES (Prioridade de Tempo)
  if (millis() - tempoAnterior >= intervaloEnvio) {
      tempoAnterior = millis();
      if (BLE.central().connected()) lerSensores();
  }

  // 2. INTELIGÊNCIA ARTIFICIAL (O Foco do Debug)
  if (record_ready) {
      
      // --- DEBUG 1: O MICROFONE ESTÁ OUVINDO? ---
      long volumeTotal = 0;
      // Calcula a média do volume gravado (RMS simplificado)
      for (int i = 0; i < SAMPLE_BUFFER_SIZE; i+=10) { // Pula alguns para ser rápido
          volumeTotal += abs(sampleBuffer[i]);
      }
      int volumeMedio = volumeTotal / (SAMPLE_BUFFER_SIZE/10);
      
      Serial.print("[Vol: ");
      Serial.print(volumeMedio); // Se isso for < 50, o microfone ta surdo
      Serial.print("] ");

      // --- DEBUG 2: QUANTO TEMPO LEVA PRA PENSAR? ---
      unsigned long inicioIA = millis();

      signal_t signal;
      signal.total_length = SAMPLE_BUFFER_SIZE;
      signal.get_data = &microphone_audio_signal_get_data;
      ei_impulse_result_t result = { 0 };

      EI_IMPULSE_ERROR r = run_classifier(&signal, &result, false);
      
      unsigned long fimIA = millis();
      Serial.print("[IA: ");
      Serial.print(fimIA - inicioIA);
      Serial.print("ms] -> ");

      if (r == EI_IMPULSE_OK) {
          float probNenem = 0.0;
          
          // --- DEBUG 3: PROBABILIDADES ---
          for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
              Serial.print(ei_classifier_inferencing_categories[i]);
              Serial.print(": ");
              // Imprime com 2 casas decimais
              Serial.print(result.classification[i].value, 2); 
              Serial.print(" ");

              if (strcmp(ei_classifier_inferencing_categories[i], "nenem") == 0) {
                  probNenem = result.classification[i].value;
              }
          }
          Serial.println(); // Pula linha

          // DECISÃO
          if (probNenem > 0.50) {
              Serial.println(">>> 👶 CHORO DETECTADO! <<<");
              somChar.writeValue(1);
          } else {
              somChar.writeValue(0);
          }
      } else {
          Serial.println("Erro na execução da IA!");
      }
      
      // Reseta
      sampleIndex = 0;
      record_ready = false;
  }
}
