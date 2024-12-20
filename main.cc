#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "logger.h"
#include "parser.cc"

using json = nlohmann::json;

void log(const std::string& message) {
    Logger::getInstance().log(message);
}

int main() {
    log("Daemon started");
/*
    // Example JSON string
    std::string jsonString = R"({
        "name": "John Doe",
        "age": 30,
        "is_student": false,
        "skills": ["C++", "Python", "JSON"]
    })";

    // Parse JSON string to JSON object
    json jsonObject = json::parse(jsonString);

    // Access JSON data
    std::string name = jsonObject["name"];
    int age = jsonObject["age"];
    bool isStudent = jsonObject["is_student"];
    std::vector<std::string> skills = jsonObject["skills"];

    // Print the data
    std::cout << "Name: " << name << std::endl;
    std::cout << "Age: " << age << std::endl;
    std::cout << "Is Student: " << (isStudent ? "Yes" : "No") << std::endl;
    std::cout << "Skills: ";
    
    log(name);
    log(std::to_string(age));
    log("Done with the tests");
*/
    //------------
    

    std::string input;
    int counter = 0;

    while (true) {
        log("Waiting for input");
        std::getline(std::cin, input);
        log(input);

        try {
            json jsonObject = json::parse(input);
            GameState gameState = jsonObject.get<GameState>();
            log("This is the game state parsed -- ");
            log(to_string(gameState));
        } catch (const std::exception& e) {
            std::cerr << "Caught an unknown exception while parsing " << e.what() << std::endl;
        }
        


        if (input == "exit") break;

        if (input == "error") {
            std::cerr << "An error occurred!" << std::endl;
            continue; 
        }

        counter++;
        // std::cout << "Processed: " << input << ", Counter: " << counter << std::endl;
        std::cout << "0 0 0" << std::endl;
        std::cout.flush(); // Ensure output is immediately flushed
    }

    return 0;
}
