@echo off
REM in fact ncat isn't strictly send each CRLF as a single packet, when period is 10ms it would put
REM multiple lines together, needs to write a script
start ncat -u -l -C localhost 9911
python send_udp_nums.py -s 100 localhost 9911

