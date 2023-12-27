import threading
import socket
import time

THREAD_COUNT = 100

fail_count = 0
comp_count = 0
start = True

def make_req(url: str):
    data = b"GET " + url.encode("ascii") + b" HTTP/1.1\r\n\r\n"
    s = socket.socket()
    s.settimeout(1)
    try:
        s.connect(("localhost", 4000))
        s.send(data)
        resp = s.recv(1024).decode("utf-8")
        try:
            return_code = resp.split(" ")
            return return_code[1]
        except IndexError:
            print(resp)
            return "failed"
    except socket.timeout:
        return "timeout"

def req_thread(path, id):
    global fail_count, comp_count

    while not start:
        time.sleep(0.1)
    
    print(f"{id} started")

    ret = make_req(path)
    if (ret != "200"):
        print(f"{id} failed")
        fail_count += 1
    else:
        print(f"{id} finished")
    
    comp_count += 1

for i in range(THREAD_COUNT):
    threading.Thread(target=req_thread, args=["/megabyte", i]).start()

start = True

while (comp_count < THREAD_COUNT):
    time.sleep(1)

print("err rate", (fail_count/THREAD_COUNT)*100)