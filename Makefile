CC = clang

all: build_dir
all: lib_chess

lib_chess: .build/lib.o
	$(CC) -shared -o .build/libchess.so $^

.build/%.o: backend/%.c
	$(CC) -c -fpic -o $@ $<

build_dir:
	@[ -d .build ] || mkdir .build
