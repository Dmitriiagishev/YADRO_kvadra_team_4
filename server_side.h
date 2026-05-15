#ifndef SERVER_SIDE_H
#define SERVER_SIDE_H

#include <string>
#include <mutex>

extern std::mutex json_mutex;
extern std::string current_json;

int start_server();

#endif