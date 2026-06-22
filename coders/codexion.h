/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adraji <adraji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 10:55:40 by adraji            #+#    #+#             */
/*   Updated: 2026/04/21 10:55:40 by adraji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>
# include <string.h>

# define CAP 2

# define EDF 1
# define FIFO 0

# define TRUE 1
# define FALSE 0

# define FAILED 1
# define SUCCESS 0

typedef struct timeval	t_tv;
typedef size_t			t_timems;

typedef struct s_sim	t_sim;
typedef char			t_sched;
typedef char			t_bool;

typedef struct s_requirements
{
	int			coder_id;
	t_timems	created_at;
	t_timems	deadline;
}	t_req;

typedef struct s_heap
{
	t_req	arr[CAP];
	int		size;
	t_sim	*sim;
}	t_heap;

typedef struct s_dongle
{
	pthread_mutex_t	mtx;
	pthread_cond_t	cv;
	int				id;
	t_bool			is_held;
	t_timems		available_at;
	t_heap			heap;
	t_sim			*sim;
}	t_dongle;

typedef struct s_coder
{
	pthread_t		thread;
	pthread_mutex_t	mtx;
	int				id;
	t_timems		last_compile_start;
	int				compile_count;
	t_dongle		*first;
	t_dongle		*second;
	t_sim			*sim;
}	t_coder;

struct s_sim
{
	int				n_coders;
	int				t_burnout;
	int				t_compile;
	int				t_debug;
	int				t_refactor;
	int				target_compiles;
	int				d_cooldown;
	t_sched			sched;
	t_timems		start_time;
	t_bool			stop;
	t_bool			start;
	pthread_mutex_t	print_mtx;
	pthread_mutex_t	state_mtx;
	pthread_cond_t	start_cv;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor;
};

t_bool		init_sim(t_sim *sim);
t_bool		parse_args(int ac, char **av, t_sim *s);

t_timems	get_time_ms(void);
t_bool		sys_sleep(t_timems duration, t_sim *s);
void		print_state(t_sim *sim, int id, const char *msg);
t_bool		check_stop(t_sim *sim);

void		heap_push(t_heap *h, t_req req);
void		heap_pop_id(t_heap *h, int id);
t_req		heap_peek(t_heap *h);

t_bool		dongles_take(t_coder *c);
void		dongles_release(t_coder *c);

void		lock_dongles(t_coder *c);
void		unlock_dongles(t_coder *c);
t_bool		both_available(t_coder *c);

void		*coder_routine(void *arg);

void		*monitor_routine(void *arg);

#endif
