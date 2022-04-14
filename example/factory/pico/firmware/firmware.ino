#include <stdio.h>
#include "TFT_eSPI.h"  // https://github.com/Bodmer/TFT_eSPI
#include <WiFiEspAT.h> // https://github.com/jandrassy/WiFiEspAT
#include "OneButton.h" // https://github.com/mathertel/OneButton

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#include "pin_config.h"
#include "picoImage.h"

OneButton button_IO6(PIN_BOTTON1, true);
OneButton button_IO7(PIN_BOTTON2, true);

/* 
Select Setup135_ST7789.h from the TFT library.
Overwrite the pin definition in pin_config.h to the pin
*/
TFT_eSPI tft = TFT_eSPI(135, 240);

const float conversion_factor = 3.3f / (1 << 12);
#define VOLT_COMPENSATOR 0.05
bool isTX = false;

void setup()
{
  Serial.begin(115200);

  /* Open the power supply */
  pinMode(PIN_PWR_ON, OUTPUT);
  digitalWrite(PIN_PWR_ON, HIGH);

  /* Configure pins for communication with ESP32C3-AT */
  Serial2.setTX(ESP32C3_TX_PIN);
  Serial2.setRX(ESP32C3_RX_PIN);
  Serial2.begin(115200);

  /* Configure battery voltage detection pin */
  adc_init();
  adc_gpio_init(PIN_BAT_VOLT);
  adc_select_input(0);

  /* Bind keystroke callback event */
  button_IO6.attachClick(io6_click_event_cb);
  button_IO7.attachClick(io7_click_event_cb);

  /* Configure LED GPIO pins */
  pinMode(PIN_RED_LED, OUTPUT);

  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, 0);

  /* Initialize the screen */
  tft.init();
  tft.fillScreen(TFT_BLACK); // Clear the screen junk
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, 240, 135, pico);

  /* Gradually lighten up */
  analogWrite(PIN_TFT_BL, 0);
  for (int i = 0; i < 0xff; i++)
  {
    delay(5);
    analogWrite(PIN_TFT_BL, i);
  }
  delay(1000);

  wifi_test();
}

void loop()
{
  blink();
  button_IO6.tick();
  button_IO7.tick();
}

void io6_click_event_cb()
{
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  Serial.println();
  Serial.println("Scanning available networks...");
  tft.println("Scanning available networks...");
  listNetworks();
}

void io7_click_event_cb()
{ // Display battery voltage
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);

  float volt = (2 * adc_read() * conversion_factor) + VOLT_COMPENSATOR;
  tft.printf("Battery volt: %0.2f\n", volt);
  Serial.printf("Battery volt: %0.2f\n", volt);
  tft.setTextSize(1);
}

void blink(void)
{
  static uint32_t MILLIS, rad;
  if (millis() - MILLIS > rad)
  {
    digitalWrite(PIN_RED_LED, !digitalRead(PIN_RED_LED));
    MILLIS = millis();
    rad = random(1, 500);
  }
}

void wifi_test()
{
  uint32_t overtime = 0;
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(0, 0);
  tft.println("Hello Pico and ESP32C3");
  Serial.println("Hello Pico and ESP32C3");
  bool isAT_Program = Send_AT_CMD("AT");
  if (isAT_Program)
  {
    Serial.println("AT success");
    tft.println("Scanning available networks...");
    /* Scanning the WIFI */
    WiFi.init(Serial2);

    if (WiFi.status() == WL_NO_MODULE)
    {
      Serial.println();
      Serial.println("Communication with WiFi module failed!");
      // don't continue
      while (true)
        ;
    }
    WiFi.disconnect();    // to clear the way. not persistent
    WiFi.setPersistent(); // set the following WiFi connection as persistent
    WiFi.endAP();         // to disable default automatic start of persistent AP at startup

    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC: ");
    printMacAddress(mac);

    // scan for existing networks:
    Serial.println();
    Serial.println("Scanning available networks...");
    listNetworks();

    Serial.println("Connecting to the network...");
    tft.println("Connecting to the network...");

    Serial.print("Attempting to connect to WPA SSID: ");
    tft.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    tft.println(ssid);

    int status;
    overtime = millis();
    while (status != WL_CONNECTED)
    {
      status = WiFi.begin(ssid, pass);
      Serial.print(".");
      tft.print(".");
      delay(500);

      if (millis() - overtime > 5000)
      {
        overtime = 0;
        break;
      }
    }

    Serial.println("You're connected to the network");
    printWifiData();
    Serial.println("Try to connect \"https://www.baidu.com\"");
    tft.println("Try to connect \"https://www.baidu.com\"");

    Serial2.println("AT+HTTPCGET=\"https://www.baidu.com/\"");
    overtime = millis();
    while (!Serial2.available())
    {
      if (millis() - overtime > 2000)
      {
        overtime = 0;
        break;
      }
    }

    String acc = "OK\r\n";
    String ack = "";
    overtime = millis();
    do
    {
      ack += Serial2.readString();
      if (millis() - overtime > 2000)
      {
        overtime = 0;
        break;
      }
    } while (!ack.endsWith(acc));

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.println(ack.c_str());
  }
  else
  {
    Serial.println("AT fail");
    tft.println("The ESP32C3 communication fails..");
  }
}

bool Send_AT_CMD(const char *cmd)
{
  Serial2.println(cmd);
  String acc = "OK\r\n";
  String ack = "";
  uint32_t smap = millis() + 100;
  while (millis() < smap)
  {
    if (Serial2.available() > 0)
    {
      ack += Serial2.readString();
      if (ack.endsWith(acc))
      {
        return true;
      }
    }
  }
  return false;
}

void printWifiData()
{
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  tft.print("IP Address: ");
  tft.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void listNetworks()
{
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1)
  {
    Serial.println("Couldn't get a WiFi connection");
    while (true)
      ;
  }

  // print the list of networks seen:
  Serial.print("number of available networks: ");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++)
  {
    Serial.print(thisNet + 1);
    Serial.print(") ");
    Serial.print("Signal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tChannel: ");
    Serial.print(WiFi.channel(thisNet));
    byte bssid[6];
    Serial.print("\t\tBSSID: ");
    printMacAddress(WiFi.BSSID(thisNet, bssid));
    Serial.print("\tEncryption: ");
    printEncryptionType(WiFi.encryptionType(thisNet));
    Serial.print("\t\tSSID: ");
    Serial.println(WiFi.SSID(thisNet));
    Serial.flush();

    tft.printf("[%d]RSSI: %ddBm SSID: %s", thisNet, WiFi.RSSI(thisNet), WiFi.SSID(thisNet));
    tft.println();
  }
  Serial.println();
  tft.println("end of scan");
}

void printEncryptionType(int thisType)
{
  // read the encryption type and print out the name:
  switch (thisType)
  {
  case ENC_TYPE_WEP:
    Serial.print("WEP");
    break;
  case ENC_TYPE_TKIP:
    Serial.print("WPA");
    break;
  case ENC_TYPE_CCMP:
    Serial.print("WPA2");
    break;
  case ENC_TYPE_NONE:
    Serial.print("None");
    break;
  case ENC_TYPE_AUTO:
    Serial.print("Auto");
    break;
  case ENC_TYPE_UNKNOWN:
  default:
    Serial.print("Unknown");
    break;
  }
}

void printMacAddress(byte mac[])
{
  for (int i = 5; i >= 0; i--)
  {
    if (mac[i] < 16)
    {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0)
    {
      Serial.print(":");
    }
  }
  Serial.println();
}