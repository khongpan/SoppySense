#include "ArduinoHttpClient.h"

// Select  modem:
#define TINY_GSM_MODEM_SIM800

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 38400

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

/*
 * Test enabled
 */
#define TINY_GSM_TEST_GPRS true
#define TINY_GSM_TEST_WIFI false
#define TINY_GSM_TEST_CALL false
#define TINY_GSM_TEST_SMS false
#define TINY_GSM_TEST_USSD false
#define TINY_GSM_TEST_BATTERY false
// powerdown modem after tests
#define TINY_GSM_POWERDOWN false

// set GSM PIN, if any
#define GSM_PIN ""

// Set phone numbers, if you want to test SMS and Calls
//#define SMS_TARGET  "+380xxxxxxxxx"
//#define CALL_TARGET "+380xxxxxxxxx"

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";
const char wifiSSID[]  = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

//Server details
 const char server[] = "agritronics.nstda.or.th";
 const char resource[] = "/webpost0606/log.php?data1=NODEMCU_002,1000,A,19/02/28,11:36:00,7,10,1,2,3,4";
 const int  port = 80;
 TinyGsmClient client(modem);
 HttpClient http(client, server, port);

void test_http() {

   SerialMon.print(F("Performing HTTP GET request... "));
  int err = http.get(resource);
  if (err != 0) {
    SerialMon.println(F("failed to connect"));
    delay(10000);
    return;
  }

  int status = http.responseStatusCode();
  SerialMon.print(F("Response status code: "));
  SerialMon.println(status);
  if (!status) {
    delay(10000);
    return;
  }

  SerialMon.println(F("Response Headers:"));
  while (http.headerAvailable()) {
    String headerName = http.readHeaderName();
    String headerValue = http.readHeaderValue();
    SerialMon.println("    " + headerName + " : " + headerValue);
  }

  int length = http.contentLength();
  if (length >= 0) {
    SerialMon.print(F("Content length is: "));
    SerialMon.println(length);
  }
  if (http.isResponseChunked()) {
    SerialMon.println(F("The response is chunked"));
  }

  String body = http.responseBody();
  SerialMon.println(F("Response:"));
  SerialMon.println(body);

  SerialMon.print(F("Body length is: "));
  SerialMon.println(body.length());

  // Shutdown

  http.stop();
  SerialMon.println(F("Server disconnected"));

}

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // Set your reset, enable, power pins here
  pinMode(20, OUTPUT);
  digitalWrite(20, HIGH);

  pinMode(23, OUTPUT);
  digitalWrite(23, HIGH);

  DBG("Wait...");

  // Set GSM module baud rate
  TinyGsmAutoBaud(SerialAT,GSM_AUTOBAUD_MIN,GSM_AUTOBAUD_MAX);
  //SerialAT.begin(9600);
  delay(3000);
}

void loop() {

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  DBG("Initializing modem...");
  //if (!modem.restart()) {
  if (!modem.init()) {
    DBG("Failed to restart modem, delaying 10s and retrying");
    delay(3000);
    // restart autobaud in case GSM just rebooted
    TinyGsmAutoBaud(SerialAT,GSM_AUTOBAUD_MIN,GSM_AUTOBAUD_MAX);
    delay(10000);
    return;
  }

  String name = modem.getModemName();
  DBG("Modem Name:", name);

  String modemInfo = modem.getModemInfo();
  DBG("Modem Info:", modemInfo);

#if TINY_GSM_TEST_GPRS
  // Unlock your SIM card with a PIN if needed
  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
    modem.simUnlock(GSM_PIN);
  }
#endif

