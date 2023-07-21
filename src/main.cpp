#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#define startTDS

//---------------Sensor Suhu---------------
OneWire pinSuhu(4);
DallasTemperature sensorSuhu(&pinSuhu);
float temperatureC;

void settingSuhu(){
  sensorSuhu.requestTemperatures(); 
  temperatureC = sensorSuhu.getTempCByIndex(0);
}
//---------------Sensor Suhu---------------

//---------------Sensor TDS---------------
#define pinTDS 35   
#define VREF 3.3    //ADC esp32
#define SCOUNT 30   //Jumlah sample      
int analogBuffer[SCOUNT], analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0;

int getMedianNum(int bArray[], int iFilterLen) 
{
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) 
  {
    for (i = 0; i < iFilterLen - j - 1; i++) 
    {
      if (bTab[i] > bTab[i + 1]) 
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

void settingTDS(){
    //membaca nilai analog TDS setiap 40 milliseconds kemudian disimpan pada buffer
    static unsigned long analogSampleTimepoint = millis();
    if(millis()-analogSampleTimepoint > 40U)
    {
      analogSampleTimepoint = millis();
      analogBuffer[analogBufferIndex] = analogRead(pinTDS);
      analogBufferIndex++;
        if(analogBufferIndex == SCOUNT) 
          analogBufferIndex = 0;
    }   
    
    static unsigned long printTimepoint = millis();
    if(millis()-printTimepoint > 800U)
    {
      printTimepoint = millis();
      for(copyIndex=0; copyIndex<SCOUNT; copyIndex++)
          analogBufferTemp[copyIndex]= analogBuffer[copyIndex];

      averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4096.0; 
      float compensationCoefficient = 1.0+0.02*(temperatureC-25.0);

      float compensationVoltage = averageVoltage/compensationCoefficient;  //temperature compensation
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5; 
    }
}
// ---------------Sensor TDS---------------

// ------------------LCD-------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);  //LCD I2C
String messageToScroll = "WATER QUALITY MONITORING SYSTEM";
void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}

void openingLCD(){
  lcd.init();                   
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("DELTA MINA");
  scrollText(1, messageToScroll, 500, 16);
  delay(1000);
  lcd.clear(); 
}
// ------------------LCD-------------------

void setup() {
  Serial.begin(115200);
  sensorSuhu.begin();
  pinMode(pinTDS, INPUT);
  openingLCD();
}

void loop() {
  settingSuhu();
  settingTDS();

  Serial.print(temperatureC);
  Serial.println("ºC");
  Serial.print("TDS Value :");
  Serial.print(tdsValue);
  Serial.println(" ppm");
}