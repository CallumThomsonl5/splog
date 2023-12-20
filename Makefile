FLAGS = 

all: test.exe


http.o: src/http.c
	clang -c src/http.c -o http.o ${FLAGS}

libsplog.o: src/splog.c
	clang -c src/splog.c -o libsplog.o ${FLAGS}

sploguser.o: src/sploguser.c
	clang -c src/sploguser.c -o sploguser.o ${FLAGS}

test.exe: test.c libsplog.o http.o sploguser.o
	clang test.c libsplog.o http.o sploguser.o -o test.exe ${FLAGS}