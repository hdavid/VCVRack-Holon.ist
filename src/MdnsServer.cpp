#include "MdnsServer.hpp"
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#ifdef ARCH_WIN
#include <winsock2.h>
#else // Linux / MacOS
#include <arpa/inet.h>
#endif

MdnsServer::MdnsServer(int port) {
	thread = new std::thread(&MdnsServer::run, this, port);
}

MdnsServer::~MdnsServer() {
	stop();
}

void MdnsServer::stop() {
	if (running) {
		shouldRun = false;
		//printf("Ports: stop mDNS Server (shouldRun = false)\n");
		fflush(stdout);
	}
}


#ifdef ARCH_MAC
void MdnsServer::handleEvents(DNSServiceRef serviceRef) {
	int dns_sd_fd = DNSServiceRefSockFD(serviceRef);
	int nfds = dns_sd_fd + 1;
	fd_set readfds;
	struct timeval timeout = {0, 50*1000};// 50ms timeout for select.
	int result;
	while (shouldRun) {
		fflush(stdout);
		FD_ZERO(&readfds);
		FD_SET(dns_sd_fd, &readfds);
		result = select(nfds, &readfds, (fd_set *)NULL, (fd_set *)NULL, &timeout);
		if (result > 0) {
			DNSServiceErrorType err = kDNSServiceErr_NoError;
			if (FD_ISSET(dns_sd_fd, &readfds))
				err = DNSServiceProcessResult(serviceRef);
			if (err) {
				shouldRun = false;
			} else {
				//printf("select(	) returned %d errno %d %s\n", result, errno, strerror(errno));
				if (errno == EINTR) {
					shouldRun = false;
				}
			}
		}
	}
}

void registerCallBack(
	DNSServiceRef service, 
	DNSServiceFlags flags,
	DNSServiceErrorType errorCode,
	const char *name, 
	const char *type,
	const char *domain, 
	void *context
) {
#pragma unused(flags)
#pragma unused(context)
	if (errorCode != kDNSServiceErr_NoError) {
		//fprintf(stderr, "mDNSRegisterCallBack returned %d\n", errorCode);
	} else {
		//printf("%-15s %s.%s%s\n", "REGISTER", name, type, domain);
	}
}
#endif
#ifdef ARCH_WIN
//Windows


#else
//Linux

/*
 void MdnsServer::entryGroupCallback(AvahiEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata) {
		assert(g == group || group == NULL);
		group = g;
		// Called whenever the entry group state changes 
		switch (state) {
				case AVAHI_ENTRY_GROUP_ESTABLISHED :
						// The entry group has been established successfully 
						fprintf(stderr, "Service '%s' successfully established.\n", name);
						break;
				case AVAHI_ENTRY_GROUP_COLLISION : {
						char *n;
						// A service name collision with a remote service
						// happened. Let's pick a new name 
						n = avahi_alternative_service_name(name);
						avahi_free(name);
						name = n;
						fprintf(stderr, "Service name collision, renaming service to '%s'\n", name);
						// And recreate the services 
						createServices(avahi_entry_group_get_client(g));
						break;
				}
				case AVAHI_ENTRY_GROUP_FAILURE :
						fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))));
						// Some kind of failure happened while we were registering our services 
						avahi_simple_poll_quit(simple_poll);
						break;
				case AVAHI_ENTRY_GROUP_UNCOMMITED:
				case AVAHI_ENTRY_GROUP_REGISTERING:
						;
		}
}
void MdnsServer::createServices(AvahiClient *c) {
		char *n, r[128];
		int ret;
		assert(c);
		// If this is the first time we're called, let's create a new
		// entry group if necessary 
		if (!group)
				if (!(group = avahi_entry_group_new(c, entryGroupCallback, NULL))) {
						fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_client_errno(c)));
						goto fail;
				}
		// If the group is empty (either because it was just created, or
		// because it was reset previously, add our entries.	
		if (avahi_entry_group_is_empty(group)) {
				fprintf(stderr, "Adding service '%s'\n", name);
				// Create some random TXT data 
				snprintf(r, sizeof(r), "random=%i", rand());
				// We will now add two services and one subtype to the entry
				// group. The two services have the same name, but differ in
				 // the service type (IPP vs. BSD LPR). Only services with the
				// same name should be put in the same entry group. 
				// Add the service for IPP 
				if ((ret = avahi_entry_group_add_service(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, name, "_ipp._tcp", NULL, NULL, 651, "test=blah", r, NULL)) < 0) {
						if (ret == AVAHI_ERR_COLLISION)
								goto collision;
						fprintf(stderr, "Failed to add _ipp._tcp service: %s\n", avahi_strerror(ret));
						goto fail;
				}
				// Add the same service for BSD LPR 
				if ((ret = avahi_entry_group_add_service(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, name, "_printer._tcp", NULL, NULL, 515, NULL)) < 0) {
						if (ret == AVAHI_ERR_COLLISION)
								goto collision;
						fprintf(stderr, "Failed to add _printer._tcp service: %s\n", avahi_strerror(ret));
						goto fail;
				}
				// Add an additional (hypothetic) subtype 
				if ((ret = avahi_entry_group_add_service_subtype(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, name, "_printer._tcp", NULL, "_magic._sub._printer._tcp") < 0)) {
						fprintf(stderr, "Failed to add subtype _magic._sub._printer._tcp: %s\n", avahi_strerror(ret));
						goto fail;
				}
				// Tell the server to register the service 
				if ((ret = avahi_entry_group_commit(group)) < 0) {
						fprintf(stderr, "Failed to commit entry group: %s\n", avahi_strerror(ret));
						goto fail;
				}
		}
		return;
collision:
		// A service name collision with a local service happened. Let's
		// pick a new name 
		n = avahi_alternative_service_name(name);
		avahi_free(name);
		name = n;
		fprintf(stderr, "Service name collision, renaming service to '%s'\n", name);
		avahi_entry_group_reset(group);
		createServices(c);
		return;
fail:
		avahi_simple_poll_quit(simple_poll);
}
void MdnsServer::clientCallback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {
		assert(c);
		// Called whenever the client or server state changes 
		switch (state) {
				case AVAHI_CLIENT_S_RUNNING:
						// The server has startup successfully and registered its host
						// name on the network, so it's time to create our services 
						createServices(c);
						break;
				case AVAHI_CLIENT_FAILURE:
						fprintf(stderr, "Client failure: %s\n", avahi_strerror(avahi_client_errno(c)));
						avahi_simple_poll_quit(simple_poll);
						break;
				case AVAHI_CLIENT_S_COLLISION:
						// Let's drop our registered services. When the server is back
						// in AVAHI_SERVER_RUNNING state we will register them
						 // again with the new host name. 
				case AVAHI_CLIENT_S_REGISTERING:
						// The server records are now being established. This
						// might be caused by a host name change. We need to wait
						// for our own records to register until the host name is
						// properly esatblished. 
						if (group)
								avahi_entry_group_reset(group);
						break;
				case AVAHI_CLIENT_CONNECTING:
						;
		}
}
void MdnsServer::modifyCallback(AVAHI_GCC_UNUSED AvahiTimeout *e, void *userdata) {
		AvahiClient *client = userdata;
		fprintf(stderr, "Doing some weird modification\n");
		avahi_free(name);
		name = avahi_strdup("Modified MegaPrinter");
		// If the server is currently running, we need to remove our
		// service and create it anew 
		if (avahi_client_get_state(client) == AVAHI_CLIENT_S_RUNNING) {
				// Remove the old services
				if (group)
						avahi_entry_group_reset(group);
				// And create them again with the new name 
				createServices(client);
		}
}
*/
#endif

