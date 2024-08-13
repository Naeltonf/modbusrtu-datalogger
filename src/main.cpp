#include <ModbusRTU.h>
#include <SoftwareSerial.h>

/*
*****Relação Endereço de Registro x Al(mA) *****
FIRST_REG 107 = 09
FIRST_REG 108 = 10
FIRST_REG 109 = 11
FIRST_REG 110 = 12
*/
#define SLAVE_ID 1
#define FIRST_REG 110
#define REG_COUNT 1
#define D1 5
#define D2 4

int primeiroByteDados;
int segundoByteDados;

SoftwareSerial S(D1, D2);
ModbusRTU mb;

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Callback to monitor errors
  if (event != Modbus::EX_SUCCESS) {
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

int lerModbus()
{
  uint16_t res[REG_COUNT];
  float f_res;

  if (!mb.slave()) {    // Check if no transaction in progress
    mb.readIreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, cb); // Send Read Hreg from Modbus Server
    while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
    }
    Serial.println();
    Serial.print("Primeiro Byte: ");
    //Serial.println(res[0], HEX);
    primeiroByteDados = res[0];
    Serial.println(primeiroByteDados);

    // Serial.print("Segundo Byte: ");
    // Serial.println(res[1], HEX);
    // segundoByteDados = res[1];
    // Serial.println(segundoByteDados);

    // f_res = integer_to_float(res[0],res[1]);
    // Serial.println("Float: ");
    // Serial.println(f_res,3);

  }
  delay(1000);

  return primeiroByteDados;
}

void imprimirRetorno(int ret)
{
  Serial.print("O VALOR MODBUS: ");
  Serial.println(ret);
}

void setup() {
  Serial.begin(9600);
  S.begin(9600, SWSERIAL_8N1);
  mb.begin(&S);
  mb.master();
  Serial.print("Terminou o setup");
}

void loop() {
  int retorno = lerModbus();

  imprimirRetorno(retorno);
}

