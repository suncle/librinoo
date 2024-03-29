/**
 * @file rinoo_socket_write_ipv6.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2014
 * @date Sun Jan 3 15:34:47 2010
 *
 * @brief Test file for read/write functions.
 *
 *
 */
#include "rinoo/rinoo.h"

extern const t_rinoosocket_class socket_class_tcp6;

void process_client(void *arg)
{
	char b;
	t_rinoosocket *socket = arg;

	rinoo_log("server - client accepted");
	rinoo_log("server - sending 'abcdef'");
	XTEST(rinoo_socket_write(socket, "abcdef", 6) == 6);
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
	char a;
	char cur;
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
	for (cur = 'a'; cur <= 'f'; cur++) {
		rinoo_log("client - receiving '%c'", cur);
		XTEST(rinoo_socket_read(socket, &a, 1) == 1);
		XTEST(a == cur);
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

	sched = rinoo_sched();
	XTEST(sched != NULL);
	XTEST(rinoo_task_start(sched, server_func, sched) == 0);
	XTEST(rinoo_task_start(sched, client_func, sched) == 0);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	XPASS();
}
