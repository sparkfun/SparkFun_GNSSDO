/*
  SparkFun GNSSDO Test Sketch

  License: MIT. Please see LICENSE.md for more details

  ESP32-WROVER-IE Pin Allocations:
  D0  : Boot + Boot Button
  D1  : Serial TX (CH340 RX)
  D2  : N/C
  D3  : Serial RX (CH340 TX)
  D4  : Serial RX (mosaic-T COM4 TX)
  D5  : N/C
  D12 : N/C
  D13 : Serial RX (mosaic-T COM1 TX)
  D14 : Serial TX (mosaic-T COM1 RX)
  D15 : N/C
  D16 : N/C
  D17 : N/C
  D18 : I2C SDA2 (SiT5358)
  D19 : I2C SCL2 (SiT5358)
  D21 : I2C SDA (OLED)
  D22 : I2C SCL (OLED)
  D23 : N/C
  D25 : Serial TX (mosaic-T COM4 RX)
  D26 : Serial CTS (mosaic-T COM1 CTS)
  D27 : Serial RTS (mosaic-T COM1 RTS)
  D32 : Error LED
  D33 : Lock LED
  A34 : N/C
  A35 : Device Sense (resistor divider)
  A36 : MRDY (mosaic-T module ready)
  A39 : N/C
*/

const int LED_ERROR = 32;
const int LED_LOCK = 33;
const int SCL_2 = 19;
const int SDA_2 = 18;

void setup()
{
  pinMode(LED_ERROR, OUTPUT);
  pinMode(LED_LOCK, OUTPUT);
}

void loop()
{
  digitalWrite(LED_ERROR, HIGH);
  digitalWrite(LED_LOCK, LOW);
  delay(500); // At 1Hz
  digitalWrite(LED_ERROR, LOW);
  digitalWrite(LED_LOCK, HIGH);
  delay(500); // At 1Hz
}
