#include "Arduino.h"
#include "ESPinfluxdbSSL.h"


//#define DEBUG_PRINT // comment this line to disable debug print

#ifndef DEBUG_PRINT
#define DEBUG_PRINT(a)
#else
#define DEBUG_PRINT(a) (Serial.println(String(F("[Debug]: "))+(a)))
#define _DEBUG
#endif

InfluxdbSSL::InfluxdbSSL(const char *host, uint16_t port, const char* fingerprint) {
        _port = String(port);
        _host = String(host);
        _fingerprint = fingerprint;
}

DB_RESPONSE InfluxdbSSL::opendb(String db, String user, String password) {
        //_db = db + "&u=" + user + "&p=" + password;
        HTTPClient http;
        http.begin("https://" + _host + ":" + _port + "/query?q=show%20databases" + "&u=" + user + "&p=" + password, _fingerprint); //HTTP
//        const char* fingerprint = "40 79 4F 58 AF 71 51 E4 65 C8 0D BF D8 21 18 7E DE 2D 09 C3";
        int httpCode = http.GET();
        if (httpCode == 200) {
                _response = DB_SUCCESS;
                String payload = http.getString();
                http.end();
                if (payload.indexOf("[[\"" + db + "\"]]" ) > 0) {
                        //_db = db;
                        _db = db + "&u=" + user + "&p=" + password;
                        return _response;
                }
        }
        _response = DB_ERROR;
        DEBUG_PRINT("Database open failed");
        return _response;
}

DB_RESPONSE InfluxdbSSL::opendb(String db) {

        HTTPClient http;
//        const char* fingerprint = "40 79 4F 58 AF 71 51 E4 65 C8 0D BF D8 21 18 7E DE 2D 09 C3";
        http.begin("https://" + _host + ":" + _port + "/query?q=show%20databases", _fingerprint); //HTTP

        int httpCode = http.GET();
        if (httpCode == 200) {
                _response = DB_SUCCESS;
                String payload = http.getString();
                http.end();
                if (payload.indexOf("[[\"" + db + "\"]]" ) > 0) {
                        _db = db;
                        return _response;
                }
        }
        _response = DB_ERROR;
        DEBUG_PRINT("Database open failed");
        return _response;

}

DB_RESPONSE InfluxdbSSL::write(dbMeasurement data) {
        return write(data.postString());
}

DB_RESPONSE InfluxdbSSL::write(String data) {


        HTTPClient http;

        DEBUG_PRINT("HTTP post begin...");
//        const char* fingerprint = "40 79 4F 58 AF 71 51 E4 65 C8 0D BF D8 21 18 7E DE 2D 09 C3";
        http.begin("https://" + _host + ":" + _port + "/write?db=" + _db, _fingerprint); //HTTP
        http.addHeader("Content-Type", "text/plain");

        int httpResponseCode = http.POST(data);

        if (httpResponseCode == 204) {
                _response = DB_SUCCESS;
                String response = http.getString();    //Get the response to the request
                DEBUG_PRINT(String(httpResponseCode)); //Print return code
                DEBUG_PRINT(response);                 //Print request answer

        } else {
                DEBUG_PRINT("Error on sending POST:");
                DEBUG_PRINT(String(httpResponseCode));
                _response=DB_ERROR;
        }

        http.end();
        return _response;
}

DB_RESPONSE InfluxdbSSL::query(String sql) {

        String url = "/query?";
        url += "pretty=true&";
        url += "db=" + _db;
        url += "&q=" + URLEncode(sql);
        DEBUG_PRINT("Requesting URL: ");
        DEBUG_PRINT(url);

        HTTPClient http;
//        const char* fingerprint = "40 79 4F 58 AF 71 51 E4 65 C8 0D BF D8 21 18 7E DE 2D 09 C3";
        http.begin("https://" + _host + ":" + _port + url, _fingerprint); //HTTP


        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode == 200) {
                // HTTP header has been send and Server response header has been handled
                _response = DB_SUCCESS;
                String reply = http.getString();
                Serial.println(reply);

        } else {
                _response = DB_ERROR;
                DEBUG_PRINT("[HTTP] GET... failed, error: " + httpCode);
        }

        http.end();
        return _response;
}


DB_RESPONSE InfluxdbSSL::response() {
        return _response;
}

/* -----------------------------------------------*/
//        Field object
/* -----------------------------------------------*/
dbMeasurement::dbMeasurement(String m) {
        measurement = m;
}

void dbMeasurement::empty() {
        _data = "";
        _tag = "";
}

void dbMeasurement::addTag(String key, String value) {
        _tag += "," + key + "=" + value;
}

void dbMeasurement::addField(String key, float value) {
        _data = (_data == "") ? (" ") : (_data += ",");
        _data += key + "=" + String(value);
}

String dbMeasurement::postString() {
        //  uint32_t utc = 1448114561 + millis() /1000;
        return measurement + _tag + _data;
}

// URL Encode with Arduino String object
String URLEncode(String msg) {
        const char *hex = "0123456789abcdef";
        String encodedMsg = "";

        uint16_t i;
        for (i = 0; i < msg.length(); i++) {
                if (('a' <= msg.charAt(i) && msg.charAt(i) <= 'z') ||
                    ('A' <= msg.charAt(i) && msg.charAt(i) <= 'Z') ||
                    ('0' <= msg.charAt(i) && msg.charAt(i) <= '9')) {
                        encodedMsg += msg.charAt(i);
                } else {
                        encodedMsg += '%';
                        encodedMsg += hex[msg.charAt(i) >> 4];
                        encodedMsg += hex[msg.charAt(i) & 15];
                }
        }
        return encodedMsg;
}