#include "../header_files/Node.h"
#include "./util.cpp"

Node::Node(int hb_mode) {
   id = create_id();
   hb = 0;
   local_clock = 0;
   mode = hb_mode;
   node_logger = new Logger(id + "_log.txt");
   status = INACTIVE;
   is_introducer = 0;
   udp_util = new Udp_socket();
}

void Node::join_group() {
    this->status = ACTIVE;
    this->membership_list_init();
    pthread_t client_thread;
    int client_create = pthread_create(&client_thread, NULL, client_start, (void*)this);
}

void Node::process_received_message(string type, string content) {
    if (type == "INIT_REQ") {
        // Current node is the introducer
        // New node joining
        // Send membership list over
        // content ip::port::start_time==hb#flag

        //May cause some nodes never get to join the group???

        string membership_list_msg = pack_membership_list();
        vector<string> parse_result = splitStirng(content, "::");
        string response_target_ip = parse_result[0];
        int send_ret = this->udp_util->send_message(membership_list_msg, response_target_ip, INIT_RES);
        if (send_ret != 0) {
            cout << "[ERROR] Message not sent" << endl;
        }

    } else if (type == "INIT_RES") {
        merge_membership_list(content);
    } else if (type == "HEARTBEAT") {
        merge_membership_list(content);
    } else if (type == "SWITCH") {
        switch_mode(content);
    }
}

void Node::print_membership_list() {
    cout << "========================================= MEMBERSHIP LIST INFO =========================================" << endl;
    cout << "               Member ID                 Heartbeat Counter             Clock                Status      " << endl;
        //    172.17.0.2::9000::140565190499201              44                      1                  active
    map <string, tuple<int, int, int>> :: iterator it;
    for (it = this->membership_list.begin(); it != this->membership_list.end(); it++) {
            string mem_id = it->first;
            tuple<int, int, int> mem_info = it->second;
            string mem_hb = to_string(get<0>(mem_info));
            string mem_clock = to_string(get<1>(mem_info));
            int flag = get<1>(mem_info);
            string status; 
            if (flag == FAILED) {
                status = "Inactive";
            } else {
                status = "Active";
            }
            cout << "  " << mem_id << "  " << "           " << mem_hb << "                       " << mem_clock << "                   " << status << "         "<<endl;

    }
    cout << "========================================================================================================" << endl;
}

void Node::print_members_in_system() {
    cout << "=============== Current Members ================"<<endl;
    int ct = 1;
    map <string, tuple<int, int, int>> :: iterator it;
    for (it = this->membership_list.begin(); it != this->membership_list.end(); it++) {
        cout << "[" << ct << "] " << it->first << endl;
        ct++;
    }
    cout << "================================================="<<endl;
}

string Node::pack_membership_list() {
    string msg;
    map <string, tuple<int, int, int>> :: iterator it;
    for (it = this->membership_list.begin(); it != this->membership_list.end(); it++) {
        string member_id = it->first;
        tuple <int, int, int> member_info = it->second;
        string member_hb = to_string(get<0>(member_info));
        string member_flag = to_string(get<2>(member_info));

        // ip_addr::port::join_time==hb#flag||ip_addr::port...
        msg += member_id + "==" + member_hb + "#" + member_flag + "||";
    }
    string sub = msg.substr(msg.length() - 2, 2);
    if (sub.compare("||")) {
        return "ERROR";
    }
    msg.erase(msg.length() - 2, 2);
    return msg;
}

void Node::update_self_info() {
    tuple <int, int, int> new_info = make_tuple(this->hb, this->local_clock, this->status);
    this->membership_list[this->id] = new_info;
}

void Node::membership_list_init() {
    tuple <int, int, int> info = make_tuple(this->hb, this->local_clock, this->status);
    this->membership_list.insert(pair<string, tuple<int, int, int>>(this->id, info));
}

void Node::send_to_introducer(string msg) {
    int send_ret = this->udp_util->send_message(msg, INTRODUCER_IP, INIT_REQ);
    if (send_ret != 0) {
        cout << "[ERROR] Node to introdocer message not sent" << endl;
    }
}

void Node::send_hb() {
    string msg_to_send = pack_membership_list();
    // map<string, tuple<int, int, int>>::iterator it;
    // vector <string> all_targets;
    // for (it = this->membership_list.begin(); it != this->membership_list.end(); it++) {
    //     if (it->first != this->id && get<2>(it->second) != FAILED) {
    //         vector<string> parse_id = splitStirng(it->first,  "::");
    //         if (parse_id.size() != 3) {
    //             cout << "[ERROR] Incorrect ip format in membership list" << endl;
    //         }
    //         string target_ip = parse_id[0];
            
    //     }
    // }
    vector <string> targets_to_send = this->get_targets();
    // if (this->mode == 1 && all_targets.size() > GOSSIP_SIZE) {
    //     for (int k = 0; k < GOSSIP_SIZE; k++) {
    //         int rand_num = rand() % all_targets.size();
    //         targets_to_send.push_back(all_targets[rand_num]);
    //         all_targets.erase(all_targets.begin() + rand_num);
    //     }
    // } else {
    //     targets_to_send = all_targets;
    // }
    for (int i = 0; i < targets_to_send.size(); i++) {
        this->udp_util->send_message(msg_to_send, targets_to_send[i], HEARTBEAT);
    }
}

