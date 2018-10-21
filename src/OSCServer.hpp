#ifndef PORTS_OCSSERVER_HPP
#define PORTS_OCSSERVER_HPP

#include <thread>


class OSCServer {
public:
	OSCServer(int port);
	~OSCServer();
	void stop();
	void setCallback(void (* cb)(const char *path, const float value)){
		callback = cb;
	}
	volatile bool running = false;
private:
	std::thread* thread;
	volatile bool shouldRun = false;
	void run(int port);
	void handleOSCBuffer(char *buffer, int len);
	void (* callback)(const char *path, const float value)  = NULL;
	
};

#endif // PORTS_OCSSERVER_HPP