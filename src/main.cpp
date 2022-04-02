#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "tuonet-iot"; // wifi credentials
const char *password = "J6NR-eEb6";
String serverName = "https://bme.vsb.cz/api/iot"; // url of database server
String hashID = "9be97ecec52a64dd8cea21c3205525c2";
String macAddress = "";
//atleast 100 hz
//adcvalue increase atleast 200
//
int ADCpin = 34;
int ADCvalue[20];
int period = 100;
int fs = 1000 / period;
int currentMillis = 0;
int previousMillis = 0;
int idx = 0;
char str[128];

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);                  // Station mode for WiFi
  WiFi.begin(ssid, password);           // connect to selected WiFi
  while (WiFi.status() != WL_CONNECTED) // Check if WiFi is connected
  {
    Serial.print("* ");
    delay(100);
  }
  Serial.println(WiFi.localIP()); // show obtained IP address
  macAddress = WiFi.macAddress(); // get MAC address of ESP32
  Serial.println(macAddress);
}

void loop()
{
  // put your main code here, to run repeatedly:
  currentMillis = millis();
  if (currentMillis - previousMillis >= period)
  {
    previousMillis = currentMillis;
    ADCvalue[idx] = analogRead(ADCpin);
    Serial.println(ADCvalue[idx]);
    idx++;
    if (idx == 20)
    {
      idx = 0;
      if (WiFi.status() == WL_CONNECTED) // check if WiFi is still connected
      {
        HTTPClient http;        // create http client object for future use
        http.begin(serverName); // begin the http communication with server

        int strindex = 0;
        for (int i = 0; i < 19; i++) // zmena z i<20 na i<19, poslední (20. prvek) pole vepiseme manuálně
        {
          strindex += sprintf(&str[strindex], "%d,", ADCvalue[i]);
        }
        strindex += sprintf(&str[strindex], "%d", ADCvalue[19]); // manualni vypsani posledního prvku, ať za ním nemáme tu čárku ("%d")
        Serial.println(str);

        http.addHeader("Content-Type", "application/json"); // change header of request to JSON
        http.POST("{\"id\":\"" + macAddress +
                  "\",\"password\":\"" +
                  hashID +
                  "\",\"tag\":\"ECG\",\"data\":{\"fs\":" +
                  String(fs) +
                  ",\"samples\":[" +
                  str +
                  "]}}"); // send data to our database server
        Serial.println("done");
      }
    }
  }
}