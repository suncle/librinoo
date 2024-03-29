/**
 * @file rinoo_socket_writev_ipv6.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2014
 * @date Sat May  3 20:55:02 2014
 *
 * @brief Test file for writev function.
 *
 *
 */
#include "rinoo/rinoo.h"

#define BUFFER_SIZE 2097152

extern const t_rinoosocket_class socket_class_tcp6;

static char *big_buffer;

void process_client(void *arg)
{
	int i;
	char b;
	t_buffer *buffers[4];
	t_buffer buffer[4];
	t_rinoosocket *socket = arg;

	buffer_static(&buffer[0], big_buffer, BUFFER_SIZE);
	buffers[0] = &buffer[0];
	rinoo_log("server - client accepted");
	rinoo_log("server - sending %d bytes", BUFFER_SIZE);
	XTEST(rinoo_socket_writev(socket, buffers, 1) == BUFFER_SIZE);
	rinoo_log("server - receiving 'b'");
	XTEST(rinoo_socket_read(socket, &b, 1) == 1);
	XTEST(b == 'b');
	for (i = 0; i < 4; i++) {
		buffer_static(&buffer[i], big_buffer + (i * (BUFFER_SIZE / 4)), BUFFER_SIZE / 4);
		buffers[i] = &buffer[i];
	}
	rinoo_log("server - sending %d bytes", BUFFER_SIZE);
	XTEST(rinoo_socket_writev(socket, buffers, 4) == BUFFER_SIZE);
	rinoo_log("server - receiving 'b'");
	XTEST(rinoo_socket_read(socket, &b, 1) == 1);
	XTEST(b == 'b');
	rinoo_log("server - receiving nothing");
	XTEST(rinoo_socket_read(socket, &b, 1) == -1);
	rinoo_socket_destroy(socket);
}

void server_func(void *arg)
{
	t_rinoosocket *server;
	t_rinoosocket *client;
	struct sockaddr_in6 addr = { 0 };
	t_rinoosched *sched = arg;

	server = rinoo_socket(sched, &socket_class_tcp6);
	XTEST(server != NULL);
	addr.sin6_port = htons(4242);
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = in6addr_any;
	XTEST(rinoo_socket_bind(server, (struct sockaddr *) &addr, sizeof(addr), 42) == 0);
	rinoo_log("server listening...");
	client = rinoo_socket_accept(server, NULL, NULL);
	XTEST(client != NULL);
	rinoo_task_start(sched, process_client, client);
	rinoo_socket_destroy(server);
}

void client_func(void *arg)
{
	int i;
	ssize_t res;
	ssize_t total;
	char tmp[8];
	struct sockaddr_in6 addr = { 0 };
	t_rinoosocket *socket;
	t_rinoosched *sched = arg;

	socket = rinoo_socket(sched, &socket_class_tcp6);
	XTEST(socket != NULL);
	addr.sin6_port = htons(4242);
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = in6addr_loopback;
	XTEST(rinoo_socket_connect(socket, (struct sockaddr *) &addr, sizeof(addr)) == 0);
	rinoo_log("client - connected");
	rinoo_log("client - reading %d bytes", BUFFER_SIZE);
	for (i = 0; i < BUFFER_SIZE / 8; i++) {
		res = 0;
		total = 0;
		while (total < 8 && (res = rinoo_socket_read(socket, tmp + total, 8 - total)) > 0) {
			total += res;
		}
		if (res < 0) {
			rinoo_log("Error: %s", strerror(errno));
		}
		XTEST(res > 0);
		XTEST(memcmp(tmp, "xxxxxxxx", 8) == 0);
	}
	rinoo_log("client - sending 'b'");
	rinoo_log("client - reading %d bytes", BUFFER_SIZE);
	XTEST(rinoo_socket_write(socket, "b", 1) == 1);
	for (i = 0; i < BUFFER_SIZE / 8; i++) {
		res = 0;
		total = 0;
		while (total < 8 && (res = rinoo_socket_read(socket, tmp + total, 8 - total)) > 0) {
			total += res;
		}
		if (res < 0) {
			rinoo_log("Error: %s", strerror(errno));
		}
		XTEST(res > 0);
		XTEST(memcmp(tmp, "xxxxxxxx", 8) == 0);
	}
	rinoo_log("client - sending 'b'");
	XTEST(rinoo_socket_write(socket, "b", 1) == 1);
	rinoo_socket_destroy(socket);
}

/**
 * Main function for this unit test.
 *
 * @return 0 if test passed
 */
int main()
{
	t_rinoosched *sched;

	big_buffer = malloc(sizeof(*big_buffer) * BUFFER_SIZE);
	XTEST(big_buffer != NULL);
	memset(big_buffer, 'x', sizeof(*big_buffer) * BUFFER_SIZE);
	sched = rinoo_sched();
	XTEST(sched != NULL);
	XTEST(rinoo_task_start(sched, server_func, sched) == 0);
	XTEST(rinoo_task_start(sched, client_func, sched) == 0);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	free(big_buffer);
	XPASS();
}
