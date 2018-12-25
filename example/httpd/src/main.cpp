#include <stdio.h>
#include <string>
#include <signal.h>
#include "httpd.h"

int g_Quit = 0;
// ------------------------------------------------
void CtrlC_Handler (int e) {
    printf ("Ctrl-C Pressed!\r\n");
    g_Quit = 1;
}
// ------------------------------------------------
int main(int argc, char* argv[]) {
    lightcone::CmdArg args;

    signal (SIGINT, CtrlC_Handler);
    lightcone::Network::start();
    args.parse(argc, argv);

    auto server = new example::HttpServer();
    auto addr = lightcone::SockAddr(args.first("--address", "0.0.0.0"),
                                    std::stoi(args.first("--port", "8080")));
    server->start(-2);
    server->listen(addr);
    printf ("Listen on %s\n", addr.to_string().c_str());
    printf ("Access test uri with\n  curl -vvv http://127.0.0.1:%u/foo\n", addr.get_port());
    for (;!g_Quit;) {
        lightcone::Threads::usleep(100000);
    }
    server->stop();
    delete server;

    lightcone::Network::stop();
    return 0;
}
// ------------------------------------------------
