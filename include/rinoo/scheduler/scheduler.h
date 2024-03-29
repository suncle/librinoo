/**
 * @file   scheduler.h
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Mon Dec 28 00:14:20 2009
 *
 * @brief  Header file for scheduler function declarations.
 *
 *
 */

#ifndef RINOO_SCHEDULER_SCHEDULER_H_
#define RINOO_SCHEDULER_SCHEDULER_H_

typedef struct s_rinoosched {
	int id;
	bool stop;
	t_rinoolist nodes;
	uint32_t nbpending;
	struct timeval clock;
	t_rinootask_driver driver;
	struct s_rinooepoll epoll;
	t_rinoosched_spawns spawns;
} t_rinoosched;

t_rinoosched *rinoo_sched(void);
void rinoo_sched_destroy(t_rinoosched *sched);
int rinoo_sched_spawn(t_rinoosched *sched, int count);
t_rinoosched *rinoo_sched_spawn_get(t_rinoosched *sched, int id);
t_rinoosched *rinoo_sched_self(void);
void rinoo_sched_stop(t_rinoosched *sched);
int rinoo_sched_waitfor(t_rinoosched_node *node,  t_rinoosched_mode mode);
int rinoo_sched_remove(t_rinoosched_node *node);
void rinoo_sched_wakeup(t_rinoosched_node *node, t_rinoosched_mode mode, int error);
int rinoo_sched_poll(t_rinoosched *sched);
void rinoo_sched_loop(t_rinoosched *sched);

#endif /* !RINOO_SCHEDULER_SCHEDULER_H */
