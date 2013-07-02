#!/usr/bin/python

import socket

#create an INET, STREAMing socket
s = socket.socket(
    socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 6666))
s.send("STOP\n")
s.shutdown(1)
s.close()
