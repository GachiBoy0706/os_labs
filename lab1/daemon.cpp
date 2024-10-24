#include <unistd.h>    // Для fork(), setsid(), sleep()
#include <sys/types.h> // Для pid_t
#include <sys/stat.h>  // Для umask()
#include <signal.h>    // Для signal()
#include <fcntl.h>     // Для open()
#include <stdlib.h>    // Для exit()
#include <iostream>
#include <fstream>

#include "daemon.hpp"


void Daemon::start(){

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << "Current working directory: " << cwd << std::endl;
    } else {
        std::cerr << "Error getting current directory." << std::endl;
    }
    current_path = std::string(cwd);
    read_config();
    daemonize();
    run();
}

void Daemon::read_config(){
    std::string path = current_path + std::string("config.txt");

    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        exit;
    }

    std::getline(file, folder1_path);
    std::getline(file, folder2_path);
    std::string line;
    std::getline(file, line);
    time = std::stoi(line); 

    file.close();

}

void Daemon::daemonize(){
    pid_t pid;
    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    signal(SIGHUP, SIG_IGN);
    umask(0);
    chdir("/");
}

//maybe this must be in different file!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


std::vector<fs::path> Daemon::findLogs(){
    std::vector<fs::path> log_files;  

    for (const auto& entry : fs::recursive_directory_iterator(current_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".log") {
            log_files.push_back(entry.path()); 
        }
    }

    return log_files;
}

void Daemon::appendTotalLog(std::vector<fs::path>& log_files){
    std::ofstream totalLog(current_path + std::string("total.log")); 

    if (!totalLog) {
        std::cerr << "Error opening output file: " << current_path + std::string("total.log") << std::endl;
        return;
    }

    for (const auto& filePath : log_files) {
        std::ifstream inputFile(filePath);

        if (!inputFile) {
            std::cerr << "Error opening input file: " << filePath << std::endl;
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
            std::cout << "Removing directory: " << entry << std::endl;
            fs::remove(entry);  
        }
    }
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


void Daemon::run(){
    /*Удаляет все файлы “*.log” из папки 1, предварительно дописывая их содержимое в файл “total.log” папки 2 
    (перед записью содержимого, в “total.log” добавляются две пустые строки, пишется имя обрабатываемого файла из папки 1, 
    пропускается еще одна строка и уже потом записывается содержимое файла папки 1)*/
    
    std::vector<fs::path> log_files;
    
    while(true){
        log_files = findLogs();
        appendTotalLog(log_files);
        removeLogs(log_files);

        sleep(time);
    }
    
}