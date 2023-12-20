FLAGS = 

all: test.exe

build/http.o: src/http.c
	clang -c src/http.c -o build/http.o ${FLAGS}

build/sploginternal.o: src/splog.c
	clang -c src/splog.c -o build/sploginternal.o ${FLAGS}

build/sploguser.o: src/sploguser.c
	clang -c src/sploguser.c -o build/sploguser.o ${FLAGS}

build/splog.lib: build/http.o build/sploginternal.o build/sploguser.o
	ar rcs build/splog.lib build/http.o build/sploginternal.o build/sploguser.o

test.exe: test.c build/splog.lib
	clang test.c -o test.exe -L./build -lsplog ${FLAGS}