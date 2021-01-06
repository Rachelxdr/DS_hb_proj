#include "../header_files/Node.h"

void* server_start (void* arg) {
    Node* cur_node = (Node*)arg;
    int server_socket_create = cur_node->udp_util->create_server_socket();
    if (server_socket_create == 1) {
        cout << "[ERROR] Failed to create server" << endl;
        cur_node->node_logger->log("[ERROR] Failed to create server socket");
        exit(1);
    }
    return NULL;
}

void* client_start(void* arg) {
    Node* cur_node = (Node*)arg;
    
    string msg = cur_node->pack_membership_list();
    if (msg.compare("ERROR") == 0) {
        cout << "[ERROR] Create membership_list" << endl;
        return NULL;
    }
    if (cur_node->membership_list.size() == 1) {
        cur_node->send_to_introducer(msg);
    }

    while (cur_node->status == ACTIVE) {
        cur_node->read_message();

        if (cur_node->status ==FAILED) {
            break;
        }
        cur_node->failure_detection();
        cur_node->hb++;
        cur_node->local_clock++;
        cur_node->send_hb();
        sleep(T_period);

        
    /*
        1. Read message
        2. Process message
            a. Update membership list
                i. Failure detection
                ii. Remove members
            b. Switch mode
        3. Increase Hb and local_clock
        4. Send out membership list based on mode
        5. Sleep for T_period
    */
    }
    return NULL;
}