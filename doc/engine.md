# Network Engine Architecture

### Engine
The network engine manage TCP connections and perform polling on multi-threaded environment using optimal implementation
for the platform: `epoll()`, `kqueue()` or `select()`.

```
┌─────────────┐  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
│   callback  │  │   listen    │ │    accept   │ │   connect   │
└──────┬──────┘  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘
       │                │               └───────┬───────┘
       │                │                ┌──────┴──────┐
       │                │                │   balancer  │
       │                │                └──────┬──────┘
       │                │               ┌───────┴───────┐
       │         ┌──────┴──────┐ ┌──────┴──────┐ ┌──────┴──────┐
       │         │   poller0   │ │   poller1   │ │   poller2   │ ... (threads)
       │         └──────┬──────┘ └──────┬──────┘ └──────┬──────┘
       │         ┌──────┴──────┐ ┌──────┴──────┐ ┌──────┴──────┐
       │         │  ev:accept  │ │  ev:r/w/c   │ │  ev:r/w/c   │
       │         └──────┬──────┘ └──────┬──────┘ └──────┬──────┘
       └────────────────┴───────────────┴───────────────┘
```
### Socket
Each socket has one input and one output buffer. Output buffer will be consumed and flushed, input buffer will be filled
by the engine. Programmer do not need to cater for incomplete send as in for BSD socket.
```
┌─────────────┐
│ TCP Socket  │
├─────────────┤
│ ibuf        │
├─────────────┤
│ obuf        │
├─────────────┤
│ user-data   │
└─────────────┘
```

### Events
While the engine utilize multi-threaded model, multiple events of same socket (e.g. read+write) will not occur in
multiple callback threads simultaneously, but will be invoked one by one on same thread. Developer may also work with the
triggering socket object during callback without lock.


### Protocol Helper / Middleware
*lightcone* provide certain handy protocol handler like *http*, which can be integrated with the engine to deal with
different protocol transport. For instant, [http example](../example/httpd) demonstrate serving http api with just few
lines of code chaining the event handlers.
