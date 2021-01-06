#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <pthread.h>
#include <stdio.h>
#include <queue>
#include "./MessageType.h"

#define PORT "9000"
#define INTRODUCER_IP "172.17.0.2" // 172.17.0.2 for docker testing, 127.0.0.1 for local testing
#define INTERFACE "eth0"  // eth0 for docker testing, lo0 for local testing
#define BACK_LOG 20
#define MAX_MSG_SIZE 2048




using namespace std;
class Udp_socket {
    public:
        queue<string> messages_received;
        pthread_mutex_t queue_lock;

        int create_server_socket();
        int send_message(string msg, string ip_addr, MessageType type);

        Udp_socket();
};


#endif