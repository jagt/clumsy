# send incrementing packets containing numbers to given host
# start up a server by using ncat
# the static 5.59BETA1 version would work
# http://nmap.org/dist/ncat-portable-5.59BETA1.zip
# the server should be started with CRLF as EOF
# eg: ncat -u -l -C localhost 9111
import subprocess
import argparse
from time import sleep
from sys import argv, exit

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='ncat num sender')
    parser.add_argument('host', type=str)
    parser.add_argument('port', type=str)
    parser.add_argument('-s', '--sleep', default=100, type=int, help='sleep time', required=False)
    parser.add_argument('--nosleep', help='nosleep', action='store_true')
    parser.add_argument('--tcp', help='use tcp instead of udp', action='store_true')
    args = parser.parse_args()

    cmd = ['ncat', '-u', '-C', args.host, args.port]
    if args.tcp:
        cmd.remove('-u')
    ncat = subprocess.Popen(cmd, stdin=subprocess.PIPE)

    cnt = 1
    while True: # send till die
        ncat.stdin.write('%s\r\n' % ('-' * (1 + (cnt % 8))))
        #ncat.stdin.write('%d\r\n' % (cnt % 100))
        cnt += 1
        print cnt
        if not args.nosleep:
            sleep(args.sleep/1000.0)




