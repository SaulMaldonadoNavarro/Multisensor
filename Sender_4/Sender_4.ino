#include <SPI.h>
#include <LoRa.h>
#include "DHT.h"

//Pines de conexión
#define ss 5
#define rst 14
#define dio0 2
#define MQ_PIN 34
#define RL_VALUE 1
#define RO_CLEAN_AIR_FACTOR 9.83
#define CALIBARAION_SAMPLE_TIMES 50
#define CALIBRATION_SAMPLE_INTERVAL 500
#define READ_SAMPLE_INTERVAL 50
#define READ_SAMPLE_TIMES 5
#define GAS_CO 1
#define GAS_SMOKE 2
#define DHTPIN 27
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);

float h;
float t;
float f;
float hi;
float hi2;
float COCurve[3] = {2.3,0.72,-0.34};
float SmokeCurve[3] ={2.3,0.53,-0.44};
float Ro = 10;

void setup() {
  Serial.begin(9600);
  Serial.print("Calibrando sensores...\n"); 

  //setup DHT
  dht.begin();
  Serial.println("DHT...OK!");

  // setup MQ
  Ro = MQCalibration(MQ_PIN);
  Serial.print("MQ...OK!");
  Serial.print(" Ro = ");
  Serial.println(Ro);

  //setup LoRa
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(915E6)) {
    Serial.println("Error al iniciar!");
    while (1);
  }
  // contraseña
  LoRa.setSyncWord(0xF3);
  Serial.println("Enviando LoRa...");
  

}

void loop (){
  h = dht.readHumidity();
  t = dht.readTemperature();
  f = dht.readTemperature(true);
  
  
  while (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Error al leer el sensor DHT...");
    delay(2500);
    h = dht.readHumidity();
    t = dht.readTemperature();
    f = dht.readTemperature(true);
  }

  hi = dht.computeHeatIndex(f, h);
  hi2 = (hi - 32)*(0.5555555556);
  /*
  Serial.print(t);
  Serial.print(", ");
  Serial.print(h);
  Serial.print(", ");
  Serial.print(hi2);
  Serial.print(", ");
  Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO) );
  Serial.print(", ");
  Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE) );
  Serial.println();*/

  
  LoRa.beginPacket();
  LoRa.print(t);
  LoRa.print(",");
  LoRa.print(h);
  LoRa.print(",");
  LoRa.print(hi2);
  LoRa.print(",");
  LoRa.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO) );
  LoRa.print(",");
  LoRa.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE) );
  LoRa.println();
  LoRa.endPacket();

  delay(2000);
  
  }
  
void Temperatura() {
  h = dht.readHumidity();
  t = dht.readTemperature();
  f = dht.readTemperature(true);
  
  
  while (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Error al leer el sensor DHT...");
    delay(2500);
    h = dht.readHumidity();
    t = dht.readTemperature();
    f = dht.readTemperature(true);
  }

  hi = dht.computeHeatIndex(f, h);
  hi2 = (hi - 32)*(0.5555555556);
}

float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}

float MQCalibration(int mq_pin)
{
  int i;
  float val=0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value

  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 

  return val; 
}

float MQRead(int mq_pin)
{
  int i;
  float rs=0;

  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs/READ_SAMPLE_TIMES;

  return rs;  
}

int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  
   if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }    

  return 0;
}

int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
