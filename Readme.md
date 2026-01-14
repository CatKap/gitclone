![GitHub release](https://img.shields.io/github/v/aarch64/CatKap/gitclone)

# 🚀 Gitclone

**Gitclone** is a lightweight utility designed for a single purpose — cloning Git repositories from a server. It supports both agent-based SSH authentication and HTTPS.

---


## Quick Install

Download and install the binary:

```bash
cd /usr/bin/
wget https://github.com/CatKap/gitclone/releases/download/release/gitclone
chmod +x ./gitclone
gitclone
```

## Dependencies
Gitclone relies on libgit2. If you want to compile the binary yourself, you must either install the libgit2 shared library or compile it from source.

### Installing Libgit2

Install the required packages for building:


```apt install cmake libssl-dev zlib1g-dev libpcre3-dev pkg-config```

Clone and build libgit2:
```
git clone https://github.com/libgit2/libgit2.git
cd libgit2
rm -rf build
mkdir build && cd build

cmake .. \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_TESTS=OFF \
  -DBUILD_CLI=OFF \
  -DUSE_GSSAPI=OFF \
  -DUSE_NTLMCLIENT=OFF \
  -DPCRE_LIBRARY=/usr/local/lib/libpcre.a \
  -DPCRE_INCLUDE_DIR=/usr/local/include \
  -DUSE_HTTPS=ON \
  -DUSE_SSH=ON \
  -DUSE_BUNDLED_ZLIB=OFF \
  -DUSE_BUNDLED_OPENSSL=OFF \
  -DUSE_BUNDLED_LIBSSH2=OFF
make 
```
or install it via 

```
apt install libgit2-dev
```

### Compilation
If you want compile from source, use the `make` utility.
```
make static # Compile libgit2 first, like mentntion previosly
make dynamic # For dynamic executable (requires libgit2 installed)
```
