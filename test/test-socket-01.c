#include <spin/socket.h>
#include <netinet/in.h>
#include <stdio.h>


struct spin_io_req out_req;
struct spin_io_req in_req;
char buff[100];
spin_stream_t sock;

void read_complete (struct spin_io_req *in_req)
{
	printf ("read completed\n");
}

void write_complete (struct spin_io_req *out_req)
{
	printf ("write completed\n");
	in_req.buff = buff;
	in_req.size = 0;
	in_req.minsize = 0;
	in_req.maxsize = sizeof (buff);
	in_req.callback = read_complete;
	spin_stream_read (sock, &in_req);
}

void callback (spin_stream_t socket)
{
	sock = socket;
	printf ("contected\n");
	out_req.buff = "1234";
	out_req.size = 0;
	out_req.maxsize = 4;
	out_req.minsize = 4;
	out_req.callback = write_complete;
	spin_stream_write (socket, &out_req);
}


int main()
{
	struct sockaddr_storage sockaddr;
	struct sockaddr_in *inaddr = (struct sockaddr_in *) &sockaddr;
	inaddr->sin_family = AF_INET;
	inaddr->sin_port = htons (4000);
	inaddr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	spin_loop_t loop = spin_loop_create ();
	spin_tcp_connect (loop, &sockaddr, callback);
	spin_loop_run (loop);
	spin_loop_destroy (loop);
}
