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
#define FACTOR_CONVERSION 7.5

// Constantes para clasificación del agua
#define MUY_SUCIA 1.40
#define ALGO_SUCIA 1.80

// Tiempos y períodos
#define PERIODO_MEDICION 5000    // 5 segundos de medición inicial
#define INTERVALO_MUESTREO 100   // 100ms entre muestras durante medición
#define INTERVALO_ENVIO 10000    // 10 segundos entre envíos
#define TIMEOUT_FLUJO 10000      // 10 segundos sin flujo para reinicio

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

// Configuración NTP
const char* ntpServer = "pool.ntp.org";

// Variables para el período de medición
bool enPeriodoMedicion = false;
unsigned long inicioPeriodoMedicion = 0;
float sumaTurbidez = 0;
int contadorMuestras = 0;
bool decisionValvulaTomada = false;

// Estructura para almacenar los registros
struct Registro {
    char timestamp[25];
    float turbidez;
    String calidad;
    float caudal1;
    float caudal2;
    float volumen1;
    float volumen2;
    String valve1_estado;
    String valve2_estado;
    bool valido;
};

// Array para almacenar registros
const int MAX_REGISTROS = 10;
Registro registros[MAX_REGISTROS];
int indiceRegistro = 0;

// Variables para sensores de flujo
volatile int numPulsos1 = 0;
volatile int numPulsos2 = 0;
float volumenTotal1 = 0;
float volumenTotal2 = 0;
unsigned long tiempoUltimoFlujo1 = 0;
unsigned long tiempoUltimoFlujo2 = 0;
unsigned long ultimoEnvio = 0;
unsigned long ultimoRegistro = 0;

void IRAM_ATTR contarPulsos1() {
    numPulsos1++;
    tiempoUltimoFlujo1 = millis();
}

void IRAM_ATTR contarPulsos2() {
    numPulsos2++;
    tiempoUltimoFlujo2 = millis();
}

void obtenerCaudales(float &caudal1, float &caudal2) {
    float frecuencia1 = numPulsos1;
    float frecuencia2 = numPulsos2;
    
    caudal1 = frecuencia1/FACTOR_CONVERSION;
    caudal2 = frecuencia2/FACTOR_CONVERSION;
    
    if(caudal1 > 0) {
        volumenTotal1 += caudal1/60;
    }
    
    if(caudal2 > 0) {
        volumenTotal2 += caudal2/60;
    }
    
    numPulsos1 = 0;
    numPulsos2 = 0;
}

String clasificarAgua(float voltage) {
    if(voltage < MUY_SUCIA) return "MUY SUCIA";
    if(voltage < ALGO_SUCIA) return "ALGO SUCIA";
    return "LIMPIA";
}

void reiniciarMedicion() {
    enPeriodoMedicion = false;
    decisionValvulaTomada = false;
    sumaTurbidez = 0;
    contadorMuestras = 0;
    
    // Al reiniciar, volver al estado por defecto: válvula sucia ON, limpia OFF
    digitalWrite(VALVE_CLEAN, LOW);
    digitalWrite(VALVE_DIRTY, HIGH);
    
    Serial.println("Sistema reiniciado - Estado por defecto: Válvula sucia ON");
}

void controlarValvulas(String calidad) {
    if(calidad == "LIMPIA") {
        digitalWrite(VALVE_CLEAN, HIGH);   // Válvula limpia ON
        digitalWrite(VALVE_DIRTY, LOW);    // Válvula sucia OFF
        Serial.println("Agua limpia detectada - Cambiando a válvula limpia");
    } else {
        digitalWrite(VALVE_CLEAN, LOW);    // Válvula limpia OFF
        digitalWrite(VALVE_DIRTY, HIGH);   // Válvula sucia ON
        Serial.println("Agua sucia detectada - Manteniendo válvula sucia");
    }
    
    // Verificar que nunca estén ambas válvulas en el mismo estado
    if(digitalRead(VALVE_CLEAN) == digitalRead(VALVE_DIRTY)) {
        // Si por algún error están en el mismo estado, forzar estados opuestos
        digitalWrite(VALVE_CLEAN, !digitalRead(VALVE_DIRTY));
        Serial.println("ADVERTENCIA: Corrigiendo estado de válvulas");
    }
}

void tokenStatusCallback(TokenInfo info) {
    if (info.status == token_status_error) {
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
        GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
    } else {
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    }
}

