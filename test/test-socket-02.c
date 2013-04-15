#include <spin/socket.h>
#include <spin/timer.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

spin_tcp_server_t srv;
spin_stream_t sock;
void read_complete (struct spin_io_req *in_req);
void write_complete (struct spin_io_req *out_req);

struct client_socket {
    spin_stream_t stream;
    struct spin_io_req out_req;
    struct spin_io_req in_req;
    char buff[100];
};

spin_timer_t timer_close_tcp_server;

void callback_close_tcp_server (void *param)
{
    spin_tcp_server_destroy (srv);
	return 0;
}

void write_complete (struct spin_io_req *out_req)
{
    //printf ("write completed\n");
    struct client_socket *p = (void*)
        (((int8_t*)out_req) - offsetof(struct client_socket, out_req));
    p->in_req.buff = p->buff;
    p->in_req.size = 0;
    p->in_req.minsize = 0;
    p->in_req.maxsize = sizeof (p->buff);
    p->in_req.callback = read_complete;
    spin_stream_read (p->stream, &p->in_req);
}

void read_complete (struct spin_io_req *in_req)
{
    //printf ("read completed\n");
    struct client_socket *p = (void*)
        (((int8_t*)in_req) - offsetof(struct client_socket, in_req));
    p->out_req.buff = p->buff;
    p->out_req.size = 0;
    p->out_req.maxsize = in_req->size;
    p->out_req.minsize = in_req->size;
    p->out_req.callback = write_complete;
    spin_stream_write (p->stream, &p->out_req);
}

void connected (spin_stream_t s, const struct sockaddr_storage *addr)
{
    printf ("connected\n");
    struct client_socket *p = malloc (sizeof (*p));
    p->stream = s;
    write_complete (&p->out_req);
}

int main()
{
    int fd = socket (PF_INET, SOCK_STREAM, 0);
    struct sockaddr_storage sockaddr;
    struct sockaddr_in *inaddr = (struct sockaddr_in *) &sockaddr;
	struct spin_itimespec x;
    inaddr->sin_family = AF_INET;
    inaddr->sin_port = htons (4000);
    inaddr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    int value = 1;
    setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
    bind (fd, &sockaddr, sizeof(sockaddr));
    spin_loop_t loop = spin_loop_create ();
    srv = spin_tcp_server_from_fd (loop, fd, connected);
	x.initial = 10000;
	x.interval = 0;
    timer_close_tcp_server = spin_timer_create (loop,
                                                callback_close_tcp_server,
                                                NULL);
	spin_timer_ctl (timer_close_tcp_server, &x, NULL);
    spin_loop_run (loop);
    spin_loop_destroy (loop);
}
