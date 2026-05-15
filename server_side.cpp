#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <mutex> 
#include "server_side.h" 

int start_server(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(1234); 

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "Server listening on port 1234..." << std::endl;

    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        char buffer[1024] = {0};
        read(new_socket, buffer, 1024);

        if (strstr(buffer, "GET /media_files") != nullptr) {
            std::string current_json_copy;
            {
                std::lock_guard<std::mutex> lock(json_mutex);
                current_json_copy = current_json;  // копируем под защитой мьютекса
            }
            
            std::string response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: application/json\r\n"
                       "Content-Length: " + std::to_string(current_json_copy.length()) + "\r\n"
                       "Connection: close\r\n"
                       "\r\n" + 
                       current_json_copy;
            send(new_socket, response.c_str(), response.length(), 0);
        } else {
            const char *response = "HTTP/1.1 404 Not Found\r\n"
                                "Content-Length: 0\r\n"
                                "\r\n";
            send(new_socket, response, strlen(response), 0);
        }

        close(new_socket);
    }

    close(server_fd);
    return 0;   
}

