#include <iostream>
#include "daemon.hpp"
int main() {
    Daemon::getInstance().start();
    return 0;
}