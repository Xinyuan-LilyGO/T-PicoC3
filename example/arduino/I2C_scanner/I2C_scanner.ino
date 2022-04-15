#include <Wire.h>

#define I2C_SDA 12
#define I2C_SCL 13

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("I2C scanner. Scanning ...SDA 12 SCL 13");
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();
  delay(500);
}

void loop()
{
  // put your main code here, to run repeatedly:
  uint8_t count = 0;
  for (uint8_t i = 1; i < 127; i++)
  {
    Wire.beginTransmission(i);
    // Serial.print(i);
    Serial.print(".");
    if (Wire.endTransmission() == 0)
    {
      Serial.println();
      Serial.print("Found address: ");
      Serial.print(i, DEC);
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.println(")");
      count++;
      delay(100); // maybe unneeded?
    }             // end of good response
  }               // end of for loop
  Serial.println();
  Serial.print("Done. ");
  Serial.print("Found ");
  Serial.print(count, DEC);
  Serial.println(" device(s).");
  delay(4000);
}
