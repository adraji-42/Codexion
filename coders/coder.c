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

static void	coder_compile(t_coder *c)
{
	pthread_mutex_lock(&c->sim->state_mtx);
	c->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&c->sim->state_mtx);
	print_state(c->sim, c->id, "is compiling");
	sys_sleep(c->sim->t_compile, c->sim);
	pthread_mutex_lock(&c->sim->state_mtx);
	c->compile_count++;
	pthread_mutex_unlock(&c->sim->state_mtx);
	dongle_release(c->d2);
	dongle_release(c->d1);
}

void	*coder_routine(void *arg)
{
	t_coder	*c;

	c = (t_coder *)arg;
	while (!check_stop(c->sim))
	{
		if (!dongle_take(c->d1, c))
			break ;
		if (!dongle_take(c->d2, c))
		{
			dongle_release(c->d1);
			break ;
		}
		coder_compile(c);
		print_state(c->sim, c->id, "is debugging");
		sys_sleep(c->sim->t_debug, c->sim);
		print_state(c->sim, c->id, "is refactoring");
		sys_sleep(c->sim->t_refactor, c->sim);
	}
	return (NULL);
}
