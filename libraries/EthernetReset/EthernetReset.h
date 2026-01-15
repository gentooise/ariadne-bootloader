#ifndef EthernetReset_h
#define EthernetReset_h

#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <string.h>
#include <Arduino.h>
#include <NewEEPROM.h>
#include <NetEEPROM.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <EthernetClient.h>

#define pgm_uchar(name)   static const prog_uchar name[] PROGMEM

#define ETHERNETRESET_DEBUG 0
#ifdef ETHERNETRESET_DEBUG > 0
	#define DBG(c) c
#else
	#define DBG(c)
#endif

#define MAX_RAW_APIS 4

class EthernetReset
{
	private:
		EthernetServer _server;
		EthernetClient _client;
		char _path[20];
		const char* _main_page; // main page expected as PROGMEM array
		const char* _raw_apis[MAX_RAW_APIS];
		const char* _raw_values[MAX_RAW_APIS];
		short _apicount;

		void rawResponse(const char*);
		void stdResponse();
		void watchdogReset();
		void stop(void);

	public:
		EthernetReset(int, const char*);
		//~EthernetReset();

		void begin(void);
		void check(void);

		void resetRawAPIs(void);
		void addRawAPI(const char*, const char*);
};

#endif
