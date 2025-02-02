#include <Arduino.h>
#include <Wire.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>
#include <GS_SDHelper.h>

// Definiciones de pines y constantes
#define LED_PIN 2           
#define TURBIDITY_PIN 32    
#define FLOW_PIN1 34        
#define FLOW_PIN2 35        
#define VALVE_CLEAN 25      
#define VALVE_DIRTY 26      
#define INTERVALO_LECTURA 500
#define TIEMPO_PROMEDIO 5000
#define FACTOR_CONVERSION 7.5
#define TIMEOUT_FLUJO 10000

#define MUY_SUCIA 1.40
#define ALGO_SUCIA 1.80

// Configuración WiFi y Google Sheets
// #define WIFI_SSID "Tu_SSID"
// #define WIFI_PASSWORD "Tu_Password"
// #define PROJECT_ID "Tu_Project_ID"
// #define CLIENT_EMAIL "Tu_Client_Email"
// const char PRIVATE_KEY[] PROGMEM = "Tu_Private_Key";
// const char spreadsheetId[] = "Tu_Spreadsheet_ID";


#define WIFI_SSID "Pedro891118"
#define WIFI_PASSWORD "fundacion123@"
// Google Project ID
#define PROJECT_ID "datalogesp32-449403"
// Service Account's client email
#define CLIENT_EMAIL "datalogesp32@datalogesp32-449403.iam.gserviceaccount.com"
// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDV9FHRcfo1T2I2\nD8L/xOnVsgqYJhkNM9ldAqqB3lIQER+0vveHPLIUgfrByhwFohJmSr21TDlaP7fG\nbX7NuOd0mgLzdeDFzHTauzhNICI8bUy0U+0RJ67+J0so6IBUYvy+88+CsqRLp0bh\n6/juDMT9RgzHNaP2fbsHyeJkxKdkXL87Bc2ZeiPmF2GTh89tP3WHMFfENMuy1OHZ\nHNZfr/MxnkT1eQgvqYD5C7rXjCiNlBN7/2YfnkWbiodFMqof44zhgn0C35DPYxSN\n8kr1j+Et3OC7rXlH3SEFz0Kam5sF68Vk87UJ+Pzb1zKBY80c91l9OGM+DlHZqtpR\nRbAb1MJXAgMBAAECggEAEr7bREyGLZGM+0uaPCqWikCVKQt9emimjL2lDsfZzMqT\nSpTIvzKUL7PLsprwpZWzov69bFsHU17tzRbe20tuDAtKYuZbETl+q4NnxWohVcYF\nyG7qAGzHqkDrFuR4Cey6VEisT7kzJ+W6EiVIpCK+R6qRiAqtGPWfI2mOymFogSRu\nAET1/UjoLOPqlW5PPo+z8qo6ogH+6upawpVeM9ZN6VfcAhHBxiFL4O+T4KiDJqMw\n7WlvFtbQqBFfBARTHJTqVRUS1TTlFmvJx8wltWfiZpLdkClICUIYladXS1hBfEsc\nn7C4VCmGolaS5mlUpU3a1c946pG6BczDWoNfoie0wQKBgQD9m8fL103YsdJ/Xtpb\nMipoBYTSFlVwdAawed3Jbb6pEW84BO6dbb+nzRDX21ixTRwJxLEP+h6IZpcAG+N3\naMN2f1Kg/wsM9fSXipNY+AORmkDWc5+R91ndoYZ9MajBQ+CtQ/9FyHgTv7fMI6yO\nRAaGHE8NUqYtQWynyIFQibMSRwKBgQDX+NANQAmhtKqO3PxtJdFw6XQAip+PdPZs\nAKJCTX64cRGneFpzreRooctpy8QD0VnUXfGrSg/bN4kxJFhr1NEiJ1zkkAN+QyUv\nApJT6pLUJfdXdGwOQbR9oRmPKl+CQpimwtK6eVQzv8fIOmUqCwfTIEBUq1sZHPp3\nwrGNbIhHcQKBgQDzyImYgtDMGrNzyKQxwdlNwPBaWymqPX4oqhzugdYjAyVcPjF5\nl4QCM3ebZ7isRqlLIo3qA87R43wHmMdhg41RHTc6l6/xt4R1H7pgbZhQoGzvOJoN\nYC48hAaZ1eCYVz1kkNaUKA51PXfaffhjkTSMOi4ramVfX2KpLm5QEu3WBQKBgC3b\nE3msvh7KeGhK/YHDRNv/hXTT+hsAj43Q+KKxR5Wr1D7FrBbhthyNNd5puF3DJpqT\ns8tVt1YpTYTUnb2PezQQPYX+Ge/GG/AGUMFmjfRjd0fnXUYNp2ABQ6cFLTBAKZ9G\nwG19c2ZlcurNk0tA+lFWiLIxi+8iGwAwo3hOsgTRAoGAF7dyic2NzSjfAPNE4gt/\nYidlrxY/g+tLJCdk3A1KtT+5/RLQYFJOj4Ra6+w6aLJ6addqbKn7buTRB0rRMGy4\nRaN0iQfIWu7pXVRoNrNrL3xKaCJwMEkB1zrcHgJ8ngpXs5tzYqzSISSRP4XAYIF2\ndXQvvFXAetDQXJg9JVc4aHM=\n-----END PRIVATE KEY-----\n";
// The ID of the spreadsheet where you'll publish the data
const char spreadsheetId[] = "1HbaVrY4ppDaMRl_sYbsy0PLPeR2vRpdURgByNdfRYq4";

