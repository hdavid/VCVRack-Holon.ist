#ifndef PORTS_MDNSSERVER_HPP
#define PORTS_MDNSSERVER_HPP

#include <thread>
#if __APPLE__
#import <dns_sd.h>
#elif _WIN32
//#include whatever.h
#else // Linux
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
// From https://www.avahi.org/doxygen/html/client-publish-service_8c-example.html
/*#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/alternative.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>*/
#endif


class MdnsServer {
public:
	MdnsServer(int port);
	~MdnsServer();
	void stop();
	volatile bool running = false;
private:
	volatile bool shouldRun = false;
	std::thread* thread;
	void run(int port);
#if __APPLE__
	void handleEvents(DNSServiceRef serviceRef);
#elif _WIN32
	//win32stuff
#else 
	//linux
	/*AvahiEntryGroup *group = NULL;
	AvahiSimplePoll *simple_poll = NULL;
	char *name = NULL;
	void entryGroupCallback(AvahiEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata);
	void createServices(AvahiClient *c);
	void clientCallback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata);
	void modifyCallback(AVAHI_GCC_UNUSED AvahiTimeout *e, void *userdata);*/
#endif
};


#endif // PORTS_MDNSSERVER_HPP