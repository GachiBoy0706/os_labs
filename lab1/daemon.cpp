#include <unistd.h>    
#include <sys/types.h> 
#include <sys/stat.h>  
#include <signal.h>    
#include <fcntl.h>     
#include <stdlib.h>    
#include <iostream>
#include <fstream>
#include <syslog.h>
#include <cstring>

#include "daemon.hpp"


void Daemon::start(){

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        syslog(LOG_ERR, "cannot get current directory, error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    current_path = std::string(cwd);
   
    openlog("diy_daemon", LOG_PID, LOG_DAEMON);

    checkAndTerminateExistingProcess();
    
    readConfig();
    
    daemonize();
    
    run();
}




bool isProcessRunning(pid_t pid) {
    std::string procPath = "/proc/" + std::to_string(pid);
    struct stat sts;
    return (stat(procPath.c_str(), &sts) == 0);
}

void terminateProcess(pid_t pid) {
    if (kill(pid, SIGTERM) != 0) {
        syslog(LOG_ERR, "couldn`t terminate process");
    }
}

void Daemon::removePidFile() {
    remove(pidFilePath);
}

void Daemon::checkAndTerminateExistingProcess() {
    std::ifstream pidFile(pidFilePath);

    if (pidFile.is_open()) {
        pid_t existingPid;
        pidFile >> existingPid;
        pidFile.close();

        if (isProcessRunning(existingPid)) {
            syslog(LOG_INFO, "detected running process with PID: %i; terminating", existingPid);
            terminateProcess(existingPid);
            sleep(1);  
        }

        removePidFile();
    }
}




void Daemon::readConfig(){
    syslog(LOG_INFO, "started reading config");
    
    std::string path = current_path + std::string("/config.txt");
    

    std::ifstream file(path);

    if (!file.is_open()) {
        std::cout << path << std::endl;
        exit(EXIT_FAILURE);
    }
   
    std::getline(file, folder1_path);
    std::getline(file, folder2_path);
    std::string line;
    std::getline(file, line);
    time = std::stoi(line); 

    file.close();

}




bool Daemon::createPidFile() {
    std::ofstream pidFile(pidFilePath);
    if (!pidFile.is_open()) {
        syslog(LOG_ERR, "couldn`t create PID file");
        return false;
    }
    pidFile << getpid();
    pidFile.close();
    return true;
}

void signalHandler(int signal){
    switch (signal)
    {
        case SIGHUP:
            syslog(LOG_INFO, "Rereading config file");
            Daemon::getInstance().readConfig();
            break;
        case SIGTERM:
            syslog(LOG_INFO, "Terminate session");
            closelog();
            exit(EXIT_SUCCESS);
            break;
    }

}

void Daemon::daemonize(){
    syslog(LOG_INFO, "started daemonizing");

    pid_t pid;
    pid = fork();

    if (pid < 0) {
        syslog(LOG_ERR, "fork went wrong, error %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        syslog(LOG_ERR, "setsid went wrong, error %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    signal(SIGHUP, SIG_IGN);
    umask(0);
    chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    if (!createPidFile()) {
        exit(EXIT_FAILURE);
    }

    signal(SIGHUP, signalHandler);
    signal(SIGTERM, signalHandler);
}





std::vector<fs::path> Daemon::findLogs(){
    std::vector<fs::path> log_files;  

    for (const auto& entry : fs::recursive_directory_iterator(folder1_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".log") {
            log_files.push_back(entry.path()); 
        }
    }

    return log_files;
}

void Daemon::appendTotalLog(std::vector<fs::path>& log_files){
    std::ofstream totalLog(folder2_path + std::string("/total.log"), std::ios::app); 

    if (!totalLog) {
        syslog(LOG_ERR, "cannot open total.log, error: %s", strerror(errno));
        return;
    }

    for (const auto& filePath : log_files) {
        std::ifstream inputFile(filePath);

        if (!inputFile) {
            syslog(LOG_ERR, "cannot open %s, error: %s", filePath.string().c_str(), strerror(errno));
            continue;  
        }

        totalLog << "\n\n";

        totalLog << filePath.string() << "\n";

        totalLog << inputFile.rdbuf();  

        inputFile.close();
    }

    totalLog.close();
}

void Daemon::removeLogs(std::vector<fs::path>& log_files) {
    
    for (const auto& entry : log_files) {
        if (entry.extension() == ".log") {
            syslog(LOG_INFO, "Removing log: %s", entry.string().c_str());
            fs::remove(entry);  
        }
    }
}

void Daemon::run(){
    
    std::vector<fs::path> log_files;
    
    while(true){
        log_files = findLogs();
        appendTotalLog(log_files);
        removeLogs(log_files);

        sleep(time);
    }
    
}