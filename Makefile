LIBS=-lssh2 -lssl -lcrypto -lz -lpthread -ldl -lpcre

static:
	gcc gitclone.c ./libgit2/build/libgit2.a $(LIBS) -o gitclone

dynamic:
	gcc gitclone.c -lgit2 $(LIBS) -o gitclone