void MdnsServer::run(int port) {
	if (running) {
		return;
	}
	shouldRun=true;
	running=true;
	//printf("Ports: starting mDNS server\n");
	fflush(stdout);
#ifdef ARCH_WIN
	char instanceName[200];
	char* hostname;
	hostname = getenv("COMPUTERNAME");
	if (hostname != 0) {
#ifdef BUILDING_FOR_MIRACK
		sprintf(instanceName, "mirack-%s", hostname);
#else
		sprintf(instanceName, "vcvrack-%s", hostname);
#endif
		
	} else {
#ifdef BUILDING_FOR_MIRACK
		sprintf(instanceName, "mirack-nohostname");
#else
		sprintf(instanceName, "vcvrack-nohostname");
#endif
	}
#endif
#ifdef ARCH_MAC
	char instanceName[200];
	char* hostname;
	hostname = new char[512];
	if (gethostname(hostname, 512) == 0) { // success = 0, failure = -1
		const char delimiters[] = ".";
		char * running = strdup(hostname);
		hostname = strsep(&running, delimiters);
#ifdef BUILDING_FOR_MIRACK
		sprintf(instanceName, "mirack-%s", hostname);
#else
		sprintf(instanceName, "vcvrack-%s", hostname);
#endif
	} else {
#ifdef BUILDING_FOR_MIRACK
		sprintf(instanceName, "mirack-nohostname");	
#else
		sprintf(instanceName, "vcvrack-nohostname");
#endif
	}
	DNSServiceRef serviceRef;
	DNSServiceErrorType error = DNSServiceRegister(
		&serviceRef,
		0, // no flags
		0, // all network interfaces
		instanceName, // name // TODO: concat vcvrack & hostname
		"_osc._udp", // service type
		"", // register in default domain(s)
		NULL, // use default host name
		htons(port), // port number
		0, // length of TXT record
		NULL, // no TXT record
		registerCallBack, // call back function
		NULL // no context
	);								
	if (error == kDNSServiceErr_NoError) {
		handleEvents(serviceRef);
		DNSServiceRefDeallocate(serviceRef);
	}else if (error == kDNSServiceErr_BadParam) {
		//printf("ERROR :		kDNSServiceErr_BadParam\n");
	}
#endif
#ifdef ARCH_WIN
	//Windows
#endif
#ifdef ARCH_LIN
	//Linux
	/*//int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char*argv[]) {
	AvahiClient *client = NULL;
	int error;
	int ret = 1;
	struct timeval tv;
	// Allocate main loop object 
	if (!(simple_poll = avahi_simple_poll_new())) {
			fprintf(stderr, "Failed to create simple poll object.\n");
			goto fail;
	}
	name = avahi_strdup("MegaPrinter");
	// Allocate a new client 
	client = avahi_client_new(avahi_simple_poll_get(simple_poll), 0, clientCallback, NULL, &error);
	// Check wether creating the client object succeeded 
	if (!client) {
			fprintf(stderr, "Failed to create client: %s\n", avahi_strerror(error));
			goto fail;
	}
	// After 10s do some weird modification to the service 
	avahi_simple_poll_get(simple_poll)->timeout_new(
			avahi_simple_poll_get(simple_poll),
			avahi_elapse_time(&tv, 1000*10, 0),
			modifyCallback,
			client
	);
	// Run the main loop 
	avahi_simple_poll_loop(simple_poll);
	ret = 0;
fail:
	/// Cleanup things 
	if (client)
			avahi_client_free(client);
	if (simple_poll)
			avahi_simple_poll_free(simple_poll);
	avahi_free(name);
	return ret;
	*/
#endif
	running = false;
	//printf("Ports: mDNS server terminated\n");
	fflush(stdout);
}

