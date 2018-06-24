# LightCoNE
> lightweight c++ optimized network engine

[![C++ Version][cpp-image]][cpp-url]
[![TravisCI][travis-image]][travis-url]

`LightCoNE` is a c++ server development kit. In the heart it consist of a network engine, along with some higher level facilities for developing inter-connected server cluster.

### Use Case
- Service platform with real-time interaction between users
- Game Servers

#### Network Engine
The network engine provides a multi-threaded event callback architecture. It utilize best approach on specific platform: `epoll` on linux, `kqueue` on bsd and mac osx. On Windows and solaris we fall back to `select`. See [engine architecture](doc/engine.md) for detail.

#### High Level Facility
`High Level Facility` enable easy communication and scaling of server cluster. A brief list of features:
- service discovery to allow hot-plug of server
- graceful handling of remote server up & down
- channel-based inter-server communication
- client session management and packet routing

## How To Build
```
cd liblightcone
./configure
make && make test
```

<!-- Markdown link & img dfn's -->
[cpp-image]: https://img.shields.io/badge/c%2B%2B-14-blue.svg
[cpp-url]: https://en.wikipedia.org/wiki/C%2B%2B14
[travis-image]: https://travis-ci.org/shadow-paw/lightcone.svg?branch=master
[travis-url]: https://travis-ci.org/shadow-paw/lightcone
