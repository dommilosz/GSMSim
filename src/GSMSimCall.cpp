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
#include "Arduino.h"
#include "GSMSim.h"

#define gsm_print(x) { \
	if(logger){ \
		logger_stream->print("AT < "); \
		logger_stream->print(x); \
		logger_stream->print(";\n"); \
	} \
	gsm.print(x); \
} \

//////////////////////////////////////
//			CALL	SECTION			//
//////////////////////////////////////
// arama ile ilgili olması gereken ayarları yapar...
bool GSMSim::initCall() {
	if(setCOLP(true)) {
		if(showCurrentCall(true)) {
			if(setCLIP(false)) {
				if(setCLIR(false)) {
					if(showCurrentCall(true)) {
						return true;
					} else {
						return false;
					}
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else {
		return false;
	}
}

// Arama Yapar
bool GSMSim::call(char* phone_number) {

	gsm_print(F("ATD"));
	gsm_print(phone_number);
	gsm_print(";\r");

	_readSerial();
	
	if (_buffer.indexOf(F("OK")) != -1)
	{
		return true;
	}
	else {
		if(_buffer.indexOf(F("+CLCC:")) != -1) {
			return true;
		} else {
			return false;
		}
	}
	
}

bool GSMSim::call(String *phone_number) {

	char c[phone_number->length()+1];
	c[phone_number->length()] = 0;
	phone_number->toCharArray(c,phone_number->length()+1);
	return call(c);
}

// Gelen aramayı cevaplar
bool GSMSim::answer() {
	gsm_print(F("ATA\r"));

	_readSerial();

	if (_buffer.indexOf(F("OK")) != -1)
	{
		return true;
	}
	else {
		return false;
	}
}

// Aramayı reddeder veya görüşmeyi sonlandırır!
bool GSMSim::hangoff() {
	gsm_print(F("ATH\r"));
	_readSerial();

	if(_buffer.indexOf(F("OK")) != -1)
	{
		return true;
	} else {
		return false;
	}
}

// Arama durumunu belirtir
int GSMSim::status() {
	/*
		values of return:
		0 Ready (MT allows commands from TA/TE)
		2 Unknown (MT is not guaranteed to respond to tructions)
		3 Ringing (MT is ready for commands from TA/TE, but the ringer is active)
		4 Call in progress
	*/
	gsm_print(F("AT+CPAS\r"));
	_readSerial();
	return _buffer.substring(_buffer.indexOf(F("+CPAS: ")) + 7, _buffer.indexOf(F("+CPAS: ")) + 9).toInt();
}

// CLIP açık ya da kapalı
bool GSMSim::setCLIP(bool active) {
	int durum = active == true ? 1 : 0;
	gsm_print(F("AT+CLIP="));
	gsm_print(durum);
	gsm_print("\r");

	_readSerial();

	if (_buffer.indexOf(F("OK")) == -1)
	{
		return false;
	}
	else {
		return true;
	}
}

// CLIR Açık ya da kapalı
bool GSMSim::setCLIR(bool active) {
	int durum = active == true ? 1 : 0;
	gsm_print(F("AT+CLIR="));
	gsm_print(durum);
	gsm_print("\r");

	_readSerial();

	if (_buffer.indexOf(F("OK")) == -1)
	{
		return false;
	}
	else {
		return true;
	}
}

// Connected Line Identification aktif veya kapalı
bool GSMSim::setCOLP(bool active) {
	int durum = active == true ? 1 : 0;
	gsm_print(F("AT+COLP="));
	gsm_print(durum);
	gsm_print("\r");

	_readSerial();

	if (_buffer.indexOf(F("OK")) == -1)
	{
		return false;
	}
	else {
		return true;
	}
}

// COLP Aktif mi değil mi?
bool GSMSim::isCOLPActive() {
	gsm_print(F("AT+COLP?\r"));
	_readSerial();

	if (_buffer.indexOf(F("+COLP: 1")) == -1) {
		return false;
	}
	else {
		return true;
	}
}

// Arayanı söyleme aktif mi değil mi?
bool GSMSim::showCurrentCall(bool active) {
	int durum = active == true ? 1 : 0;
	gsm_print(F("AT+CLCC="));
	gsm_print(durum);
	gsm_print("\r");

	_readSerial();

	if (_buffer.indexOf(F("OK")) == -1)
	{
		return false;
	}
	else {
		return true;
	}
}
// şimdi arayanı söyle
String GSMSim::readCurrentCall(String serialRaw) {

	String sonuc = "";
	serialRaw.trim();
	if (serialRaw.indexOf(F("+CLCC:")) != -1) {
		String CLCCvalue = serialRaw.substring(serialRaw.indexOf(F("+CLCC:"))); // on answer call, it give +COLP: xxxx OK and +CLCC: xxx. So we must split it.
		String durum = CLCCvalue.substring(CLCCvalue.indexOf(F("+CLCC:"))+11,CLCCvalue.indexOf(F("+CLCC:"))+12);
		String numara = CLCCvalue.substring(CLCCvalue.indexOf(F(",\""))+2, CLCCvalue.indexOf(F("\",")));

		if (durum == "0") {
			durum = "STATUS:ACTIVE"; // Görüşme var
		}
		else if (durum == "1") {
			durum = "STATUS:HELD";
		}
		else if (durum == "2") {
			durum = "STATUS:DIALING"; // Çevriliyor
		}
		else if (durum == "3") {
			durum = "STATUS:RINGING"; // Çalıyor
		}
		else if (durum == "4") {
			durum = "STATUS:INCOMING"; // Gelen arama
		}
		else if (durum == "5") {
			durum = "STATUS:WAITING"; // gelen arama bekliyor
		}
		else if (durum == "6") {
			if(serialRaw.indexOf(F("BUSY")) != -1) {
				durum = "STATUS:BUSY"; // kullanıcı meşgul bitti
			} else {
				durum = "STATUS:CALLEND"; // görüşme bitti
			}
			
		}

		sonuc = durum + "|NUMBER:" + numara;
	}

	return sonuc;
}

// gelen aramaları otomatik reddetmeyi açar kapatır
bool GSMSim::setCallReject(bool reject) {
	if(reject) {
		gsm_print(F("AT+GSMBUSY=1\r"));
	} else {
		gsm_print(F("AT+GSMBUSY=0\r"));
	}

	_readSerial();

	if (_buffer.indexOf(F("OK")) != -1)
	{
		return true;
	}
	else {
		return false;
	}
}
