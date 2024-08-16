#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define SLAVE_ID 1
#define FIRST_REG 107
#define REG_COUNT 6
#define D1 26
#define D2 25
#define PIN_SERIAL 17
#define PIN_MODBUS 16
#define LED_ESP 27
#define LED_SERIAL 12
#define LED_RF 14

String var;
int dataBuffer[13];
int bufferIndex = 0;

int primeiroByteDados;
int segundoByteDados;
int terceiroByteDados;
int quartoByteDados;
int quintoByteDados;
int sextoByteDados;

SoftwareSerial serialTransparente(19, 18); // RX - TX
SoftwareSerial S(D1, D2);
ModbusRTU mb;

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
  pinMode(LED_ESP, OUTPUT);    // LED para indicar que o ESP32 está ligado
  pinMode(LED_SERIAL, OUTPUT); // LED para indicar comunicação via Serial
  pinMode(LED_RF, OUTPUT);     // LED para indicar comunicação via RF

  pinMode(PIN_SERIAL, INPUT);
  pinMode(PIN_MODBUS, INPUT);

  Serial.begin(9600);
  S.begin(9600, SWSERIAL_8N1);
  serialTransparente.begin(9600);
  mb.begin(&S);
  mb.master();

  // Acende o LED ao iniciar o ESP32
  digitalWrite(LED_ESP, HIGH);
  Serial.println("Terminou o setup");
}

void loop()
{
  if (digitalRead(PIN_SERIAL) == HIGH && digitalRead(PIN_MODBUS) == LOW)
  {
    if (S.available() > 0)
    {
      int receivedValue = S.read(); // Lê o valor recebido
      Serial.println("Dado recebido na Serial.");

      // Pisca o LED para indicar que recebeu dado na Serial
      digitalWrite(LED_SERIAL, HIGH);
      delay(300);
      digitalWrite(LED_SERIAL, LOW);

      dataBuffer[bufferIndex] = receivedValue; // Armazena no buffer
      bufferIndex++;                           // Incrementa o índice do buffer

      // Se o buffer estiver cheio (12 leituras), envia os dados e reseta o buffer
      if (bufferIndex >= 12)
      {
        for (int i = 0; i < 12; i++)
        {
          serialTransparente.println(dataBuffer[i]);
          Serial.println("Dado enviado via RF: " + String(dataBuffer[i]));
        }

        // Pisca o LED para indicar que houve envio de dados via RF
        digitalWrite(LED_RF, HIGH);
        delay(300);
        digitalWrite(LED_RF, LOW);

        // Reseta o buffer e o índice
        bufferIndex = 0;
      }
    }
  }
  else
  {
    Serial.println("Dado recebido na Serial.");

    // Pisca o LED para indicar que recebeu dado na Serial
    digitalWrite(LED_SERIAL, HIGH);
    delay(300);
    digitalWrite(LED_SERIAL, LOW);

    uint16_t res[REG_COUNT];
    float f_res1, f_res2, f_res3;

    if (!mb.slave())
    {
      mb.readIreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, cb);
      while (mb.slave())
      {
        mb.task();
        delay(10);
      }

      // Criação de um buffer String para armazenar todos os dados
      String dataBuffer = "";

      dataBuffer += "AL 09: " + String(res[0]) + "\n";
      dataBuffer += "AL 10: " + String(res[1]) + "\n";
      dataBuffer += "AL 11: " + String(res[2]) + "\n";
      dataBuffer += "AL 12: " + String(res[3]) + "\n";

      f_res1 = integer_to_float(res[0], res[1]);
      f_res2 = integer_to_float(res[2], res[3]);
      f_res3 = integer_to_float(res[4], res[5]);

      // Enviar todos os dados de uma só vez
      serialTransparente.print(dataBuffer);
      Serial.println("Dados enviados via RF:");

      // Pisca o LED para indicar que houve envio de dados via RF
      digitalWrite(LED_RF, HIGH);
      delay(300);
      digitalWrite(LED_RF, LOW);

      // Limpar o buffer após enviar os dados
      dataBuffer = "";
    }
    delay(30000);
  }
}
