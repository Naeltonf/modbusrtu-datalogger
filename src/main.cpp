#include <Arduino.h>
#include <ModbusRTU.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "LEDfuncition.h"

#define SLAVE_ID 1
#define FIRST_REG 103
#define REG_COUNT 8
#define D1 26
#define D2 25
#define PIN_SERIAL 17
#define PIN_MODBUS 16

int dataBuffer[13];
int bufferIndex = 0;
String htmlContent;

SoftwareSerial serialTransparente(19, 18); // RX - TX
SoftwareSerial S(D1, D2);
ModbusRTU mb;

// Configuração do servidor
AsyncWebServer server(80);

// HTML e CSS como strings
const char *htmlHeader = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Dados Coletados</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f4f4f4;
            color: #333;
        }
        h1 {
            color: #4CAF50;
            text-align: center;
        }
        pre {
            background: #fff;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
            max-width: 800px;
            margin: 0 auto;
            white-space: pre-wrap; /* Permite a quebra de linha no conteúdo */
        }
    </style>
    <script>
        function fetchData() {
            fetch('/data')
                .then(response => response.text())
                .then(data => {
                    document.getElementById('data').innerHTML = data;
                })
                .catch(error => console.error('Error fetching data:', error));
        }

        setInterval(fetchData, 5000); // Atualiza a cada 5 segundos
        window.onload = fetchData; // Carrega os dados quando a página é carregada
    </script>
</head>
<body>
    <h1>Dados Coletados</h1>
    <pre id="data">Carregando...</pre>
</body>
</html>
)rawliteral";

bool cb(Modbus::ResultCode event, uint16_t transactionId, void *data)
{
    if (event != Modbus::EX_SUCCESS)
    {
        Serial.print("Request result: 0x");
        Serial.print(event, HEX);
    }
    return true;
}

float integer_to_float(uint16_t reg1, uint16_t reg2)
{
    union converter
    {
        float f;
        uint16_t i[2];
    };
    union converter f_number;
    f_number.i[0] = reg2;
    f_number.i[1] = reg1;
    return f_number.f;
}

void setup()
{
    setupLEDs();
    pinMode(27, OUTPUT); // ONOFF
    pinMode(PIN_SERIAL, INPUT);
    pinMode(PIN_MODBUS, INPUT);

    Serial.begin(9600);
    S.begin(9600, SWSERIAL_8N1);
    serialTransparente.begin(9600);
    mb.begin(&S);
    mb.master();

    // Configura a rede Wi-Fi do ESP32 como Access Point
    WiFi.softAP("Interface Vetorlog", "vetorlog2024");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("IP Address: ");
    Serial.println(IP);

    // Configuração do servidor web
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        // Envia o HTML principal
        request->send(200, "text/html", String(htmlHeader)); });

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        // Atualiza a página HTML com os dados coletados
        String html = htmlContent;
        html.replace("\n", "<br>"); // Substitui as quebras de linha por quebras de linha HTML
        request->send(200, "text/html", html); });

    server.begin();
    Serial.println("Servidor HTTP iniciado.");
}

void loop()
{
    digitalWrite(27, HIGH);
    if (digitalRead(PIN_SERIAL) == HIGH && digitalRead(PIN_MODBUS) == LOW)
    {
        if (S.available() > 0)
        {
            int receivedValue = S.read();
            Serial.println("Dado recebido na Serial.");

            blinkLED(LED_SERIAL, 300);

            dataBuffer[bufferIndex] = receivedValue;
            bufferIndex++;

            // Se o buffer estiver cheio (12 leituras), envia os dados e reseta o buffer
            if (bufferIndex >= 12)
            {
                String dataStr = "";
                for (int i = 0; i < 12; i++)
                {
                    serialTransparente.println(dataBuffer[i]);
                    Serial.println("Dado enviado via RF: " + String(dataBuffer[i]));
                    // Adiciona os dados ao conteúdo HTML
                    dataStr += "Byte " + String(i + 1) + ": " + String(dataBuffer[i]) + "\n";
                }
                blinkLED(LED_RF, 300); // Pisca o LED para indicar que houve envio de dados via RF
                bufferIndex = 0;       // Reseta o buffer e o índice

                // Atualiza a variável 'htmlContent' com os dados recebidos na Serial
                htmlContent = "Dados Serial:<br>" + dataStr;
                htmlContent.replace("\n", "<br>"); // Substitui as quebras de linha por quebras de linha HTML
            }
        }
    }
    else
    {
        Serial.println("Dado recebido na Serial.");

        blinkLED(LED_SERIAL, 300); // Pisca o LED para indicar que recebeu dado na Serial

        uint16_t res[REG_COUNT];
        if (!mb.slave())
        {
            mb.readIreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, cb);
            while (mb.slave())
            {
                mb.task();
                delay(10);
            }

            // Criação de um buffer String para armazenar todos os dados
            String dataStr = "";

            dataStr += "AL V 05: " + String(res[0]) + "\n";
            dataStr += "AL V 06: " + String(res[1]) + "\n";
            dataStr += "AL V 07: " + String(res[2]) + "\n";
            dataStr += "AL V 08: " + String(res[3]) + "\n";
            dataStr += "AL mA 09: " + String(res[4]) + "\n";
            dataStr += "AL mA 10: " + String(res[5]) + "\n";
            dataStr += "AL mA 11: " + String(res[6]) + "\n";
            dataStr += "AL mA 12: " + String(res[7]) + "\n";

            float f_res1 = integer_to_float(res[0], res[1]);
            float f_res2 = integer_to_float(res[2], res[3]);
            float f_res3 = integer_to_float(res[4], res[5]);
            float f_res4 = integer_to_float(res[6], res[7]);

            // Enviar todos os dados de uma só vez
            serialTransparente.print(dataStr);
            Serial.println("Dados enviados via RF:");
            Serial.println(dataStr);

            blinkLED(LED_RF, 300); // Pisca o LED para indicar que houve envio de dados via RF

            // Limpar a variável 'htmlContent' e atualizar com dados Modbus
            htmlContent = "Dados Modbus:<br>" + dataStr;
            htmlContent.replace("\n", "<br>"); // Substitui as quebras de linha por quebras de linha HTML
        }
        delay(30000);
    }
}