// Variables para sensores de flujo
volatile int numPulsos1 = 0;
volatile int numPulsos2 = 0;
float volumenTotal1 = 0;
float volumenTotal2 = 0;
unsigned long tiempoUltimoFlujo1 = 0;
unsigned long tiempoUltimoFlujo2 = 0;
bool mensajeMostrado1 = false;
bool mensajeMostrado2 = false;
bool contadorReiniciado1 = false;
bool contadorReiniciado2 = false;

// Variables para acumulación de datos
struct MedicionAgua {
    char timestamp[25];
    float turbidez;
    float caudal1;
    float caudal2;
    float volumen1;
    float volumen2;
    String calidad;
};

const int MAX_SAMPLES = 10;
MedicionAgua dataArray[MAX_SAMPLES];
int currentIndex = 0;

// Variables de tiempo
unsigned long lastTime = 0;
unsigned long sampleTime = 0;
const unsigned long SAMPLE_DELAY = 1000;
const unsigned long SEND_DELAY = 10000;

// Variables NTP
const char* ntpServer = "pool.ntp.org";
unsigned long epochTime;

void IRAM_ATTR contarPulsos1() {
    numPulsos1++;
    tiempoUltimoFlujo1 = millis();
    if(contadorReiniciado1) {
        mensajeMostrado1 = false;
        contadorReiniciado1 = false;
    }
}

void IRAM_ATTR contarPulsos2() {
    numPulsos2++;
    tiempoUltimoFlujo2 = millis();
    if(contadorReiniciado2) {
        mensajeMostrado2 = false;
        contadorReiniciado2 = false;
    }
}

void obtenerCaudales(float &caudal1, float &caudal2) {
    float frecuencia1 = numPulsos1;
    float frecuencia2 = numPulsos2;
    
    caudal1 = frecuencia1/FACTOR_CONVERSION;
    caudal2 = frecuencia2/FACTOR_CONVERSION;
    
    if(caudal1 > 0) {
        volumenTotal1 += caudal1/60;
        tiempoUltimoFlujo1 = millis();
    } else if(millis() - tiempoUltimoFlujo1 > TIMEOUT_FLUJO && !contadorReiniciado1) {
        volumenTotal1 = 0;
        digitalWrite(VALVE_DIRTY, HIGH);
        digitalWrite(VALVE_CLEAN, LOW);
        if(!mensajeMostrado1) {
            Serial.println("Contador 1 reiniciado después de 10s sin flujo");
            mensajeMostrado1 = true;
        }
        contadorReiniciado1 = true;
    }

    if(caudal2 > 0) {
        volumenTotal2 += caudal2/60;
        tiempoUltimoFlujo2 = millis();
    } else if(millis() - tiempoUltimoFlujo2 > TIMEOUT_FLUJO && !contadorReiniciado2) {
        volumenTotal2 = 0;
        if(!mensajeMostrado2) {
            Serial.println("Contador 2 reiniciado después de 10s sin flujo");
            mensajeMostrado2 = true;
        }
        contadorReiniciado2 = true;
    }
    
    numPulsos1 = 0;
    numPulsos2 = 0;
}

String clasificarAgua(float voltage) {
    if(voltage < MUY_SUCIA) return "MUY SUCIA";
    if(voltage < ALGO_SUCIA) return "ALGO SUCIA";
    return "LIMPIA";
}

void tokenStatusCallback(TokenInfo info) {
    if (info.status == token_status_error) {
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
        GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
    } else {
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    }
}

