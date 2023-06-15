// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       maxtrix 8x32 clock.ino
    Created:	5/6/2023 8:48:14 PM
    Author:     LINH-PC\linhb
*/

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//


#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <time.h>
#include <ESP8266WebServer.h>
#include "EEPROM.h"
#include <time.h>
int pinCS = D4; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
char time_value[20];


ESP8266WebServer server(80);

const char*     ssid = "configWifiForTime";
const char*     passphrase = "123456789";

String          st;
String          content;
int             statusCode;
int hour, minute, second, day, month, year;
// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D4  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 100; // In milliseconds

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels

void setup() {
	matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
	matrix.setRotation(0, 1);    // The first display is position upside down
	matrix.setRotation(1, 1);    // The first display is position upside down
	matrix.setRotation(2, 1);    // The first display is position upside down
	matrix.setRotation(3, 1);    // The first display is position upside down
	Serial.begin(115200);
	EEPROM.begin(512);
	delay(10);
	Serial.println("Startup");
	display_message("Connecting");
	// read eeprom for ssid, pass and blynk
	Serial.println("Reading EEPROM ssid");
	String esid;
	for (int i = 0; i < 32; ++i) {
		esid += char(EEPROM.read(i));
	}
	Serial.print("SSID: ");
	Serial.println(esid.c_str());
	esid.trim();

	Serial.println("Reading EEPROM pass");
	String epass = "";
	for (int i = 32; i < 96; ++i) {
		epass += char(EEPROM.read(i));
	}
	Serial.print("PASS: ");
	Serial.println(epass.c_str());
	epass.trim();


	if (esid.length() > 1) {
		WiFi.begin(esid.c_str(), epass.c_str());
		if (testWifi()) {
			launchWeb(0);
			//WiFi.disconnect();
			EEPROM.end();
			int timezone = 7 * 3600;
			int dst = 0;
			configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
			display_message("Connected");
			return;
		}
	}
	setupAP();
	EEPROM.end();

	configTime(7 * 3600, 3, "1.asia.pool.ntp.org", "time.nist.gov");
	//setenv("TZ", "GMT07ST,M3.5.0/01,M10.5.0/02",1);

}

void loop() {
	server.handleClient();
	showTime();
	//delay(2000);
	//display_message(time); // Display time in format 'Wed, Mar 01 16:03:20 2017
}
void showTime() 	{
	static uint32_t lastShow;
	static uint32_t lastChangeDot;
	uint32_t nowMillis = millis();
	static bool dot;
	if (nowMillis >= lastChangeDot + 500) 		{
		lastChangeDot = nowMillis;
		updateTime();
		if (hour>8 && hour<22) {
			dot = !dot;
		}
	}
	if (nowMillis >= lastShow + 200) {
		lastShow = nowMillis;
		matrix.fillScreen(LOW);
		time_t now = time(nullptr);
		String time = String(ctime(&now));
		time.trim();
		Serial.println(time);
		time.substring(11, 19).toCharArray(time_value, 10);
		matrix.drawChar(2, 0, time_value[0], HIGH, LOW, 1); // H
		matrix.drawChar(8, 0, time_value[1], HIGH, LOW, 1); // HH
		matrix.drawChar(14, 0, (dot? time_value[2]:' '), HIGH, LOW, 1); // HH:
		matrix.drawChar(20, 0, time_value[3], HIGH, LOW, 1); // HH:M
		matrix.drawChar(26, 0, time_value[4], HIGH, LOW, 1); // HH:MM
		for (int i = 1; i <= (second / 2)+1; i++) 			{
			matrix.drawPixel(i, 7, 1);
		}
		
		matrix.write();
		 // Send bitmap to display
	}

}
void display_message(String message) {
	for (int i = 0; i < width * message.length() + matrix.width() - spacer; i++) {
		//matrix.fillScreen(LOW);
		int letter = i / width;
		int x = (matrix.width() - 1) - i % width;
		int y = (matrix.height() - 8) / 2; // center the text vertically
		while (x + width - spacer >= 0 && letter >= 0) {
			if (letter < message.length()) {
				matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background off, reverse to invert the image
			}
			letter--;
			x -= width;
		}
		matrix.write(); // Send bitmap to display
		delay(wait / 2);
	}
}
bool testWifi(void) {
	int c = 0;
	bool i = true;
	Serial.println("Xin vui long doi ket noi WIFI");
	while (c < 32) {
		if (WiFi.status() == WL_CONNECTED) {
			return true;
		}
		delay(500);
		Serial.print(WiFi.status());
		c++;
		i = !i;
		matrix.drawChar(1, 1, 'C', 1, 0, 1);
		matrix.drawChar(8, 1, 'O', 1, 0, 1);
		matrix.drawChar(16, 1, 'N', 1, 0, 1);
		matrix.drawChar(24, 1, 'N', 1, 0, 1);
		matrix.drawPixel(c, 0, 1);
		matrix.write();
	}
	Serial.println("");
	Serial.println("Thoi gian ket noi cham, Mo AP");
	return false;
}

