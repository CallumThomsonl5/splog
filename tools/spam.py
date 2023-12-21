import requests


i = 0
while True:
    r = requests.get("http://localhost:4000/test?a=b&b=c&d=e&count=" + str(i))
    print("done")
    i+=1