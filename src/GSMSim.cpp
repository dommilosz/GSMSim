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
#include "stdint.h"

#define gsm_print(x) { \
	if(logger){ \
		logger_stream->print("AT < "); \
		logger_stream->print(x); \
		logger_stream->print(";\n"); \
	} \
	gsm.print(x); \
} \

void GSMSim::init()
{
	pinMode(RESET_PIN, OUTPUT);
	digitalWrite(RESET_PIN, HIGH);

	if (LED_FLAG)
	{
		pinMode(LED_PIN, OUTPUT);
	}

	_buffer.reserve(BUFFER_RESERVE_MEMORY);

	echoOff();
}

GSMStatus GSMSim::getStatus()
{
	//+CIND:("battchg",(0-5)), ("signal",(0-5)), ("service",(0,1)), ("message",(0,1)),("call",(0,1)), ("roam",(0,1)), ("smsfull",(0,1))
	gsm_print(F("AT+CIND?\r"));
	_readSerial();
	String data = _buffer;

	GSMStatus status;
	status.error = true;

	if (data.indexOf("+CIND:") != -1)
	{
		data = data.substring(data.indexOf("+CIND: ") + 7);
		//+CIND: 5,4,1,1,1,0,0 - call
		String d = data.substring(0, data.indexOf(","));
		data = data.substring(data.indexOf(",") + 1);
		status.battchg = d.toInt();
		d = data.substring(0, data.indexOf(","));
		data = data.substring(data.indexOf(",") + 1);
		status.signal = d.toInt();
		d = data.substring(0, data.indexOf(","));
		data = data.substring(data.indexOf(",") + 1);
		status.service = d.toInt();
		d = data.substring(0, data.indexOf(","));
		data = data.substring(data.indexOf(",") + 1);
		status.message = d.toInt();
		d = data.substring(0, data.indexOf(","));
		data = data.substring(data.indexOf(",") + 1);
		status.call = d.toInt();
		d = data.substring(0, data.indexOf(","));
		data = data.substring(data.indexOf(",") + 1);
		status.roam = d.toInt();
		d = data.substring(0, 1);
		status.smsfull = d.toInt();
		status.error = false;
	}
	return status;
}

void GSMSim::reset()
{
	if (LED_FLAG)
	{
		digitalWrite(LED_PIN, HIGH);
	}

	digitalWrite(RESET_PIN, LOW);
	delay(1000);
	digitalWrite(RESET_PIN, HIGH);
	delay(1000);

	// Modul kendine geldi mi onu bekle
	gsm_print(F("AT\r"));
	_readSerial();
	while (_buffer.indexOf(F("OK")) == -1)
	{
		gsm_print(F("AT\r"));
		_readSerial();
	}

	if (LED_FLAG)
	{
		digitalWrite(LED_PIN, LOW);
	}
}

bool GSMSim::checkConnection(int retries)
{
	gsm_print(F("AT\r"));
	_readSerial();
	if (_buffer.indexOf(F("OK")) == -1)
	{
		if (retries > 0)
			return checkConnection(retries - 1);
		return false;
	}
	return true;
}

// send AT Command to module
String GSMSim::sendATCommand(char *command)
{
	gsm_print(command);
	gsm_print("\r");
	_readSerial(10000);
	return _buffer;
}