vector<string> Node::get_targets() {
    vector <string> all_targets;
    map<string, tuple<int, int, int>>::iterator it;
    for (it = this->membership_list.begin(); it != this->membership_list.end(); it++) {
        if (it->first != this->id && get<2>(it->second) != FAILED) {
            vector<string> parse_id = splitStirng(it->first,  "::");
            if (parse_id.size() != 3) {
                cout << "[ERROR] Incorrect ip format in membership list" << endl;
            }
            string target_ip = parse_id[0];
            all_targets.push_back(target_ip);
        }
    }
    vector <string> targets_to_send;
    if (this->mode == 1 && all_targets.size() > GOSSIP_SIZE) {
        for (int k = 0; k < GOSSIP_SIZE; k++) {
            int rand_num = rand() % all_targets.size();
            targets_to_send.push_back(all_targets[rand_num]);
            cout << "[INFO] Gossip target: " << all_targets[rand_num] << endl;;
            all_targets.erase(all_targets.begin() + rand_num);
        }
    } else {
        targets_to_send = all_targets;
    }
    return targets_to_send;
}

void Node::send_switch_request() {
    string target_mode;
    if (this->mode == 0) {
        target_mode = "Gossip";
    } else {
        target_mode = "All2All";
    }
    map<string, tuple<int, int, int>>::iterator it;
    for (it = this->membership_list.begin(); it != this->membership_list.end(); it++) {
        if (get<2>(it->second) != FAILED) {
            vector<string> parse_id = splitStirng(it->first,  "::");
            if (parse_id.size() != 3) {
                cout << "[ERROR] Incorrect ip format in membership list" << endl;
            }
            string target_ip = parse_id[0];
            this->udp_util->send_message(target_mode, target_ip, SWITCH);
        }
    }
}

void Node::switch_mode(string target_mode) {
    if (target_mode == "Gossip" && this->mode == 0) {
        this->mode = 1;
        string msg = "[SWITCH] Switched mode to GOSSIP";
        this->node_logger->log(msg);
        cout << msg << endl;
        return;
    }

    if (target_mode == "All2All" && this->mode == 1) {
        this->mode = 0;
        string msg = "[SWITCH] Switched mode to All2All";
        this->node_logger->log(msg);
        cout << msg << endl;
        return;
    }
    
}

void Node::merge_membership_list(string msg) {
    // ip_addr::port::join_time==hb#flag||ip_addr::port...
    cout << "[DEBUG] membership message before split string: "<< msg << endl;
    vector<string> members = splitStirng(msg, "||");
    cout << "[DEBUG] =================== member vector ====================== " << endl;
    int ct = 0;
    for (string cur_mem : members) {
        cout << "At position: " << ct << " " << cur_mem << endl;
        ct++;
    }
    for (int i = 0; i < members.size(); i++) {
        cout << "[DEBUG MEMBER] Received member info: " << members[i] <<endl;
        vector<string> parse_mem = splitStirng(members[i], "==");
            if (parse_mem.size() != 2) {
                cout << "[ERROR] HEARTBEAT format invalid" <<endl;
                continue;
            }
            string mem_id = parse_mem[0]; // ip_addr::port::join_time
            string mem_info = parse_mem[1]; // hb# flag
            vector<string> parse_info = splitStirng(mem_info, "#");
            if (parse_info.size() != 2) {
                cout << "[ERROR] Member info format error" <<endl;
                continue;
            }

            int mem_hb = stoi(parse_info[0]);
            int mem_flag = stoi(parse_info[1]);

            /*
            1. If member is self:
                a. If marked as failed, change this->Status to FAILED and break
                b. Do nothing ?
            2. <NOT SELF> If id doesn't exist in the list, add new member
            3. <NOT SELF> <NOT NEW> If marked as failed, change the member flag on list and update local time
            4. <NOT SELF> <NOT NEW> <NOT FAILED> If incoming hb is larger than local hb, update hb and local time.
            5. <NOT SELF> <NOT NEW> <NOT FAILED> <SMALLER/EQ HB VAL> Do nothing. 
            */
        
           if (mem_id == this->id) { // Member is self
               if (mem_flag == FAILED) {
                   this->status = FAILED;
                   string log_msg = "[VOLUNTARILY LEAVE] " + this->id + " leaves group";
                   this->node_logger->log(log_msg); 
                   break;
               }
           } else {
               // TODO: other cases
                map<string, tuple<int, int, int>>::iterator it;
                it = this->membership_list.find(mem_id);
                if (it == this->membership_list.end()) {
                    if (mem_flag != FAILED) { // New member
                        tuple <int, int, int> mem_to_list = make_tuple(mem_hb, this->local_clock, mem_flag);
                        this->membership_list.insert(pair<string, tuple<int, int, int>>(mem_id, mem_to_list));
                        string log_msg = "[NEW MEMBER] " + mem_id;
                        cout << log_msg << endl;
                        this->node_logger->log(log_msg);
                    }
                } else {
                    int hb_in_list = get<0>(this->membership_list[mem_id]);
                    int clock_in_list = get<1>(this->membership_list[mem_id]);
                    int flag_in_list = get<2>(this->membership_list[mem_id]);

                    if (mem_flag == FAILED && flag_in_list != FAILED) { // Received as failed member
                        tuple<int, int, int> new_tuple = make_tuple(hb_in_list, this->local_clock, FAILED);
                        this->membership_list[mem_id] = new_tuple;
                        string log_msg = "[FAILURE INFO] " + mem_id;
                        cout << log_msg << endl;
                        this->node_logger->log(log_msg);
                    } else{
                        if (hb_in_list < mem_hb) { // Higher hb value
                            tuple<int, int, int> new_tuple = make_tuple(mem_hb,  this->local_clock, mem_flag);
                            this->membership_list[mem_id] = new_tuple;
                        } 
                    }
                }
           }
            

    }
}

