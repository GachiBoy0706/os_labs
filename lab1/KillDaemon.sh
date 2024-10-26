#!/bin/bash
sudo kill -SIGTERM $(cat /var/run/diy_daemon.pid)