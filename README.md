# RiNOO
[![Build Status](https://drone.io/github.com/reginaldl/librinoo/status.png)](https://drone.io/github.com/reginaldl/librinoo/latest)

RiNOO is a socket management library. RiNOO sockets are asynchronous but "appear" synchronous.
This is possible by using fast-contexts (see fcontext project). Code looks simple. The complexity
of asynchronous sockets is hidden.
RiNOO is a simple way to create high scalability client/server applications.

## Documentation

* [Using librinoo for fun and profit](https://github.com/reginaldl/librinoo/wiki/Using-librinoo-for-fun-and-profit)
* [Libevent vs. RiNOO](https://github.com/reginaldl/librinoo/wiki/Libevent-vs.-RiNOO)

## Examples

### Hello world!

    #include "rinoo/rinoo.h"

    void task_client(void *socket)
    {
    	char a;

    	rinoo_socket_write(socket, "Hello world!\n", 13);
    	rinoo_socket_read(socket, &a, 1);
    	rinoo_socket_destroy(socket);
    }

    void task_server(void *sched)
    {
    	t_rinoosocket *server;
    	t_rinoosocket *client;

    	server = rinoo_tcp_server(sched, IP_ANY, 4242);
    	while ((client = rinoo_tcp_accept(server, NULL, NULL)) != NULL) {
    		rinoo_task_start(sched, task_client, client);
    	}
    	rinoo_socket_destroy(server);
    }

    int main()
    {
    	t_rinoosched *sched;

    	sched = rinoo_sched();
    	rinoo_task_start(sched, task_server, sched);
    	rinoo_sched_loop(sched);
    	rinoo_sched_destroy(sched);
    	return 0;
    }

### Multi-threading

    #include "rinoo/rinoo.h"

    void task_client(void *socket)
    {
    	char a;

    	rinoo_socket_write(socket, "Hello world!\n", 13);
    	rinoo_socket_read(socket, &a, 1);
    	rinoo_socket_destroy(socket);
    }

    void task_server(void *server)
    {
        t_rinoosched *sched;
    	t_rinoosocket *client;

        sched = rinoo_sched_self();
    	while ((client = rinoo_tcp_accept(server, NULL, NULL)) != NULL) {
                rinoo_log("Accepted connection on thread %d", sched->id);
                rinoo_task_start(sched, task_client, client);
    	}
    	rinoo_socket_destroy(server);
    }

    int main()
    {
        int i;
    	t_rinoosched *spawn;
    	t_rinoosched *sched;
    	t_rinoosocket *server;

    	sched = rinoo_sched();
    	server = rinoo_tcp_server(sched, IP_ANY, 4242);
        /* Spawning 10 schedulers, each running in a separate thread */
        rinoo_spawn(sched, 10);
        for (i = 1; i <= 10; i++) {
                spawn = rinoo_spawn_get(sched, i);
                rinoo_task_start(spawn, task_server, rinoo_socket_dup(spawn, server));
        }
        rinoo_task_start(sched, task_server, server);
    	rinoo_sched_loop(sched);
    	rinoo_sched_destroy(sched);
    	return 0;
    }

### HTTP

    #include "rinoo/rinoo.h"

    void http_client(void *sched)
    {
        t_rinoohttp http;
        t_rinoosocket *client;

        client = rinoo_tcp_client(sched, IP_LOOPBACK, 80, 0);
        rinoohttp_init(client, &http);
        rinoohttp_request_send(&http, RINOO_HTTP_METHOD_GET, "/", NULL);
        rinoohttp_response_get(&http);
        rinoo_log("client - %.*s", buffer_size(http.response.buffer), buffer_ptr(http.response.buffer));
        rinoohttp_destroy(&http);
        rinoo_socket_destroy(client);
    }

    int main()
    {
        t_rinoosched *sched;

        sched = rinoo_sched();
        rinoo_task_start(sched, http_client, sched);
        rinoo_sched_loop(sched);
        rinoo_sched_destroy(sched);
        return 0;
    }

### HTTP easy server

    #include "rinoo/rinoo.h"

    t_rinoohttp_route routes[] = {
        { "/", 200, RINOO_HTTP_ROUTE_STATIC, .content = "<html><body><center>Welcome to RiNOO HTTP server!<br/><br/><a href=\"/motd\">motd</a></center><body></html>" },
        { "/motd", 200, RINOO_HTTP_ROUTE_FILE, .file = "/etc/motd" },
        { NULL, 302, RINOO_HTTP_ROUTE_REDIRECT, .location = "/" }
    };

    int main()
    {
        t_rinoosched *sched;

        sched = rinoo_sched();
        rinoohttp_easy_server(sched, 0, 4242, routes, sizeof(routes) / sizeof(*routes));
        rinoo_sched_loop(sched);
        rinoo_sched_destroy(sched);
        return 0;
    }
