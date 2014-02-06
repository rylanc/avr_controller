#!/usr/bin/python

import socket

#create an INET, STREAMing socket
s = socket.socket(
    socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 6666))
s.send("PLAY\n")
s.recv(1)
s.shutdown(1)
s.close()
