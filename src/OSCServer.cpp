#include "OSCServer.hpp"

#include "tinyosc.h"
#ifdef ARCH_WIN
#include <winsock2.h>
#else // Linux / MacOS
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

OSCServer::OSCServer(int port) {
	thread = new std::thread(&OSCServer::run, this, port);
}

OSCServer::~OSCServer() {
	stop();
}

void OSCServer::stop() { 
	if (shouldRun){
		shouldRun = false;
#ifdef ARCH_WIN
		closesocket(s);
#endif
		//printf("Ports: stop Osc Server (shouldRun = false)\n");
		//fflush(stdout);
	}
}


void OSCServer::handleOSCBuffer(char *buffer, int len) {
	if (tosc_isBundle(buffer)) {
		tosc_bundle bundle;
		tosc_parseBundle(&bundle, buffer, len);
		// const uint64_t timetag = tosc_getTimetag(&bundle);
		tosc_message osc;
		while (tosc_getNextMessage(&bundle, &osc)) {
			tosc_printMessage(&osc);
			char *path = tosc_getAddress(&osc);
			float value = 0;
			for (int i = 0; osc.format[i] != '\0'; i++) {
				switch (osc.format[i]) {
				case 'f':
					value = tosc_getNextFloat(&osc);
					break;
				case 'd':
					value = tosc_getNextDouble(&osc);
					break;
				case 'i':
					value = tosc_getNextInt32(&osc);
					break;
				default:
					//printf(" Unknown format: '%c'", osc.format[i]);
					break;
				}
			}
			if (callback!=NULL && shouldRun){
				callback(path, value);
			}
		}
	} else {
		tosc_message osc;
		tosc_parseMessage(&osc, buffer, len);
		char *path = tosc_getAddress(&osc); 
		float value = 0;
		for (int i = 0; osc.format[i] != '\0'; i++) {
			switch (osc.format[i]) {
			case 'f':
				value = tosc_getNextFloat(&osc);
				break;
			case 'd':
				value = tosc_getNextDouble(&osc);
				break;
			case 'i':
				value = tosc_getNextInt32(&osc);
				break;
			default:
				//printf(" Unknown format: '%c'", osc.format[i]);
				break;
			}
		}
		if (callback!=NULL && shouldRun) {
			callback(path, value);
		}
	}
}

void OSCServer::run(int port) {
	if (running) {
		return;
	}
	running = true;
	shouldRun = true;
	//printf("Ports: starting OSC server\n");
	fflush(stdout);
	char buffer[2048];
	struct sockaddr_in server;
	struct sockaddr sa;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;
	int len;
#ifdef ARCH_WIN
	int sa_len;
	WSADATA wsa;
	sa_len = sizeof(sa);
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		//printf("Ports: Failed. Error Code : %d", WSAGetLastError());
		return;
	}
	// Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		//printf("Ports: Could not create socket : %d", WSAGetLastError());
	}
	// Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		//printf("Ports: Bind failed with error code : %d", WSAGetLastError());
		return;
	}
	while (shouldRun) {
		memset(buffer, '\0', 2048);
		// try to receive some data, this is a blocking call, but we interupt it using closesocket(s);
		if ((len = recvfrom(s, buffer, 2048, 0, (struct sockaddr *)&sa, &sa_len)) == SOCKET_ERROR) {
			//printf("Ports: recvfrom() failed with error code : %d", WSAGetLastError());
		} else {
			handleOSCBuffer(buffer, len);
		}
	}
	closesocket(s);
	WSACleanup();
#else // Linux / MacOS
	// open a socket to listen for datagrams
	const int fd = socket(AF_INET, SOCK_DGRAM, 0);
	fcntl(fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking
	int error = bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
	if (error!=0) {
		//printf("Ports: FAILED to bind OSC port 9000 : %d\n", error);
		return;
	}
	struct timeval timeout = {0, 50*1000}; // select times out after 50ms, to allow for other threads to complete happily
	while (shouldRun) {
		fflush(stdout);
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(fd, &readSet);
		if (select(fd + 1, &readSet, NULL, NULL, &timeout) > 0) {
			socklen_t sa_len = sizeof(struct sockaddr_in);
			len = 0;
			while ((len = (int)recvfrom(fd, buffer, sizeof(buffer), 0, &sa, &sa_len)) > 0) {
				handleOSCBuffer(buffer, len);
			}
		}
	}
	// close the UDP socket
	close(fd);
#endif
	running = false;
	//printf("Ports: terminate OSC Server\n");
	fflush(stdout);
}
