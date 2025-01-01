#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>

class Config {
public:
    static bool enableLogging;
    static bool enableMetrics;
    static bool livePlayPlayer0;
    static bool livePlayPlayer1;
    static int portPlayer0;
    static int portPlayer1;

    static void parseConfig(const std::string& filename);
};

#endif // CONFIG_H
