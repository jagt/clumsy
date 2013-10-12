# send incrementing packets containing numbers to given host
# start up a server by using ncat
# the static 5.59BETA1 version would work
# http://nmap.org/dist/ncat-portable-5.59BETA1.zip
# the server should be started with CRLF as EOF
# eg: ncat -u -l -C localhost 9111
import subprocess
from time import sleep
from sys import argv, exit

if __name__ == '__main__':
    if len(argv) < 3:
        print "usage : python send_udp_nums.py <host> <port> [<period-in-ms>]"
        exit(1)
    if len(argv) == 3: argv.append('400')
    host, port, period = argv[1:]
    period = int(period)
    cnt = 0
    ncat = subprocess.Popen(['ncat', '-u', '-C', host, port], stdin=subprocess.PIPE)
    while True: # send till die
        ncat.stdin.write('%d\r\n' % (cnt % 100))
        cnt += 1
        print cnt
        sleep(period/1000.0)