void setup() {
    Serial.begin(115200);
    
    // Configuración de pines
    pinMode(LED_PIN, OUTPUT);
    pinMode(FLOW_PIN1, INPUT_PULLUP);
    pinMode(FLOW_PIN2, INPUT_PULLUP);
    pinMode(VALVE_CLEAN, OUTPUT);
    pinMode(VALVE_DIRTY, OUTPUT);
    digitalWrite(VALVE_DIRTY, HIGH);
    digitalWrite(VALVE_CLEAN, LOW);
    
    // Interrupciones para sensores de flujo
    attachInterrupt(digitalPinToInterrupt(FLOW_PIN1), contarPulsos1, RISING);
    attachInterrupt(digitalPinToInterrupt(FLOW_PIN2), contarPulsos2, RISING);

    // Configuración WiFi y tiempo
    configTime(0, 0, ntpServer);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConectado a WiFi");

    // Configuración Google Sheets
    GSheet.setTokenCallback(tokenStatusCallback);
    GSheet.setPrerefreshSeconds(10 * 60);
    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

void loop() {
    bool ready = GSheet.ready();
    unsigned long currentTime = millis();
    float caudal1, caudal2;
    
    // Tomar muestra cada segundo
    if (currentTime - sampleTime >= SAMPLE_DELAY && currentIndex < MAX_SAMPLES) {
        sampleTime = currentTime;
        
        // Leer sensores
        obtenerCaudales(caudal1, caudal2);
        float voltage = analogRead(TURBIDITY_PIN) * (3.3 / 4096.0);
        String calidad = clasificarAgua(voltage);

        // Obtener timestamp
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        
        // Guardar datos en el array
        strftime(dataArray[currentIndex].timestamp, 25, "%Y-%m-%d %H:%M:%S", &timeinfo);
        dataArray[currentIndex].turbidez = voltage;
        dataArray[currentIndex].caudal1 = caudal1;
        dataArray[currentIndex].caudal2 = caudal2;
        dataArray[currentIndex].volumen1 = volumenTotal1;
        dataArray[currentIndex].volumen2 = volumenTotal2;
        dataArray[currentIndex].calidad = calidad;
        
        // Control de LED según turbidez
        if(voltage < MUY_SUCIA) {
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
        } else if(voltage < ALGO_SUCIA) {
            digitalWrite(LED_PIN, HIGH);
            delay(500);
            digitalWrite(LED_PIN, LOW);
        } else {
            digitalWrite(LED_PIN, HIGH);
            delay(1000);
            digitalWrite(LED_PIN, LOW);
        }
        
        // Control de válvulas según calidad
        if(calidad == "LIMPIA") {
            digitalWrite(VALVE_CLEAN, HIGH);
            digitalWrite(VALVE_DIRTY, LOW);
        } else {
            digitalWrite(VALVE_CLEAN, LOW);
            digitalWrite(VALVE_DIRTY, HIGH);
        }
        
        currentIndex++;
        
        // Imprimir datos en Serial
        Serial.printf("Muestra %d: Turbidez=%.2fV (%s), Caudal1=%.2fL/m, Caudal2=%.2fL/m\n", 
                     currentIndex, voltage, calidad.c_str(), caudal1, caudal2);
    }

    // Enviar datos cuando se acumulen 10 muestras
    if (ready && currentIndex >= MAX_SAMPLES) {
        FirebaseJson response;
        FirebaseJson valueRange;
        
        Serial.println("\nEnviando lote de datos...");
        valueRange.add("majorDimension", "COLUMNS");
        
        // Añadir todos los datos acumulados
        for(int i = 0; i < currentIndex; i++) {
            String baseIndex = String(i);
            valueRange.set("values/[0]/[" + baseIndex + "]", String(dataArray[i].timestamp));
            valueRange.set("values/[1]/[" + baseIndex + "]", dataArray[i].turbidez);
            valueRange.set("values/[2]/[" + baseIndex + "]", dataArray[i].calidad);
            valueRange.set("values/[3]/[" + baseIndex + "]", dataArray[i].caudal1);
            valueRange.set("values/[4]/[" + baseIndex + "]", dataArray[i].caudal2);
            valueRange.set("values/[5]/[" + baseIndex + "]", dataArray[i].volumen1);
            valueRange.set("values/[6]/[" + baseIndex + "]", dataArray[i].volumen2);
        }

        // Enviar a Google Sheets
        bool success = GSheet.values.append(&response, spreadsheetId, "Sheet1!A1", &valueRange);
        if (success) {
            Serial.println("Lote de datos enviado exitosamente");
            valueRange.clear();
            currentIndex = 0;
        } else {
            Serial.println(GSheet.errorReason());
        }
    }
}