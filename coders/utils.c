/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adraji <adraji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 10:55:22 by adraji            #+#    #+#             */
/*   Updated: 2026/04/21 10:55:22 by adraji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_timems	get_time_ms(void)
{
	t_tv	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

t_bool	check_stop(t_sim *sim)
{
	t_bool	stop;

	pthread_mutex_lock(&sim->state_mtx);
	stop = sim->stop;
	pthread_mutex_unlock(&sim->state_mtx);
	return (stop);
}

t_bool	sys_sleep(t_timems duration, t_sim *sim)
{
	t_timems		start;

	start = get_time_ms();
	while (get_time_ms() - start < duration)
	{
		if (check_stop(sim))
			return (FALSE);
		usleep(100);
	}
	return (TRUE);
}

void	print_state(t_sim *sim, int id, const char *msg)
{
	t_timems	time;

	pthread_mutex_lock(&sim->print_mtx);
	time = get_time_ms() - sim->start_time;
	pthread_mutex_lock(&sim->state_mtx);
	if (!sim->stop)
		printf("%zu %d %s\n", time, id, msg);
	pthread_mutex_unlock(&sim->state_mtx);
	pthread_mutex_unlock(&sim->print_mtx);
}
