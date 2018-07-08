#include <stdio.h>
#include <string>
#include <signal.h>
#include "echoserver.h"
#include "tester.h"

int g_Quit = 0;
// ------------------------------------------------
void CtrlC_Handler (int e) {
    printf ("Ctrl-C Pressed!\r\n");
    g_Quit = 1;
}
// ------------------------------------------------
void usage() {
    printf("usage: ./example --mode=server|client ...\n"
           "server mode:\n"
           "  ./example --mode=server [--address=0.0.0.0] [--port=8000]\n"
           "client mode:\n"
           "  ./example --mode=client [--address=127.0.0.1] [--port=8000] [--concurrent=1024] [--packetsize=128]\n");
}
// ------------------------------------------------
int server_main(const lightcone::CmdArg& args) {
    auto server = new example::EchoServer();
    auto addr = lightcone::SockAddr(args.first("--address", "0.0.0.0"),
                                    std::stoi(args.first("--port", "8000")));
    server->start(-2);
    server->listen(addr);
    printf ("Listen on %s\n", addr.to_string().c_str());
    for (;!g_Quit;) {
        lightcone::Threads::usleep(100000);
    }
    server->stop();
    delete server;
    return 0;
}
// ------------------------------------------------
int client_main(const lightcone::CmdArg& args) {
    auto tester = new example::Tester();
    int concurrent = std::stoi(args.first("--concurrent", "1024"));
    size_t packetsize = (size_t)std::stoi(args.first("--packetsize", "128"));
    auto addr = lightcone::SockAddr(args.first("--address", "127.0.0.1"),
                                    std::stoi(args.first("--port", "8000")));
    tester->setup(concurrent, packetsize);
    tester->start(-2);
    printf ("Connect to %s for %d connections\n", addr.to_string().c_str(), concurrent);
    for (int i=0; i<concurrent; i++) {
        tester->connect(addr);
    }
    tester->packets = 0;
    for (;!g_Quit;) {
        lightcone::Threads::sleep(1);
        int connects = tester->connects;
        int packets = tester->packets;
        int timeouts = tester->timeouts;
        tester->packets = 0;
        printf ("Connected: %04d. Timeouts: %04d. Packets: %d, Bandwidth: %lu bytes/sec\n", connects, timeouts, packets, packetsize * packets);
    }
    tester->stop();
    delete tester;
    return 0;
}
// ------------------------------------------------
int main(int argc, char* argv[]) {
    lightcone::CmdArg args;
    int rez = 0;

    signal (SIGINT, CtrlC_Handler);
    lightcone::Network::start();
    args.parse(argc, argv);
    auto mode = args.first("--mode");
    if (mode == "server") {
        rez = server_main(args);
    } else if (mode == "client") {
        rez = client_main(args);
    } else {
        usage();
    }
    lightcone::Network::stop();
    return rez;
}
// ------------------------------------------------

