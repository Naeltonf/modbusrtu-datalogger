#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define SLAVE_ID 1
//numero inicial
//#define FIRST_REG 15
#define FIRST_REG 107
#define REG_COUNT 6
#define D1 26
#define D2 25

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
{ // Callback to monitor errors
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
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(27, OUTPUT);

  Serial.begin(9600);
  S.begin(9600, SWSERIAL_8N1);
  serialTransparente.begin(9600);
  mb.begin(&S);
  mb.master();
  Serial.print("Terminou o setup");
}

void loop()
{
  digitalWrite(12, HIGH);
  delay(250);
  digitalWrite(12, LOW);
  delay(250);

  digitalWrite(14, HIGH);
  delay(250);
  digitalWrite(14, LOW);
  delay(250);

  digitalWrite(27, HIGH);

  serialTransparente.println("teste");
  delay(2000);

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

    Serial.println();
    Serial.print("Primeiro Byte: ");
    primeiroByteDados = res[0];
    Serial.println(primeiroByteDados);

    Serial.print("Segundo Byte: ");
    segundoByteDados = res[1];
    Serial.println(segundoByteDados);

    Serial.print("Terceiro Byte: ");
    terceiroByteDados = res[2];
    Serial.println(terceiroByteDados);

    Serial.print("Quarto Byte: ");
    quartoByteDados = res[3];
    Serial.println(quartoByteDados);

    Serial.print("Quinto Byte: ");
    quintoByteDados = res[4];
    Serial.println(quintoByteDados);

    Serial.print("Sexto Byte: ");
    sextoByteDados = res[5];
    Serial.println(sextoByteDados);

    // Convert each pair of registers to float
    f_res1 = integer_to_float(res[0], res[1]);
    f_res2 = integer_to_float(res[2], res[3]);
    f_res3 = integer_to_float(res[4], res[5]);

    //Serial.println("Float 1: ");
    //Serial.println(f_res1, 3);

    //Serial.println("Float 2: ");
    //Serial.println(f_res2, 3);

    //Serial.println("Float 3: ");
    //Serial.println(f_res3, 3);

    serialTransparente.println(primeiroByteDados);
    delay(2000);
    serialTransparente.println(segundoByteDados);
    delay(2000);
    serialTransparente.println(terceiroByteDados);
    delay(2000);
    serialTransparente.println(quartoByteDados);
    delay(2000);
    serialTransparente.println(quintoByteDados);
    delay(2000);
    serialTransparente.println(sextoByteDados);
    delay(2000);
  }
}

/*

#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define SLAVE_ID 1
#define FIRST_REG 15
#define REG_COUNT 2
#define D1 26
#define D2 25

int primeiroByteDados;
int segundoByteDados;

SoftwareSerial serialTransparente(18, 19); // RX - TX
SoftwareSerial S(D1, D2);
ModbusRTU mb;

bool cb(Modbus::ResultCode event, uint16_t transactionId, void *data)
{ // Callback to monitor errors
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
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(27, OUTPUT);

  Serial.begin(9600);
  S.begin(9600, SWSERIAL_8N1);
  serialTransparente.begin(9600);
  mb.begin(&S);
  mb.master();
  Serial.print("Terminou o setup");
}

void loop()
{
  digitalWrite(12, HIGH);
  delay(250);
  digitalWrite(12, LOW);
  delay(250);

  digitalWrite(14, HIGH);
  delay(250);
  digitalWrite(14, LOW);
  delay(250);

  digitalWrite(27, HIGH);

  serialTransparente.println("teste");
  delay(2000);

  uint16_t res[REG_COUNT];
  float f_res;

  if (!mb.slave())
  {                                                       // Check if no transaction in progress
    mb.readIreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, cb); // Send Read Hreg from Modbus Server
    while (mb.slave())
    { // Check if transaction is active
      mb.task();
      delay(10);
    }
    Serial.println();
    Serial.print("Primeiro Byte: ");
    // Serial.println(res[0], HEX);
    primeiroByteDados = res[0];
    Serial.println(primeiroByteDados);

    Serial.print("Segundo Byte: ");
    // Serial.println(res[1], HEX);
    segundoByteDados = res[1];
    Serial.println(segundoByteDados);

    f_res = integer_to_float(res[0], res[1]);
    Serial.println("Float: ");
    Serial.println(f_res, 3);

    // serialTransparente.println(primeiroByteDados);
    delay(7000);
    // serialTransparente.println("teste");
  }
  delay(7000);
}

*/