/**
 * @file rinoo_socket_timeout.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date Sun Jan 3 15:34:47 2010
 *
 * @brief Test file for timeout function.
 *
 *
 */
#include "rinoo/rinoo.h"

extern const t_rinoosocket_class socket_class_tcp;

void process_client(void *arg)
{
	char a;
	t_rinoosocket *socket = arg;

	rinoo_log("server - client accepted");
	rinoo_log("server - receiving nothing, waiting timeout");
	XTEST(rinoo_socket_read(socket, &a, 1) <= 0);
	rinoo_socket_destroy(socket);
}

void server_func(void *arg)
{
	t_rinoosocket *server;
	t_rinoosocket *client;
	struct sockaddr_in addr;
	t_rinoosched *sched = arg;

	server = rinoo_socket(sched, &socket_class_tcp);
	XTEST(server != NULL);
	addr.sin_port = htons(4242);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = 0;
	XTEST(rinoo_socket_bind(server, (struct sockaddr *) &addr, sizeof(addr), 42) == 0);
	rinoo_log("server listening...");
	client = rinoo_socket_accept(server, (struct sockaddr *) &addr, (socklen_t *)(int[]){(sizeof(struct sockaddr))});
	XTEST(client != NULL);
	rinoo_log("server - accepting client (%s:%d)", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	rinoo_task_start(sched, process_client, client);
	rinoo_socket_destroy(server);
}

void client_func(void *arg)
{
	char a;
	struct sockaddr_in addr;
	t_rinoosocket *socket;
	t_rinoosched *sched = arg;

	socket = rinoo_socket(sched, &socket_class_tcp);
	XTEST(socket != NULL);
	addr.sin_port = htons(4242);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = 0;
	XTEST(rinoo_socket_connect(socket, (struct sockaddr *) &addr, sizeof(addr)) == 0);
	rinoo_log("client - connected");
	rinoo_socket_timeout(socket, 1000);
	XTEST(rinoo_socket_read(socket, &a, 1) == -1);
	rinoo_log("server  - timeout");
	perror("socket");
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
