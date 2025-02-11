/*
 * GSMSim Library
 * 
 * This library written for SIMCOM modules. Tested on Sim800L. Library may worked on any SIMCOM and another GSM modules and GSM Shields. Some AT commands for only SIMCOM modules.
 *
 * Created 11.05.2017
 * By Erdem ARSLAN
 * Modified 06.05.2020
 *
 * Version: v.2.0.1
 *
 * Erdem ARSLAN
 * Science and Technology Teacher, an Arduino Lover =)
 * erdemsaid@gmail.com
 * https://www.erdemarslan.com/
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

#ifndef __GSMSim_H__
#define __GSMSim_H__

#include <Arduino.h>

// DEFAULT PIN DEFINATIONS IN HERE
#define DEFAULT_RST_PIN 2
#define DEFAULT_LED_PIN 13
#define DEFAULT_LED_FLAG true

#define BUFFER_RESERVE_MEMORY 255
#define TIME_OUT_READ_SERIAL 5000

// PIN (PERSONAL IDENTIFICATION NUMBER) STATUS
#define PIN_READY 0
#define SIM_PIN 1
#define SIM_PUK 2
#define PH_SIM_PIN 3
#define PH_SIM_PUK 4
#define SIM_PIN2 5
#define SIM_PUK2 6
#define PIN_STATUS_UNKNOWN 7

// CALL STATUS
#define READY 0
#define UNKNOWN 2
#define RINGING 3
#define IN_CALL 4
#define NOT_READ 99

struct SMSStruct
{
  unsigned int id;
  bool error;
  String folder, status, phoneno, date, message;
};

struct SMSArray
{
  SMSStruct *smses;
  int count;
};

struct GSMStatus
{
  byte battchg, signal, service, message, call, roam, smsfull;
  bool error;
};

struct PhoneBookEntry{
	byte id,type;
	String phoneno,name;
	bool error;
};

class GSMSim
{
protected:
  Stream &gsm;
  unsigned int _timeout;
  String _buffer;
  void _readSerial();
  void _readSerial(uint32_t timeout);

public:
  unsigned int RESET_PIN;
  unsigned int LED_PIN;
  bool LED_FLAG;
  // Sınıfı Başlatıcı...
  GSMSim(Stream &s) : gsm(s)
  {
    RESET_PIN = DEFAULT_RST_PIN;
    LED_PIN = DEFAULT_LED_PIN;
    LED_FLAG = DEFAULT_LED_FLAG;
  }

  GSMSim(Stream &s, unsigned int resetPin) : gsm(s)
  {
    RESET_PIN = resetPin;
    LED_PIN = DEFAULT_LED_PIN;
    LED_FLAG = DEFAULT_LED_FLAG;
  }

  GSMSim(Stream &s, unsigned int resetPin, unsigned int ledPin, bool ledFlag) : gsm(s)
  {
    RESET_PIN = resetPin;
    LED_PIN = ledPin;
    LED_FLAG = ledFlag;
  }

  // Init GSMSim
  void init();

  GSMStatus getStatus();

  // Reset Module
  void reset();

  bool checkConnection(int retries = 0);

  String sendATCommand(char *command);
  // Telefon fonksiyonunu ayarla
  bool setPhoneFunc(int level);
  // Sinyal kalitesi
  unsigned int signalQuality();
  // operatöre kayıtlı mı
  bool isRegistered();
  // sim kart takılı mı?
  bool isSimInserted();
  // Pin durumu
  unsigned int pinStatus();
  // enter pin code
  bool enterPinCode(char *pinCode);
  bool enterPukCode(char *pukCode,char *pinCode);
  bool changePinCode(char *oldCode,char *newCode);
  // enable pin code
  bool enablePinCode(char *pinCode);
  // disable pin code
  bool disablePinCode(char *pinCode);
  int getPinStatus();
  // operatör ismi
  String operatorName();
  // Sim kartta kayıtlı operatör ismi
  String operatorNameFromSim();
  // Telefon durumu
  unsigned int phoneStatus();
  // echo kapalı
  bool echoOff();
  // echo açıkı
  bool echoOn();
  // modül üreticisi
  String moduleManufacturer();
  // modül modeli
  String moduleModel();
  // modül revizyon
  String moduleRevision();
  // modül imei
  String moduleIMEI();
  // modül imei değiştirme
  bool moduleIMEIChange(char *imeino);
  // modül sim no
  String moduleIMSI();
  // modül sim operatör no
  String moduleICCID();
  // zil volümü
  unsigned int ringerVolume();
  // zil seviyesini ayarlar
  bool setRingerVolume(unsigned int level);
  // speaker düzeyi
  unsigned int speakerVolume();
  // speaker düzeyini ayarla
  bool setSpeakerVolume(unsigned int level);
  // debug modu - verbose mode
  String moduleDebug();
  // Ayarları cihaz üzerine kaydeder.
  bool saveSettingsToModule();
  bool setBaudrate(int rate);

  // SMS Fonksiyonları
  bool initSMS();
  // sms i text yada pdu moda döndürür
  bool setTextMode(bool textModeON);
  // sms için kayıt kaynağı seçer
  bool setPreferredSMSStorage(char *mem1, char *mem2, char *mem3);
  bool getPreferredSMSStorage(int *arr);
  // yeni mesajı <mem>,<smsid> şeklinde geri dönmesi için ayarlar...
  bool setNewMessageIndication();
  // charseti ayarlar...
  bool setCharset(char *charset);
  // sms gönderir
  bool send(char *number, char *message);
  // okunmamış mesaj listesi
  int list(void (*cb)(SMSStruct sms), bool onlyUnread);
  // indexi verilen mesajı oku
  SMSStruct read(unsigned int index);
  // indexi verilen mesajı oku
  SMSStruct read(unsigned int index, bool markRead);
  // verilen indexteki mesajı göndereni söyler...
  String getSenderNo(unsigned int index);
  // serialden direk mesajı oku -> serialden gelen veri verilmeli
  SMSStruct readFromSerial(String serialRaw);
  // serialden gelen sms bilgisinin indexini ver
  unsigned int indexFromSerial(String serialRaw);
  // mesaj merkezini öğren
  String readMessageCenter();
  // mesaj merkezini değiştir
  bool changeMessageCenter(char *messageCenter);
  // mesajı sil
  bool deleteOne(unsigned int index);
  // tüm okunmuşları sil
  bool deleteAllRead();
  // tüm mesajları sil
  bool deleteAll();

  // Arama Fonksiyonları
  bool initCall();
  // arama yapar
  bool call(char *phone_number);
  bool call(String *phone_number);
  // arama cevaplar
  bool answer();
  // aramayı sonlandırır
  bool hangoff();
  // arama durumu
  int status();
  bool setCLIP(bool active);
  bool setCLIR(bool active);
  // COLP u aktif veya pasif yapar
  bool setCOLP(bool active);
  // COLP aktif mi?
  bool isCOLPActive();
  // Arayanı söyleme aktif mi değil mi?
  bool showCurrentCall(bool active);
  // şimdi arayanı söyle
  String readCurrentCall(String serialRaw);
  // enable or disable reject all calls
  bool setCallReject(bool rejectAll);
  
  int getPhoneBookEntries(void (*cb)(PhoneBookEntry pe),int from, int to);
  bool deletePhoneBookEntry(int id);
  void setLogger(bool enabled,Stream *stream);
  bool logger;
  Stream *logger_stream;
  
  bool setAudioChannel(int channel);
};

#endif
