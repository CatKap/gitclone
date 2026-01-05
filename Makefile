LIBS=-lssh2 -lssl -lcrypto -lz -lpthread -ldl -lpcre

all:
	gcc gitclone.c ./libgit2/libgit2.a $(LIBS) -o gitclone


























