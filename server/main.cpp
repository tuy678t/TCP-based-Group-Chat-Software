#include  "TCPServer.h"
#include <unistd.h>

int main(int argc, char* argv[])
{

    if (argc != 3) {
        printf("Usage: server ip port\n");
        return 1;
    }

    daemon(1, 1);

    const auto ip = argv[1];
    const auto port = atoi(argv[2]);
    TCPServer server(ip, port);

    server.loop();

    return 0;
}
