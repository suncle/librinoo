/**
 * @file   http.c
 * @author reginaldl <reginald.l@gmail.com> - Copyright 2013
 * @date   Mon Feb 25 14:48:20 2013
 *
 * @brief  http client/server unit test
 *
 *
 */

#include "rinoo/rinoo.h"

#define HTTP_CONTENT	"This content is for test purpose\n"

void http_client(void *sched)
{
	t_rinoohttp http;
	t_rinoosocket *client;

	client = rinoo_tcp_client(sched, IP_LOOPBACK, 4242, 0);
	XTEST(client != NULL);
	XTEST(rinoohttp_init(client, &http) == 0);
	XTEST(rinoohttp_request_send(&http, RINOO_HTTP_METHOD_GET, "/", NULL) == 0);
	XTEST(rinoohttp_response_get(&http));
	XTEST(buffer_size(&http.response.content) == strlen(HTTP_CONTENT));
	XTEST(http.response.code == 200);
	rinoohttp_destroy(&http);
	rinoo_socket_destroy(client);
}

void http_server_process(void *socket)
{
	t_buffer content;
	t_rinoohttp http;

	XTEST(rinoohttp_init(socket, &http) == 0);
	XTEST(rinoohttp_request_get(&http));
	http.response.code = 200;
	strtobuffer(&content, HTTP_CONTENT);
	XTEST(rinoohttp_response_send(&http, &content) == 0);
	rinoohttp_destroy(&http);
	rinoo_socket_destroy(socket);
}

void http_server(void *sched)
{
	t_ip ip;
	uint16_t port;
	t_rinoosocket *server;
	t_rinoosocket *client;

	server = rinoo_tcp_server(sched, IP_ANY, 4242);
	XTEST(server != NULL);
	client = rinoo_tcp_accept(server, &ip, &port);
	XTEST(client != NULL);
	rinoo_log("server - accepting client (%s:%d)", inet_ntoa(*(struct in_addr *) &ip), port);
	rinoo_task_start(sched, http_server_process, client);
	rinoo_socket_destroy(server);
}

/**
 * Main function for this unit test.
 *
 *
 * @return 0 if test passed
 */
int main()
{
	t_rinoosched *sched;

	sched = rinoo_sched();
	XTEST(sched != NULL);
	XTEST(rinoo_task_start(sched, http_server, sched) == 0);
	XTEST(rinoo_task_start(sched, http_client, sched) == 0);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	XPASS();
}
