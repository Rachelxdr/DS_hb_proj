#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <tuple>
#include <string>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sstream>
#include <vector>
#include <pthread.h>

#include "./Udp_socket.h"
#include "./Logger.h"



#define INACTIVE 0
#define ACTIVE 1
#define FAILED 2

// TODO: why?
#define T_fail 3
#define T_cleanup 3
#define T_period 5


using namespace std;

string get_ip();
string create_id();
vector<string> splitString(string s, string delim);
void* server_start(void* arg);
void* client_start(void* arg);

class Node {
    public:
        // tuple <string, string, string> id;
        string id; //ip_addr::port::join_time
        int hb;
        int local_clock;
        map <string, tuple <int, int, int>> membership_list; //id -> HB, clock, flag
        Udp_socket* udp_util;
        int mode; // 0 = a2a, 1 = gossip
        Logger* node_logger;
        int status;
        int is_introducer;
        

        Node(int mode);


        void join_group();
        string pack_membership_list();
        void send_to_introducer(string msg);
        void read_message();
        void failure_detection();
        void send_hb();
        void update_self_info();

    private:
        void membership_list_init();
        void process_received_message(string type, string content);
        void merge_membership_list(string msg);
        void process_mem(string mem_id, string mem_info);


};

#endif