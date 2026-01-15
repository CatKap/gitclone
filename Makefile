LIBS=-lssh2 -lssl -lcrypto -lz -lpthread -ldl -lpcre

static:
	gcc gitclone.c  -I./libgit2/include ./libgit2/build/libgit2.a $(LIBS) -Wl,--gc-sections -O3 -o gitclone

dynamic:
	gcc gitclone.c -lgit2 -DUSE_SYSTEM_LIBRARY $(LIBS) -o gitclone































