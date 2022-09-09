#include <WiFi.h>
#include <HTTPClient.h>
#define NAQC 7
#define OVERSMP 128

const char* Wifi_ssid = "3M-GFUnpad";
const char* Wifi_password = "haruman3mgf";

//Your Domain name with URL path or IP address with path
const char* serverName = "https://dock.unpad.ac.id/aggr-435dc0e7/DataSubSoil";
String apikey = "aggr-435dc0e7/DataSubSoil";
String devid = "DataSoil-1";
float lat= -7.137237;
float lon= 107.582877;
String data="";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Set timer to 1 hour (3600s=3600000ms)
unsigned long timerDelay = 3600000;

static const uint8_t porouspot_pins[] = {37,38,39,34,35,32,33};
uint8_t Ground = 36;

float val[NAQC];

void FetchData() {
  int ovr;
  int tmp;
  float ts;
  
  analogSetAttenuation(ADC_2_5db); //set adc read to max 1.1 v (4094.2 in adc) make high resolution
  // Setup pins for input
  for(int i=0;i<NAQC;i++) {
    float Gndvoltage = (float)analogRead(Ground) / 4095 * 1.1;
    float Porvoltage = (float)analogRead(porouspot_pins[i]) / 4095 * 1.1;
    tmp = Gndvoltage-Porvoltage;
    if(tmp<0) tmp = -tmp;
    //val[i]=tmp;
    data+=String(tmp+",");
    delay(2000); //TimeSpace between data acq (2s)
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(Wifi_ssid, Wifi_password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("It will take 17 seconds before publishing the first reading.");
}

void loop() {
  //Send an HTTP POST request every 10 minutes
  if(data=="") FetchData() ;
  else{
      //Check WiFi connection status
      if(WiFi.status()== WL_CONNECTED){
        WiFiClient client;
        HTTPClient http;
      
        // Your Domain name with URL path or IP address with path
        http.begin(client, serverName);
  
        // Specify content-type header
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        // Data to send with HTTP POST
        String httpRequestData = "api_key="+apikey+"&dev="+devid+"&pos="+String(lon)+","+String(lat)+"&data="+data;           
        // Send HTTP POST request
        int httpResponseCode = http.POST(httpRequestData);
        
        if (httpResponseCode==HTTP_CODE_OK || httpResponseCode==HTTP_CODE_MOVED_PERMANENTLY) {
         Serial.println(http.getString());
        } else {
         Serial.println("Request Failure, Something Wrong with HTTP Request");
        }
        // Free resources
        http.end();
        data="";
        Serial.println("I'm awake, but I'm going into deep sleep mode for 1 hour");
        ESP.deepSleep(3600e6);
      }
      else {
        Serial.println("WiFi Disconnected");
        ESP.restart();
      }
  }
}
