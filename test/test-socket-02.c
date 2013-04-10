#include <spin/socket.h>
#include <netinet/in.h>
#include <stdio.h>

spin_tcp_server_t srv;

void connected (spin_stream_t sock, const struct sockaddr_storage *s)
{
    printf ("connected\n");
}

int main()
{
    int fd = socket (PF_INET, SOCK_STREAM, 0);
	struct sockaddr_storage sockaddr;
	struct sockaddr_in *inaddr = (struct sockaddr_in *) &sockaddr;
	inaddr->sin_family = AF_INET;
	inaddr->sin_port = htons (4000);
	inaddr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    bind (fd, &sockaddr, sizeof(sockaddr));
	spin_loop_t loop = spin_loop_create ();
    srv = spin_tcp_server_from_fd (loop, fd, connected);
	spin_loop_run (loop);
	spin_loop_destroy (loop);
}
