FLAGS = -Wall -pedantic

all: test

build/http.o: src/http.c
	clang -c src/http.c -o build/http.o ${FLAGS}

build/sploginternal.o: src/splog.c
	clang -c src/splog.c -o build/sploginternal.o ${FLAGS}

build/sploguser.o: src/sploguser.c
	clang -c src/sploguser.c -o build/sploguser.o ${FLAGS}

build/libsplog.a: build/http.o build/sploginternal.o build/sploguser.o
	ar rcs build/libsplog.a build/http.o build/sploginternal.o build/sploguser.o

test: test.c build/libsplog.a
	clang test.c -o test -L./build -lsplog ${FLAGS}