void Node::failure_detection() {
    map<string, tuple<int, int, int>>::iterator it;
    vector<string> remove_members;
    for (it = this->membership_list.begin(); it != this->membership_list.end(); it++) {
        string mem_id = it->first;
        int mem_hb = get<0>(it->second);
        int mem_clock = get<1>(it->second);
        int mem_flag = get<2>(it->second);
         /*
         !! Skip self
         1. if member is failed and local_clock - mem_clock is more than T_cleanup: Add to remove member.
         2. <NOT FAILED> if local_clock - mem_clock is more than T_fail: mark member as failed
         */
        if (mem_id == this->id) {
            continue;
        } else {
            if (mem_flag == FAILED && this->local_clock - mem_clock > T_cleanup) {
                remove_members.push_back(mem_id);
            } else {
                if (mem_flag != FAILED && this->local_clock - mem_clock > T_fail) {
                    tuple<int, int, int> new_tuple = make_tuple(mem_hb, mem_clock, FAILED);
                    this->membership_list[mem_id] = new_tuple;
                    string log_msg = "[FAILURE DETECTION] " + mem_id + "current clock: " + to_string(this->local_clock) + " member clock: " + to_string(mem_clock) + "member hb: " + to_string(mem_hb);
                    this->node_logger->log(log_msg);
                    cout << log_msg << endl;
                }
            }
        }
        
    }
    for (int i = 0; i < remove_members.size(); i++ ) {
        this->membership_list.erase(remove_members[i]);
        string log_msg = "[REMOVE] " + remove_members[i];
        cout << log_msg <<endl;
        this->node_logger->log(log_msg);
    }
}

void Node::read_message(){
    pthread_mutex_lock(&this->udp_util->queue_lock);

    
    // cout << "[DEBUG] mem addr of queue update_list: " << &this->udp_util->messages_received<<endl;
   
    queue<string> messages_to_process(this->udp_util->messages_received);
    this->udp_util->messages_received = queue<string>();
    
    pthread_mutex_unlock(&this->udp_util->queue_lock);
    // Message format : MessageType>>>message
    string msg;
    // cout << "[DEBUG] updating membership list" <<endl;
    // cout << "[DEBUG] messages_to_process size: " << messages_to_process.size() <<endl;
    while (!messages_to_process.empty()) {
        
        msg = messages_to_process.front();
        cout << "[DEBUG] Node msg from queue msg: " << msg << endl; 
        // Parse message type
        vector<string> str_v = splitStirng(msg, ">>>");
        // for (int i = 0; i < str_v.size(); i++) {
        //     cout<< "[DEBUG CLIENT] str_v at " << i << " : " << str_v[i] <<endl;
        //     fflush(stdout);
        // }
        if (str_v.size() == 2) {
            string type = str_v[0];
            string content = str_v[1];
            process_received_message(type, content);
        } else {
            cout << "[ERROR] Invalid queued message format "<<endl;
        }
        messages_to_process.pop();
    }
    // sleep(10);
}