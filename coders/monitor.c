/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adraji <adraji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 10:55:29 by adraji            #+#    #+#             */
/*   Updated: 2026/04/21 10:55:29 by adraji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	coder_dead(t_coder *c)
{
	long long	now;
	long long	limit;
	int			dead;

	dead = FALSE;
	pthread_mutex_lock(&c->sim->state_mtx);
	limit = c->last_compile_start + c->sim->t_burnout;
	now = get_time_ms();
	if (now >= limit && !c->sim->sim_stop)
	{
		c->sim->sim_stop = 1;
		dead = TRUE;
	}
	pthread_mutex_unlock(&c->sim->state_mtx);
	if (dead)
	{
		pthread_mutex_lock(&c->sim->print_mtx);
		printf("%lld %d burned out\n", now - c->sim->start_time, c->id);
		pthread_mutex_unlock(&c->sim->print_mtx);
	}
	return (dead);
}

static int	all_compiled(t_sim *sim)
{
	int	i;
	int	done;

	if (sim->target_compiles < 0)
		return (0);
	i = 0;
	done = 1;
	pthread_mutex_lock(&sim->state_mtx);
	while (i < sim->num_coders)
	{
		if (sim->coders[i].compile_count < sim->target_compiles)
		{
			done = 0;
			break ;
		}
		i++;
	}
	if (done && !sim->sim_stop)
		sim->sim_stop = 1;
	pthread_mutex_unlock(&sim->state_mtx);
	return (done);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		i;

	sim = (t_sim *)arg;
	while (!check_stop(sim))
	{
		i = 0;
		while (i < sim->num_coders)
		{
			if (coder_dead(&sim->coders[i]))
				return (NULL);
			i++;
		}
		if (all_compiled(sim))
			return (NULL);
		usleep(1000);
	}
	return (NULL);
}
