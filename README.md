# LightCoNE
> lightweight c++ optimized network engine

[![C++ Version][cpp-image]][cpp-url]
[![doc][doc-image]][doc-url]
[![TravisCI][travis-image]][travis-url]  
[![BSD][target-bsd-image]](#bsd)
[![Linux][target-linux-image]](#linux)
[![Mac][target-mac-image]](#mac-os-x)
[![Windows][target-win-image]](#windows)

`LightCoNE` is a c++ server development kit. It consist of a network engine, along with some higher level facilities for
developing inter-connected server cluster.

### Use Case
- Service platform with real-time interaction between users
- Game Servers

#### Network Engine
The network engine provides a multi-threaded event callback architecture. It utilize best approach on specific platform:
`epoll` on linux, `kqueue` on bsd and mac osx. On Windows and solaris we fall back to `select`. See
[engine architecture](doc/engine.md) for detail.

#### Protocol
The kit provides conventent tools to deal with http and websocket.

#### Server Cluster
Easy communication between cluster of servers. A brief list of features:
- service discovery to allow hot-plug of server
- graceful handling of server up & down
- channel-based inter-server communication
- client session management and packet routing

## Performance
*Capacity & Performance* is a key goal for lightcone, and we have some stunning results.
- [http server](example/httpd) built on top of lightcone.

## How To Build

#### Windows
Download [Visual Studio Community 2017](https://visualstudio.microsoft.com/downloads/). Open *lightcone.sln*.

#### Mac OS X
Install [xcode](https://developer.apple.com/xcode/) and command line tool.
```
cd lightcone
./configure
make
make test
```

#### Linux
Install gcc-7 or above.
```
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install gcc-8 g++-8
```
> NOTE: Ubuntu 18.04 comes with gcc-7 which supports c++17, you may skip the above installation.
```
cd lightcone
./configure
CC=gcc-8 CXX=g++-8 make
CC=gcc-8 CXX=g++-8 make test
```

#### BSD
Install gcc-8 with port.
```
> su
# cd /usr/port/lang/gcc8
# make install clean
```
> NOTE: FreeBSD 12 comes with gcc-7 which supports c++17, you may skip the above installation.
```
cd lightcone
./configure
gmake
gmake test
```

<!-- Markdown link & img dfn's -->
[cpp-image]: https://img.shields.io/badge/c%2B%2B-17-blue.svg
[cpp-url]: https://en.wikipedia.org/wiki/C%2B%2B14
[doc-image]: https://img.shields.io/badge/doc-doxygen-orange.svg
[doc-url]: https://shadow-paw.github.io/lightcone/
[travis-image]: https://travis-ci.org/shadow-paw/lightcone.svg?branch=master
[travis-url]: https://travis-ci.org/shadow-paw/lightcone
[target-bsd-image]: https://img.shields.io/badge/target-bsd-blue.svg
[target-linux-image]: https://img.shields.io/badge/target-linux-blue.svg
[target-mac-image]: https://img.shields.io/badge/target-mac-blue.svg
[target-win-image]: https://img.shields.io/badge/target-windows-blue.svg
