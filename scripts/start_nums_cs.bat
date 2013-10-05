@echo off
start ncat -u -l -C localhost 9911
python send_udp_nums.py localhost 9911 100