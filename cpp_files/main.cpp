#include "../header_files/Node.h"

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cout << "[USAGE] ./node.cpp <HEARTBEAT_MODE> (0 = a2a / 1 = gossip) [OPTIONAL] -i"<<endl;
        exit(1);
    }
   


    if (stoi(argv[1]) == 0) {
        cout << "[MODE] ALL2ALL"<< endl;
    } else if (stoi(argv[1]) == 1) {
        cout << "[MODE] GOSSIP" << endl;
    } else {
        cout << "[ERROR CMD ARG] INVALID MODE --- 0 = ALL2ALL  1 = GOSSIP" <<endl;
        exit(1);
    }
    int hb_mode = stoi(argv[1]);

    Node* node = new Node(hb_mode);
    
    node->node_logger->log("[START_NODE] " + node->id);
     cout << "[START_NODE]" + node->id<< endl;
    

    pthread_t server_thread;
    int server_thread_create = pthread_create(&server_thread, NULL, server_start, (void*) node);

    if (server_thread_create != 0) {
        cout << "[ERROR] Failed to create server thread" << endl;
        exit(1);
    }
    
    string cmd;
    while (1) {
        cin >> cmd;
        if (cmd == "join") {
            node->join_group();
        } else if (cmd == "leave") {
            cout << "leave group" << endl;
        } else if (cmd == "list") {
            cout << "membership list" << endl;
        } else if (cmd == "id") {
            cout << "self id" << endl;
        } else if (cmd == "switch") {
            cout << "switch hb style" << endl;
        } else {
            cout << "[ERROR] Invalid command"<< endl;
        }
    }


    return 0;
}