void launchWeb(int webtype) {
	Serial.println("");
	Serial.println("WiFi ket noi");
	Serial.print("Dia chi IP: ");
	Serial.println(WiFi.localIP());
	Serial.print("SoftAP IP: ");
	Serial.println(WiFi.softAPIP());
	createWebServer(webtype);
	// Start the server
	server.begin();
	Serial.println("May chu bat dau");
}

void setupAP(void) {
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);
	int n = WiFi.scanNetworks();
	Serial.println("Tim hoan tat");
	if (n == 0) {
		Serial.println("khong tim thay wifi");
	} else {
		Serial.print(n);
		Serial.println(" networks found");
		for (int i = 0; i < n; ++i) {
			// Print SSID and RSSI for each network found
			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.print(")");
			Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
			delay(10);
		}
	}
	Serial.println("");
	st = "<ol>";
	for (int i = 0; i < n; ++i) {
		// Print SSID and RSSI for each network found
		st += "<li>";
		st += WiFi.SSID(i);
		st += " (";
		st += WiFi.RSSI(i);
		st += ")";
		st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
		st += "</li>";
	}
	st += "</ol>";
	delay(100);
	Serial.println("softap");
	Serial.println(ssid);
	Serial.println(passphrase);
	WiFi.softAP(ssid, passphrase, 6);
	display_message("Config WIFI");
	launchWeb(1);
	Serial.println("over");
}

void createWebServer(int webtype) {
	if (webtype == 1) {
		server.on("/", []() {
			IPAddress ip = WiFi.softAPIP();
			String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
			content = "<!DOCTYPE HTML>\r\n<html><h2>linhtran_electronics 0335644677</h2>";
			//content += ipStr;
			//content += "<form method='get' action='setting'><table width='100%' border='1'><tr><td width=\"30%\"><label>Wifi</label></td><td width=\"70%\><input name='ssid' length=32 width='500'></td></tr><tr><td><label>Password</label></td><td><input name='pass' length=64 width='500'></td></tr><tr><td><label>Blynk</label></td><td><input name='blynk' length=32 width='500'></td></tr><tr><td></td><td><input type='submit'></tr></tr></table></form>";
			content += "<form method=\"get\" action=\"setting\">";
			content += "<div>Wifi</div>";
			content += "<div><input name=\"ssid\" size=\"40\"></div>";
			content += "<div>Mat Khau</div>";
			content += "<div><input name=\"pass\" size=\"40\"></div>";
			content += "<div><input type='submit'></div>";

			content += "<p>";
			content += st;
			content += "</p>";
			content += "</html>";
			server.send(200, "text/html", content);
		});
		server.on("/setting", []() {
			String qsid = server.arg("ssid");
			String qpass = server.arg("pass");
			String qblynk = server.arg("blynk");
			if (qsid.length() > 0 && qpass.length() > 0) {
				EEPROM.begin(512);
				Serial.println("clearing eeprom");
				for (int i = 0; i < 128; ++i) {
					EEPROM.write(i, 0);
				}
				EEPROM.commit();
				Serial.println(qsid);
				Serial.println("");
				Serial.println(qpass);
				Serial.println("");


				Serial.println("writing eeprom ssid:");
				for (int i = 0; i < qsid.length(); ++i) {
					EEPROM.write(i, qsid[i]);
					Serial.print("Wrote: ");
					Serial.println(qsid[i]);
				}

				Serial.println("writing eeprom pass:");
				for (int i = 0; i < qpass.length(); ++i) {
					EEPROM.write(32 + i, qpass[i]);
					Serial.print("Wrote: ");
					Serial.println(qpass[i]);
				}

				Serial.println("writing eeprom blynk:");
				for (int i = 0; i < qblynk.length(); ++i) {
					EEPROM.write(96 + i, qblynk[i]);
					Serial.print("Wrote: ");
					Serial.println(qblynk[i]);
				}
				EEPROM.commit();
				EEPROM.end();
				content = "{\"Success\":\"luu hoan tat. khoi dong lai thiet bi! LINHTRAN_ELECTRONICS xin cam on.\"}";
				statusCode = 200;
			} else {
				content = "{\"Error\":\"404 not found\"}";
				statusCode = 404;
				Serial.println("Sending 404");
			}
			server.send(statusCode, "application/json", content);
		});
	} else if (webtype == 0) {
		server.on("/", []() {
			IPAddress ip = WiFi.localIP();
			String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
			server.send(200, "application/json", "{\"IP\":\"" + ipStr + "\"}");
		});
		server.on("/cleareeprom", []() {
			content = "<!DOCTYPE HTML>\r\n<html>";
			content += "<h2>XSwitch</h2><p>Clearing the EEPROM</p></html>";
			server.send(200, "text/html", content);
			Serial.println("clearing eeprom");
			for (int i = 0; i < 128; ++i) {
				EEPROM.write(i, 0);
			}
			EEPROM.commit();
		});
	}
}

void updateTime() {
	time_t now = time(nullptr);
	struct tm* p_tm = localtime(&now);
	hour = p_tm->tm_hour;
	minute = p_tm->tm_min;
	second = p_tm->tm_sec;
	day = p_tm->tm_mday;
	month = p_tm->tm_mon + 1;
	year = p_tm->tm_year + 1900;
	year = year - 2000;
}