#if TINY_GSM_TEST_WIFI
  DBG("Setting SSID/password...");
  if (!modem.networkConnect(wifiSSID, wifiPass)) {
    DBG(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");
#endif

#if TINY_GSM_TEST_GPRS && defined TINY_GSM_MODEM_XBEE
  // The XBee must run the gprsConnect function BEFORE waiting for network!
  modem.gprsConnect(apn, gprsUser, gprsPass);
#endif

  DBG("Waiting for network...");
  if (!modem.waitForNetwork()) {
    delay(10000);
    return;
  }

  if (modem.isNetworkConnected()) {
    DBG("Network connected");
  }

#if TINY_GSM_TEST_GPRS
  DBG("Connecting to", apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    delay(10000);
    return;
  }

  bool res = modem.isGprsConnected();
  DBG("GPRS status:", res ? "connected" : "not connected");

  String ccid = modem.getSimCCID();
  DBG("CCID:", ccid);

  String imei = modem.getIMEI();
  DBG("IMEI:", imei);

  String cop = modem.getOperator();
  DBG("Operator:", cop);

  IPAddress local = modem.localIP();
  DBG("Local IP:", local);

  int csq = modem.getSignalQuality();
  DBG("Signal quality:", csq);

#ifdef TINY_GSM_MODEM_SIM800
  // This is only supported on SIMxxx series
   String gsmLoc = modem.getGsmLocation();
   DBG("GSM location:", gsmLoc);

  // This is only supported on SIMxxx series
  String gsmTime = modem.getGSMDateTime(DATE_TIME);
  DBG("GSM Time:", gsmTime);
  String gsmDate = modem.getGSMDateTime(DATE_DATE);
  DBG("GSM Date:", gsmDate);

  String ussd_balance = modem.sendUSSD("*111#");
  DBG("Balance (USSD):", ussd_balance);

  String ussd_phone_num = modem.sendUSSD("*161#");
  DBG("Phone number (USSD):", ussd_phone_num);
#endif
#endif //if

#if defined(TINY_GSM_MODEM_HAS_GPS)
  modem.enableGPS();
  String gps_raw = modem.getGPSraw();
  modem.disableGPS();
  DBG("GPS raw data:", gps_raw);
#endif

#if TINY_GSM_TEST_SMS && defined(SMS_TARGET)
  res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
  DBG("SMS:", res ? "OK" : "fail");

  // This is only supported on SIMxxx series
  res = modem.sendSMS_UTF16(SMS_TARGET, u"Привіііт!", 9);
  DBG("UTF16 SMS:", res ? "OK" : "fail");
#endif

#if TINY_GSM_TEST_CALL && defined(CALL_TARGET)
  DBG("Calling:", CALL_TARGET);

  // This is NOT supported on M590
  res = modem.callNumber(CALL_TARGET);
  DBG("Call:", res ? "OK" : "fail");

  if (res) {
    delay(1000L);

    // Play DTMF A, duration 1000ms
    modem.dtmfSend('A', 1000);

    // Play DTMF 0..4, default duration (100ms)
    for (char tone='0'; tone<='4'; tone++) {
      modem.dtmfSend(tone);
    }

    delay(5000);

    res = modem.callHangup();
    DBG("Hang up:", res ? "OK" : "fail");
  }
#endif

#if TINY_GSM_TEST_BATTERY
  uint8_t chargeState = -99;
  int8_t percent = -99;
  uint16_t milliVolts = -9999;
  modem.getBattStats(chargeState, percent, milliVolts)
  DBG("Battery charge state:", chargeState);
  DBG("Battery charge 'percent':", percent);
  DBG("Battery voltage:", milliVolts / 1000.0F);

  float temp = modem.getTemperature();
  DBG("Chip temperature:", temp);
#endif


  test_http();

#if TINY_GSM_TEST_GPRS
  modem.gprsDisconnect();
  if (!modem.isGprsConnected()) {
    DBG("GPRS disconnected");
  } else {
    DBG("GPRS disconnect: Failed.");
  }
#endif

#if TINY_GSM_TEST_WIFI
  modem.networkDisconnect();
  DBG("WiFi disconnected");
#endif

#if TINY_GSM_POWERDOWN
  // Try to power-off (modem may decide to restart automatically)
  // To turn off modem completely, please use Reset/Enable pins
  modem.poweroff();
  DBG("Poweroff.");
#endif

  // Do nothing forevermore
  while (true) {
    modem.maintain();
  }
}