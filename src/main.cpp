#include <Arduino.h>
#include <ModbusRTU.h>
#include <SoftwareSerial.h>
#include "LEDfuncition.h"

#define SLAVE_ID 1
#define FIRST_REG 103
#define REG_COUNT 8
#define D1 26
#define D2 25
#define PIN_SERIAL 17
#define PIN_MODBUS 16

String var;
int dataBuffer[13];
int bufferIndex = 0;

SoftwareSerial serialTransparente(19, 18); // RX - TX
SoftwareSerial S(D1, D2);
ModbusRTU mb;

bool cb(Modbus::ResultCode event, uint16_t transactionId, void *data) {
    if (event != Modbus::EX_SUCCESS) {
        Serial.print("Request result: 0x");
        Serial.print(event, HEX);
    }
    return true;
}

float integer_to_float(uint16_t reg1, uint16_t reg2) {
    union converter {
        float f;
        uint16_t i[2];
    };
    union converter f_number;
    f_number.i[0] = reg2;
    f_number.i[1] = reg1;
    return f_number.f;
}

void setup() {
  setupLEDs();
    pinMode(27, OUTPUT); //ONOFF
    pinMode(PIN_SERIAL, INPUT);
    pinMode(PIN_MODBUS, INPUT);

    Serial.begin(9600);
    S.begin(9600, SWSERIAL_8N1);
    serialTransparente.begin(9600);
    mb.begin(&S);
    mb.master();

    Serial.println("Terminou o setup");
}

void loop() {
    digitalWrite(27, HIGH);
    if (digitalRead(PIN_SERIAL) == HIGH && digitalRead(PIN_MODBUS) == LOW) {
        if (S.available() > 0) {
            int receivedValue = S.read(); 
            Serial.println("Dado recebido na Serial.");
            
            blinkLED(LED_SERIAL, 300);

            dataBuffer[bufferIndex] = receivedValue; 
            bufferIndex++; 

            // Se o buffer estiver cheio (12 leituras), envia os dados e reseta o buffer
            if (bufferIndex >= 12) {
                for (int i = 0; i < 12; i++) {
                    serialTransparente.println(dataBuffer[i]);
                    Serial.println("Dado enviado via RF: " + String(dataBuffer[i]));
                }
                blinkLED(LED_RF, 300); // Pisca o LED para indicar que houve envio de dados via RF
                bufferIndex = 0; // Reseta o buffer e o índice
            }
        }
    } else {
        Serial.println("Dado recebido na Serial.");
        
        blinkLED(LED_SERIAL, 300); // Pisca o LED para indicar que recebeu dado na Serial

        uint16_t res[REG_COUNT];
        if (!mb.slave()) {
            mb.readIreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, cb);
            while (mb.slave()) {
                mb.task();
                delay(10);
            }

            // Criação de um buffer String para armazenar todos os dados
            String dataBuffer = "";

            dataBuffer += "AL V 05: " + String(res[0]) + "\n";
            dataBuffer += "AL V 06: " + String(res[1]) + "\n";
            dataBuffer += "AL V 07: " + String(res[2]) + "\n";
            dataBuffer += "AL V 08: " + String(res[3]) + "\n";
            dataBuffer += "AL mA 09: " + String(res[4]) + "\n";
            dataBuffer += "AL mA 10: " + String(res[5]) + "\n";
            dataBuffer += "AL mA 11: " + String(res[6]) + "\n";
            dataBuffer += "AL mA 12: " + String(res[7]) + "\n";

            float f_res1 = integer_to_float(res[0], res[1]);
            float f_res2 = integer_to_float(res[2], res[3]);
            float f_res3 = integer_to_float(res[4], res[5]);
            float f_res4 = integer_to_float(res[6], res[7]);

            // Enviar todos os dados de uma só vez
            serialTransparente.print(dataBuffer);
            Serial.println("Dados enviados via RF:");
            Serial.println(dataBuffer);
            
            blinkLED(LED_RF, 300); // Pisca o LED para indicar que houve envio de dados via RF
            dataBuffer = ""; // Limpar o buffer após enviar os dados
        }
        delay(30000);
    }
}
