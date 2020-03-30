#!/usr/bin/env python3

import re
import getopt
import sys
import socket

EXIT_SUCCESS = 0
EXIT_USAGE_ERROR = 1

DEFAULT_PORT = 11111


def usage(exec_name):
    print("usage: %s -p port -s server_name -f file_name" % (exec_name))


def words_from_file(filename):
    try:
        f = open(filename, "r")
        words = re.split(r"[,.;:?\s]+", f.read())
        f.close()
        return words
    except IOError:
        print("Error opening %s for reading. Quitting" % (filename))
        exit()


def main():
    verbose = False
    filename = None
    hostname = "localhost"
    port = DEFAULT_PORT

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hf:s:p:v",
                                   ["help", "server=", "file=", "port="])
    except getopt.GetoptError as err:
        print("unrecognized option")
        usage(sys.argv[0])
        sys.exit(EXIT_USAGE_ERROR)
    output = None
    verbose = False
    for o, a in opts:
        if o == "-v":
            verbose = True
        elif o in ("-h", "--help"):
            usage(sys.argv[0])
            sys.exit()
        elif o in ("-f", "--file"):
            filename = a
        elif o in ("-s", "--server"):
            host = a
        elif o in ("-p", "--port"):
            try:
                port = int(a)
            except ValueError:
                print("port should be an integer")
                sys.exit(EXIT_USAGE_ERROR)
        else:
            assert False, "unhandled option"
    if verbose:
        print("using host %s:%d to spell check %s ..." %
              (hostname, port, filename))

    if filename == None:
        usage(sys.argv[0])
        sys.exit(EXIT_USAGE_ERROR)

    words = words_from_file(filename)

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    inf = s.makefile(mode="r")
    outf = s.makefile(mode="w")

    for word in words:
        outf.write("%s\n" % (word))
        outf.flush()
        line = inf.readline()
        print("%s" % (line))

    outf.close()
    inf.close()
    return EXIT_SUCCESS


if __name__ == "__main__":
    sys.exit(main())
