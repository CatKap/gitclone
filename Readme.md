# Gitclone 
---

This util is using only for one purpose - clone git repo from server. Supports agent-based ssh autheinfication and https.

## Quick install
---
Just download it, and use it. 
```
cd /usr/bin/ && wget https://github.com/CatKap/gitclone/releases/download/release/gitclone && chmod +x ./gitclone
gitclone 
```


## Dependenses 
---
Used libgit2, if you want to compile binay by yourself, you shoud or install the libgit2 shared library, or comlipe from sourse.

Also use 


### Libgit2 
---
Download packages required for the build: 

```apt installcmake libssl-dev zlib1g-dev```

Download and compile like this:
```
git clone https://github.com/libgit2/libgit2.git
cd libgit2
cd -rf build
mkdir build && cd build

cmake .. \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_TESTS=OFF \
  -DBUILD_CLI=OFF \
  -DUSE_GSSAPI=OFF \
  -DUSE_NTLMCLIENT=OFF \
  -DREGEX_BACKEND=SYSTEM \
  -DPCRE_LIBRARY=/usr/local/lib/libpcre.a \
  -DPCRE_INCLUDE_DIR=/usr/local/include \
  -DUSE_HTTPS=ON \
  -DUSE_SSH=ON \
  -DUSE_BUNDLED_ZLIB=OFF \
  -DUSE_BUNDLED_OPENSSL=OFF \
  -DUSE_BUNDLED_LIBSSH2=OFF
make 
```

### Compilation
---
If you want compile from source, use make utility.
```
make static # Compile libgit2 first, like mentntion previosly
make dynamic # For dynamic executable (requires libgit2 installed)
```
