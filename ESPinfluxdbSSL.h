/* Influxdb library

   MIT license
   Written by HW Wong
 */

#ifndef INFLUXDBSSL_H
#define INFLUXDBSSL_H
#include "Arduino.h"

#if defined(ESP8266)
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <HTTPClient.h>
#endif

enum DB_RESPONSE {DB_SUCCESS, DB_ERROR, DB_CONNECT_FAILED};

// Url encode function
String URLEncode(String msg);

class dbMeasurement
{
public:
dbMeasurement(String m);

String measurement;


void addField(String key, float value);
void addTag(String key, String value);
void empty();
String postString();

private:
String _data;
String _tag;
};

class InfluxdbSSL
{
public:
InfluxdbSSL(const char* host, uint16_t port, const char* fingerprint );

DB_RESPONSE opendb(String db);
DB_RESPONSE opendb(String db, String user, String password);
DB_RESPONSE write(dbMeasurement data);
DB_RESPONSE write(String data);
DB_RESPONSE query(String sql);
//uint8_t createDatabase(char *dbname);
DB_RESPONSE response();

private:
String _port;
String _host;
String _db;
const char *_fingerprint;

DB_RESPONSE _response = DB_ERROR;

};




#endif
