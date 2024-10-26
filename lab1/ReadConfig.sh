#!/bin/bash
sudo kill -SIGHUP $(cat /var/run/diy_daemon.pid)