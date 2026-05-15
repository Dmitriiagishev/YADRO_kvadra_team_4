#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>  
#include <thread>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include "server_side.h"
std::mutex json_mutex;
std::string current_json;

std::vector<std::string> split(std::string in_str, char delimiter)
{
    std::stringstream ss(in_str);
    std::vector<std::string> res;
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        res.push_back(token);
    }
    return res;
}

std::string make_json(std::unordered_map<std::string, std::vector<std::string>>& file_lib){
    std::string out_str = "{ \"audio\":[";
    if (file_lib["audio"].size() > 0){
        for (size_t i = 0; i < file_lib["audio"].size()-1; i++){
            out_str.append("\"");
            out_str.append(file_lib["audio"][i]);
            out_str.append("\", ");
        }
        out_str.append("\"");
        out_str.append(file_lib["audio"].back());
        out_str.append("\"");
    }
    out_str.append("], \"video\":[");

    if (file_lib["video"].size() > 0){
        for (size_t i = 0; i < file_lib["video"].size()-1; i++){
            out_str.append("\"");
            out_str.append(file_lib["video"][i]);
            out_str.append("\", ");
        }
        out_str.append("\"");
        out_str.append(file_lib["video"].back());
        out_str.append("\"");
    }
    out_str.append("], \"images\":[");
    
    if (file_lib["images"].size() > 0){
        for (size_t i = 0; i < file_lib["images"].size()-1; i++){
            out_str.append("\"");
            out_str.append(file_lib["images"][i]);
            out_str.append("\", ");
        }
        out_str.append("\"");
        out_str.append(file_lib["images"].back());
        out_str.append("\"");
    }
    out_str.append("] }");
    return out_str;
}

std::unordered_map<std::string, std::vector<std::string>> sort_by_type(std::vector<std::string>& found_files){
    std::unordered_map<std::string, std::string> file_types; 
    const std::vector<std::string> audio = {"mp3", "wav", "flac", "aac", "ogg", "wma", "m4a", "opus"};
    for (size_t i = 0; i < audio.size(); i ++){
        file_types[audio[i]] = "audio";
    }
    const std::vector<std::string> video = {"mp4", "avi", "mkv", "mov", "wmv", "flv", "webm", "m4v", "mpg", "mpeg", "3gp"};
    for (size_t i = 0; i < video.size(); i ++){
        file_types[video[i]] = "video";
    }
    const std::vector<std::string> images = {"jpg", "jpeg", "png", "gif", "bmp", "tiff", "webp", "svg", "ico", "raw"};
    for (size_t i = 0; i < images.size(); i ++){
        file_types[images[i]] = "images";
    }
    std::unordered_map<std::string, std::vector<std::string>> files_lib;
    for (size_t i = 0; i < found_files.size(); i ++){
        std::string extension = split(found_files[i], '.').back();
        if (file_types.find(extension) == file_types.end()){
            continue;
        }
        files_lib[file_types[extension]].push_back(found_files[i]);
    }
    //std::cout << "audio\t" << files_lib["audio"].size() << "\nvideo\t" << files_lib["video"].size() << "\nimages\t" << files_lib["images"].size() << '\n';
    return files_lib;
}

void crawl(int seconds){
    while (true){
        auto start = std::chrono::steady_clock::now();
        const char* home_dir = getenv("HOME");
        
        if (!home_dir) {
            std::cerr << "Error: Cannot get HOME directory" << std::endl;
            return ;
        }
        
        const std::filesystem::path target_path{home_dir};
        std::vector<std::string> found_files;
        
        // Пропуск недоступных директорий
        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(
                target_path, 
                std::filesystem::directory_options::skip_permission_denied))
        {
            try {
                if (std::filesystem::is_regular_file(dir_entry.path()))
                {
                    found_files.push_back(dir_entry.path().filename().string());
                }
            }
            catch (std::filesystem::filesystem_error const& ex) {
                // Пропуск файлов, к которым нет доступа
                continue;
            }
        }
        
        std::unordered_map<std::string, std::vector<std::string>> files_lib = sort_by_type(found_files);

        std::string output_path = std::string(home_dir) + "/.media_files";
        std::ofstream out_file(output_path); 
        //make_json(files_lib);
        /*
        if (out_file.is_open()) {
            out_file << make_json(files_lib);
            out_file.close(); 
            
        } else {
            std::cout << "Unable to open file for writing: " << output_path << std::endl;
        }*/
        {
            std::lock_guard<std::mutex> lock(json_mutex);
            current_json = make_json(files_lib);
        }

        auto end = std::chrono::steady_clock::now();
        auto work_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        auto sleep_time = std::chrono::milliseconds(seconds * 1000) - work_duration;
        std::cout << "new snapshot ready\n";
        if (sleep_time > std::chrono::milliseconds(0)) {
            std::this_thread::sleep_for(sleep_time);
        } 
    }
    
    return ;
}

int main(int argc, char *argv[])
{
    int interval = 300;
    
    if (argc == 2) interval = atoi(argv[1]);
    else std::cout << "Can\'t recognise int argument\nSnapshots will be made every 300 seconds\n";

    
    std::thread scanner(crawl, interval);
    std::thread server(start_server); // который внутри вызывает start_server

    // Ждать завершения (сервер обычно бесконечный, так что main просто висит)
    server.join();
    scanner.join();

    return 0;
}