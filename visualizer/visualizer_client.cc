#include <iostream>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <array>

#include "visualizer_client.h"
#include "logger.h"

// Custom deleter for FILE pointers
struct FileCloser {
    void operator()(FILE* file) const {
        if (file) {
            pclose(file);
        }
    }
};

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;

    // Use std::unique_ptr with the custom deleter
    std::unique_ptr<FILE, FileCloser> pipe(popen(cmd, "r"));
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

int send_game_data() {
    std::string url = "http://localhost:8088/hello";
    std::string command = "curl -s " + url;

    try {
        std::string response = exec(command.c_str());
        Logger::getInstance().log("Response: " + response);
    } catch (const std::exception& e) {
        Logger::getInstance().log("Exception: " + std::string(e.what()));
    }

    return 0;
}
