/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adraji <adraji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 10:55:46 by adraji            #+#    #+#             */
/*   Updated: 2026/04/21 10:55:46 by adraji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_bool	coder_compile(t_coder *c)
{
	pthread_mutex_lock(&c->mtx);
	c->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&c->mtx);
	print_state(c->sim, c->id, "is compiling");
	if (!sys_sleep(c->sim->t_compile, c->sim))
	{
		dongles_release(c);
		return (FALSE);
	}
	pthread_mutex_lock(&c->mtx);
	c->compile_count++;
	pthread_mutex_unlock(&c->mtx);
	dongles_release(c);
	return (TRUE);
}

static t_bool	wait_start(t_sim *sim)
{
	pthread_mutex_lock(&sim->state_mtx);
	while (!sim->start && !sim->stop)
		pthread_cond_wait(&sim->start_cv, &sim->state_mtx);
	pthread_mutex_unlock(&sim->state_mtx);
	if (check_stop(sim))
		return (FALSE);
	return (TRUE);
}

void	*coder_routine(void *arg)
{
	t_coder	*c;

	c = (t_coder *)arg;
	if (!wait_start(c->sim))
		return (NULL);
	if (!(c->id % 2))
		sys_sleep(c->sim->t_compile + c->sim->d_cooldown, c->sim);
	else if (c->id == c->sim->n_coders)
		sys_sleep((c->sim->t_compile + c->sim->d_cooldown) * 2, c->sim);
	while (TRUE)
	{
		if (!dongles_take(c))
			break ;
		if (!coder_compile(c))
			break ;
		print_state(c->sim, c->id, "is debugging");
		if (!sys_sleep(c->sim->t_debug, c->sim))
			break ;
		print_state(c->sim, c->id, "is refactoring");
		if (!sys_sleep(c->sim->t_refactor, c->sim))
			break ;
	}
	return (NULL);
}
