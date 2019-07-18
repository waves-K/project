#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/epoll.h>

#define MAX 1024
#define HOME_PAGE "index.html"
#define PAGE_404 "wwwroot/404.html"
#define PAGE_400 "wwwroot/400.html"
#define PAGE_500 "wwwroot/500.html"


#define INFO 0
#define WARNING 1
#define ERROR 2

uint64_t getTimeStap() {
	struct timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec;
}

std::string getLevel(int level) {
	switch(level) {
		case 0:
			return "INFO";
		case 1:
			return "WARNING";
		case 2:
			return "ERROR";
		default:
			return "UNKNOW";
	}
}

void log(int level, std::string message, std::string file, int line) {
	std::cout << " [ " << getTimeStap() << " ] ";
	std::cout << " [ " << getLevel(level) << " ] ";
	std::cout << " [ " << file << " ] ";
	std::cout << " [ " << line << " ] ";
	std::cout << " [ " << message << " ] ";
	std::cout << std::endl;
}

#define LOG(level, str) log(level, str, __FILE__, __LINE__)

#endif