// SET PHONE FUNC +
bool GSMSim::setPhoneFunc(int level = 1)
{

	if (level == 0 || level == 1 || level == 4)
	{
		gsm_print(F("AT+CFUN="));
		gsm_print(level);
		gsm_print(F("\r"));

		_readSerial();

		if ((_buffer.indexOf(F("OK"))) != -1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

// SIGNAL QUALTY - 0-31 | 0-> poor | 31 - Full | 99 -> Unknown +
unsigned int GSMSim::signalQuality()
{
	gsm_print(F("AT+CSQ\r"));
	_readSerial(5000);

	if ((_buffer.indexOf(F("+CSQ:"))) != -1)
	{
		return _buffer.substring(_buffer.indexOf(F("+CSQ: ")) + 6, _buffer.indexOf(F(","))).toInt();
	}
	else
	{
		return 99;
	}
}

// IS Module connected to the operator? +
bool GSMSim::isRegistered()
{
	gsm_print(F("AT+CREG?\r"));
	_readSerial();

	if ((_buffer.indexOf(F("+CREG: 0,1"))) != -1 || (_buffer.indexOf(F("+CREG: 0,5"))) != -1 || (_buffer.indexOf(F("+CREG: 1,1"))) != -1 || (_buffer.indexOf(F("+CREG: 1,5"))) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// IS SIM Inserted? +
bool GSMSim::isSimInserted()
{
	gsm_print(F("AT+CSMINS?\r"));
	_readSerial();
	if (_buffer.indexOf(",") != -1)
	{
		// bölelim
		String veri = _buffer.substring(_buffer.indexOf(F(",")) + 1, _buffer.indexOf(F("OK")));
		veri.trim();
		//return veri;
		if (veri == "1")
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

// Pin statüsü - AT+CPIN? +
unsigned int GSMSim::pinStatus()
{
	gsm_print(F("AT+CPIN?\r"));
	_readSerial();

	if (_buffer.indexOf(F("READY")) != -1)
	{
		return 0;
	}
	else if (_buffer.indexOf(F("SIM PIN")) != -1)
	{
		return 1;
	}
	else if (_buffer.indexOf(F("SIM PUK")) != -1)
	{
		return 2;
	}
	else if (_buffer.indexOf(F("PH_SIM PIN")) != -1)
	{
		return 3;
	}
	else if (_buffer.indexOf(F("PH_SIM PUK")) != -1)
	{
		return 4;
	}
	else if (_buffer.indexOf(F("SIM PIN2")) != -1)
	{
		return 5;
	}
	else if (_buffer.indexOf(F("SIM PUK2")) != -1)
	{
		return 6;
	}
	else
	{
		return 7;
	}
}

// Unlock the pin code +
bool GSMSim::enterPinCode(char *pinCode)
{
	gsm_print(F("AT+CPIN=\""));
	gsm_print(pinCode);
	gsm_print(F("\"\r"));
	_readSerial(6000);

	if (_buffer.indexOf(F("ERROR")) != -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool GSMSim::enterPukCode(char *pukCode,char *pinCode)
{
	gsm_print(F("AT+CPIN=\""));
	gsm_print(pukCode);
	gsm_print("\",\"");
	gsm_print(pinCode);
	gsm_print(F("\"\r"));
	_readSerial(6000);

	if (_buffer.indexOf(F("ERROR")) != -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool GSMSim::changePinCode(char *oldCode, char *newCode){
gsm_print(F("AT+CPWD=\"SC\",\""));
	gsm_print(oldCode);
	gsm_print("\",\"");
	gsm_print(newCode);
	gsm_print(F("\"\r"));
	_readSerial(6000);

	if (_buffer.indexOf(F("ERROR")) != -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

// enable pin code... +
bool GSMSim::enablePinCode(char *pinCode)
{
	gsm_print(F("AT+CLCK=\"SC\",1,\""));
	gsm_print(pinCode);
	gsm_print(F("\"\r"));
	_readSerial(6000);
	if (_buffer.indexOf(F("ERROR")) != -1)
	{
		return false;
	}
	else
	{
		reset();
		return true;
	}
}

// disable pin code +
bool GSMSim::disablePinCode(char *pinCode)
{
	gsm_print(F("AT+CLCK=\"SC\",0,\""));
	gsm_print(pinCode);
	gsm_print(F("\"\r"));
	_readSerial(6000);
	if (_buffer.indexOf(F("ERROR")) != -1)
	{
		return false;
	}
	else
	{
		reset();
		return true;
	}
}

int GSMSim::getPinStatus()
{
	gsm_print(F("AT+CLCK=\"SC\",2\r"));
	_readSerial(6000);
	if (_buffer.indexOf(F("+CLCK:")) != -1)
	{
		_buffer = _buffer.substring(_buffer.indexOf("+CLCK: ")+7);
		String status = _buffer.substring(0,1);
		return status.toInt();
	}
	else
	{
		return -1;
	}
}

// OPERATOR NAME +
String GSMSim::operatorName()
{
	gsm_print(F("AT+COPS?\r"));
	_readSerial();

	if (_buffer.indexOf(F(",")) == -1)
	{
		return "NOT CONNECTED";
	}
	else
	{
		return _buffer.substring(_buffer.indexOf(F(",\"")) + 2, _buffer.lastIndexOf(F("\"")));
	}
}

// OPERATOR NAME FROM SIM +
String GSMSim::operatorNameFromSim()
{
	gsm.flush();
	gsm_print(F("AT+CSPN?\r"));
	_readSerial();
	//delay(250);
	//_readSerial();
	/*
	return _buffer;
	*/
	if (_buffer.indexOf(F("OK")) != -1)
	{
		return _buffer.substring(_buffer.indexOf(F(" \"")) + 2, _buffer.lastIndexOf(F("\"")));
	}
	else
	{
		return "NOT CONNECTED";
	}
}

// PHONE STATUS +
unsigned int GSMSim::phoneStatus()
{
	gsm_print(F("AT+CPAS\r"));
	_readSerial();

	if ((_buffer.indexOf("+CPAS: ")) != -1)
	{
		return _buffer.substring(_buffer.indexOf(F("+CPAS: ")) + 7, _buffer.indexOf(F("+CPAS: ")) + 9).toInt();
	}
	else
	{
		return 99; // not read from module
	}
}

int GSMSim::getPhoneBookEntries(void (*cb)(PhoneBookEntry pe),int from, int to){
	gsm_print(F("AT+CPBR="));
	gsm_print(from);
	gsm_print(",");
	gsm_print(to);
	gsm_print("\r");
	_readSerial();
	int count;

	if (_buffer.indexOf("ERROR") != -1)
	{
		return count;
	}
	else
	{
		// +CMGL: varsa döngüye girelim. yoksa sadece OK dönmüştür. O zaman NO_SMS diyelim
		if (_buffer.indexOf("+CPBR:") != -1)
		{
			String data = _buffer;
			bool quitLoop = false;

			while (!quitLoop)
			{
				if (data.indexOf("+CPBR:") == -1)
				{
					quitLoop = true;
					continue;
				}
				PhoneBookEntry pe;
				
				data = data.substring(data.indexOf("+CPBR: ") + 7);

  pe.id = data.substring(0, data.indexOf(",")).toInt();
  data = data.substring(data.indexOf("\"") + 1);
  pe.phoneno = data.substring(0, data.indexOf("\""));
  data = data.substring(data.indexOf(",")+1);
  pe.type = data.substring(0, data.indexOf(",")).toInt();
  data = data.substring(data.indexOf(",") + 2);
  pe.name = data.substring(0, data.indexOf("\""));
  pe.error = 0;
				
				cb(pe);
				count++;
			}
		}
		else
		{
			return count;
		}
	}

	return count;

}

bool GSMSim::deletePhoneBookEntry(int id){
	//AT+CPBW=${id}
	gsm_print(F("AT+CPBW="));
	gsm_print(id);
	gsm_print("\r");
	_readSerial();
	
	if (_buffer.indexOf(F("ERROR")) != -1)
	{
		return false;
	}
	else
	{
		return true;
	}
	
}

// ECHO OFF +
bool GSMSim::echoOff()
{
	gsm_print(F("ATE0\r"));
	_readSerial();
	if ((_buffer.indexOf(F("OK"))) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// ECHO ON +
bool GSMSim::echoOn()
{
	gsm_print(F("ATE1\r"));
	_readSerial();
	if ((_buffer.indexOf(F("OK"))) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Modül Üreticisi +
String GSMSim::moduleManufacturer()
{
	gsm_print(F("AT+CGMI\r"));
	_readSerial();
	String veri = _buffer.substring(8, _buffer.indexOf(F("OK")));
	veri.trim();
	veri.replace("_", " ");
	return veri;
}

// Modül Modeli +
String GSMSim::moduleModel()
{
	gsm_print(F("AT+CGMM\r"));
	_readSerial();

	String veri = _buffer.substring(8, _buffer.indexOf(F("OK")));
	veri.trim();
	veri.replace("_", " ");
	return veri;
}

// Modül Revizyonu +
String GSMSim::moduleRevision()
{
	gsm_print(F("AT+CGMR\r"));
	_readSerial();

	String veri = _buffer.substring(_buffer.indexOf(F(":")) + 1, _buffer.indexOf(F("OK")));
	veri.trim();
	return veri;
}

// Modülün IMEI numarası +
String GSMSim::moduleIMEI()
{
	gsm_print(F("AT+CGSN\r"));
	_readSerial();

	String veri = _buffer.substring(8, _buffer.indexOf(F("OK")));
	veri.trim();
	return veri;
}

// Modülün IMEI Numarasını değiştirir. +
bool GSMSim::moduleIMEIChange(char *imeino)
{
	gsm_print(F("AT+SIMEI="));
	gsm_print(imeino);
	gsm_print("\r");

	_readSerial();

	if ((_buffer.indexOf(F("OK"))) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Modülün SIM Numarası +
String GSMSim::moduleIMSI()
{
	gsm_print(F("AT+CIMI\r"));
	_readSerial();

	String veri = _buffer.substring(8, _buffer.indexOf(F("OK")));
	veri.trim();
	return veri;
}

// Sim Kart Seri Numarası +
String GSMSim::moduleICCID()
{
	gsm_print(F("AT+CCID\r"));
	_readSerial();

	String veri = _buffer.substring(8, _buffer.indexOf(F("OK")));
	veri.trim();

	return veri;
}

// Çalma Sesi +
unsigned int GSMSim::ringerVolume()
{
	gsm_print(F("AT+CRSL?\r"));
	_readSerial();

	String veri = _buffer.substring(7, _buffer.indexOf(F("OK")));
	veri.trim();

	return veri.toInt();
}

// Çalma sesini ayarla +
bool GSMSim::setRingerVolume(unsigned int level)
{
	if (level > 100)
	{
		level = 100;
	}

	gsm_print(F("AT+CRSL="));
	gsm_print(level);
	gsm_print(F("\r"));
	_readSerial();

	if (_buffer.indexOf(F("OK")) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Hoparlör sesi +
unsigned int GSMSim::speakerVolume()
{
	gsm_print(F("AT+CLVL?\r"));
	_readSerial();

	String veri = _buffer.substring(7, _buffer.indexOf(F("OK")));
	veri.trim();

	return veri.toInt();
}

// Hoparlör sesini ayarla +
bool GSMSim::setSpeakerVolume(unsigned int level)
{
	if (level > 100)
	{
		level = 100;
	}

	gsm_print(F("AT+CLVL="));
	gsm_print(level);
	gsm_print(F("\r"));

	_readSerial();

	if (_buffer.indexOf(F("OK")) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Modül Debug
String GSMSim::moduleDebug()
{
	gsm_print(F("AT&V\r"));
	_readSerial(60000);

	return _buffer;
}
// Bazı fonksiyonların modül üzerine kaydedilmesini sağlar...
bool GSMSim::saveSettingsToModule()
{
	gsm_print(F("AT&W\r"));
	_readSerial();

	if (_buffer.indexOf(F("OK")) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool GSMSim::setBaudrate(int rate)
{
	gsm_print(F("AT+IPR="));
	gsm_print(rate);
	gsm_print(F("\r"));
	_readSerial();

	if (_buffer.indexOf(F("OK")) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//////////////////////////////////////
//			PRIVATE METHODS			//
//////////////////////////////////////

// READ FROM SERIAL
void GSMSim::_readSerial()
{

	_buffer = "";
	uint64_t timeOld = millis();
	// Bir süre bekle...
	while (!gsm.available() && !(millis() > timeOld + TIME_OUT_READ_SERIAL))
	{
		;
	}
	// beklemeden çıkınca ya da süre dolunca varsa seriali oku, yoksa çık git...
	if (gsm.available())
	{
		_buffer = gsm.readString();
	}
}

void GSMSim::_readSerial(uint32_t timeout)
{
	_buffer = "";
	uint64_t timeOld = millis();
	// Bir süre bekle...
	while (!gsm.available() && !(millis() > timeOld + timeout))
	{
		;
	}
	// beklemeden çıkınca ya da süre dolunca varsa seriali oku, yoksa çık git...
	if (gsm.available())
	{
		_buffer = gsm.readString();
	}
}

void GSMSim::setLogger(bool enabled,Stream *stream){
	logger = enabled;
	logger_stream = stream;
}
	
bool GSMSim::setAudioChannel(int channel){
	gsm_print(F("AT+QAUDCH="));
	gsm_print(channel);
	gsm_print(F("\r"));
	_readSerial();

	if (_buffer.indexOf(F("OK")) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}
