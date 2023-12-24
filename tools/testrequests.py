import socket

http_req = "POST /fuck HTTP/1.1\r\nContent-Length: 10\r\n\r\nShit"

s = socket.socket()
s.connect(("localhost", 4000))
s.send(http_req.encode("utf-8"))
s.close()