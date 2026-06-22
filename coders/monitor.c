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

static t_bool	is_burnout(t_sim *sim, t_coder *c)
{
	t_timems	now;

	now = get_time_ms();
	pthread_mutex_lock(&c->mtx);
	if (now < c->last_compile_start + sim->t_burnout)
	{
		pthread_mutex_unlock(&c->mtx);
		return (FALSE);
	}
	pthread_mutex_unlock(&c->mtx);
	pthread_mutex_lock(&sim->print_mtx);
	pthread_mutex_lock(&sim->state_mtx);
	sim->stop = TRUE;
	pthread_mutex_unlock(&sim->state_mtx);
	printf("%lu %d burned out\n", get_time_ms() - sim->start_time, c->id);
	pthread_mutex_unlock(&sim->print_mtx);
	return (TRUE);
}

static t_bool	is_finished(t_sim *sim, t_coder *c)
{
	t_bool	finished;

	finished = FALSE;
	pthread_mutex_lock(&c->mtx);
	if (c->compile_count >= sim->target_compiles)
		finished = TRUE;
	pthread_mutex_unlock(&c->mtx);
	return (finished);
}

static t_sim	*start_simulation(t_sim *sim)
{
	int	i;

	i = 0;
	pthread_mutex_lock(&sim->state_mtx);
	sim->start_time = get_time_ms();
	sim->start = TRUE;
	while (i < sim->n_coders)
		sim->coders[i++].last_compile_start = sim->start_time;
	pthread_cond_broadcast(&sim->start_cv);
	pthread_mutex_unlock(&sim->state_mtx);
	return (sim);
}

void	*monitor_routine(void *arg)
{
	int		i;
	int		finished_count;
	t_sim	*sim;

	sim = start_simulation((t_sim *)arg);
	while (TRUE)
	{
		i = -1;
		finished_count = 0;
		while (++i < sim->n_coders)
		{
			if (is_burnout(sim, &sim->coders[i]))
				return (NULL);
			finished_count += is_finished(sim, &sim->coders[i]);
		}
		if (finished_count == sim->n_coders)
		{
			pthread_mutex_lock(&sim->state_mtx);
			sim->stop = TRUE;
			pthread_mutex_unlock(&sim->state_mtx);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
