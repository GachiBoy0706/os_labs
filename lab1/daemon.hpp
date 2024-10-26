#pragma once
#include <limits.h> 
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;


class Daemon{

    public:

    static Daemon& getInstance(){
        static Daemon instance;
        return instance;
    };

    void start();


    private:

    Daemon() = default;
    ~Daemon() = default;
    Daemon(const Daemon&) = delete;
    Daemon& operator = (const Daemon&) = delete;

    void daemonize();
    void readConfig();
    void run();


    std::vector<fs::path> findLogs();
    void appendTotalLog(std::vector<fs::path>&);
    void removeLogs(std::vector<fs::path>&);

    friend void signalHandler(int singal);

    const char* pidFilePath = "/var/run/diy_daemon.pid";

    bool createPidFile();
    void removePidFile();
    void checkAndTerminateExistingProcess();

    std::string current_path;
    std::string folder1_path;
    std::string folder2_path;
    int time;
};