void enviarRegistrosASheet() {
    if (!GSheet.ready()) return;
    
    FirebaseJson response;
    FirebaseJson valueRange;
    
    Serial.println("\nEnviando registros...");
    valueRange.add("majorDimension", "ROWS");
    
    int registrosValidos = 0;
    for(int i = 0; i < MAX_REGISTROS; i++) {
        if(registros[i].valido) registrosValidos++;
    }
    
    if(registrosValidos == 0) {
        Serial.println("No hay registros válidos para enviar");
        return;
    }
    
    Serial.printf("Enviando %d registros\n", registrosValidos);
    
    int indiceValido = 0;
    for(int i = 0; i < MAX_REGISTROS; i++) {
        if(registros[i].valido) {
            valueRange.set("values/[" + String(indiceValido) + "]/[0]", String(registros[i].timestamp));
            valueRange.set("values/[" + String(indiceValido) + "]/[1]", registros[i].turbidez);
            valueRange.set("values/[" + String(indiceValido) + "]/[2]", registros[i].calidad);
            valueRange.set("values/[" + String(indiceValido) + "]/[3]", registros[i].caudal1);
            valueRange.set("values/[" + String(indiceValido) + "]/[4]", registros[i].caudal2);
            valueRange.set("values/[" + String(indiceValido) + "]/[5]", registros[i].volumen1);
            valueRange.set("values/[" + String(indiceValido) + "]/[6]", registros[i].volumen2);
            valueRange.set("values/[" + String(indiceValido) + "]/[7]", registros[i].valve1_estado);
            valueRange.set("values/[" + String(indiceValido) + "]/[8]", registros[i].valve2_estado);
            indiceValido++;
        }
    }

    bool success = GSheet.values.append(&response, spreadsheetId, "Sheet1!A1", &valueRange);
    if (success) {
        Serial.println("Registros enviados exitosamente");
        valueRange.clear();
        
        // Reiniciar registros
        for(int i = 0; i < MAX_REGISTROS; i++) {
            registros[i].valido = false;
        }
        indiceRegistro = 0;
    } else {
        Serial.println(GSheet.errorReason());
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
    
    // Estado inicial: válvula sucia ON, válvula limpia OFF
    digitalWrite(VALVE_CLEAN, LOW);
    digitalWrite(VALVE_DIRTY, HIGH);
    Serial.println("Estado inicial: Válvula sucia ON, Válvula limpia OFF");
    
    // Configurar interrupciones
    attachInterrupt(digitalPinToInterrupt(FLOW_PIN1), contarPulsos1, RISING);
    attachInterrupt(digitalPinToInterrupt(FLOW_PIN2), contarPulsos2, RISING);

    // Inicializar registros
    for(int i = 0; i < MAX_REGISTROS; i++) {
        registros[i].valido = false;
    }

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
    unsigned long currentTime = millis();
    float caudal1, caudal2;
    
    // Obtener lecturas de sensores
    obtenerCaudales(caudal1, caudal2);
    float voltage = analogRead(TURBIDITY_PIN) * (3.3 / 4096.0);
    String calidad = clasificarAgua(voltage);
    
    // Detectar inicio de flujo nuevo
    if((caudal1 > 0 || caudal2 > 0) && !enPeriodoMedicion && !decisionValvulaTomada) {
        enPeriodoMedicion = true;
        inicioPeriodoMedicion = currentTime;
        Serial.println("Iniciando período de medición...");
    }
    
    // Proceso durante el período de medición
    if(enPeriodoMedicion) {
        if(currentTime - inicioPeriodoMedicion < PERIODO_MEDICION) {
            // Acumular mediciones cada INTERVALO_MUESTREO
            if(currentTime % INTERVALO_MUESTREO == 0) {
                sumaTurbidez += voltage;
                contadorMuestras++;
                Serial.printf("Muestra %d: %.2fV\n", contadorMuestras, voltage);
            }
        } else {
            // Fin del período de medición
            float turbidezPromedio = sumaTurbidez / contadorMuestras;
            String calidadFinal = clasificarAgua(turbidezPromedio);
            
            Serial.printf("Fin de medición - Turbidez promedio: %.2f, Calidad: %s\n", 
                         turbidezPromedio, calidadFinal.c_str());
            
            controlarValvulas(calidadFinal);
            enPeriodoMedicion = false;
            decisionValvulaTomada = true;
        }
    }
    
    // Registrar datos cada segundo si hay flujo
    if((caudal1 > 0 || caudal2 > 0) && currentTime - ultimoRegistro >= 1000) {
        ultimoRegistro = currentTime;
        
        // Obtener timestamp
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        
        // Guardar registro
        strftime(registros[indiceRegistro].timestamp, 25, "%Y-%m-%d %H:%M:%S", &timeinfo);
        registros[indiceRegistro].turbidez = voltage;
        registros[indiceRegistro].calidad = calidad;
        registros[indiceRegistro].caudal1 = caudal1;
        registros[indiceRegistro].caudal2 = caudal2;
        registros[indiceRegistro].volumen1 = volumenTotal1;
        registros[indiceRegistro].volumen2 = volumenTotal2;
        registros[indiceRegistro].valve1_estado = digitalRead(VALVE_CLEAN) ? "ON" : "OFF";
        registros[indiceRegistro].valve2_estado = digitalRead(VALVE_DIRTY) ? "ON" : "OFF";
        registros[indiceRegistro].valido = true;
        
        Serial.printf("Registro guardado %d: Caudal1=%.2f, Caudal2=%.2f\n", 
                     indiceRegistro, caudal1, caudal2);
        
        indiceRegistro = (indiceRegistro + 1) % MAX_REGISTROS;
    }
    
    // Enviar datos acumulados cada 10 segundos
    if(currentTime - ultimoEnvio >= INTERVALO_ENVIO) {
        ultimoEnvio = currentTime;
        enviarRegistrosASheet();
    }
    
    // Detectar fin de flujo y reiniciar estado
    if(caudal1 == 0 && caudal2 == 0 && decisionValvulaTomada) {
        if(currentTime - tiempoUltimoFlujo1 > TIMEOUT_FLUJO && 
           currentTime - tiempoUltimoFlujo2 > TIMEOUT_FLUJO) {
            
            Serial.println("Flujo detenido - Guardando estado final antes de reiniciar");
            
            // Guardar un último registro con el cambio de válvulas
            time_t now;
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            
            // Registrar el estado actual antes del cambio
            strftime(registros[indiceRegistro].timestamp, 25, "%Y-%m-%d %H:%M:%S", &timeinfo);
            registros[indiceRegistro].turbidez = voltage;
            registros[indiceRegistro].calidad = calidad;
            registros[indiceRegistro].caudal1 = 0;
            registros[indiceRegistro].caudal2 = 0;
            registros[indiceRegistro].volumen1 = volumenTotal1;
            registros[indiceRegistro].volumen2 = volumenTotal2;
            registros[indiceRegistro].valve1_estado = digitalRead(VALVE_CLEAN) ? "ON" : "OFF";
            registros[indiceRegistro].valve2_estado = digitalRead(VALVE_DIRTY) ? "ON" : "OFF";
            registros[indiceRegistro].valido = true;
            indiceRegistro = (indiceRegistro + 1) % MAX_REGISTROS;
            
            // Volver a estado inicial y registrar el cambio
            digitalWrite(VALVE_CLEAN, LOW);
            digitalWrite(VALVE_DIRTY, HIGH);
            
            // Registrar el nuevo estado después del cambio
            strftime(registros[indiceRegistro].timestamp, 25, "%Y-%m-%d %H:%M:%S", &timeinfo);
            registros[indiceRegistro].turbidez = voltage;
            registros[indiceRegistro].calidad = calidad;
            registros[indiceRegistro].caudal1 = 0;
            registros[indiceRegistro].caudal2 = 0;
            registros[indiceRegistro].volumen1 = 0;
            registros[indiceRegistro].volumen2 = 0;
            registros[indiceRegistro].valve1_estado = "OFF";  // Nuevo estado
            registros[indiceRegistro].valve2_estado = "ON";   // Nuevo estado
            registros[indiceRegistro].valido = true;
            indiceRegistro = (indiceRegistro + 1) % MAX_REGISTROS;
            
            // Forzar un envío inmediato a Google Sheets
            enviarRegistrosASheet();
            
            Serial.println("Reiniciando sistema - Válvulas en posición inicial");
            reiniciarMedicion();
            volumenTotal1 = 0;
            volumenTotal2 = 0;
        }
    }
    
    delay(100); // Pequeña pausa para estabilidad
}