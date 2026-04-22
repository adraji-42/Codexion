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

long long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

t_bool	check_stop(t_sim *sim)
{
	t_bool	stop;

	pthread_mutex_lock(&sim->state_mtx);
	stop = sim->sim_stop;
	pthread_mutex_unlock(&sim->state_mtx);
	return (stop);
}

t_bool	sys_sleep(long long duration, t_sim *sim)
{
	long long		start;

	start = get_time_ms();
	while (get_time_ms() - start < duration)
	{
		if (check_stop(sim))
			return (FALSE);
		usleep(1000);
	}
	return (TRUE);
}

void	print_state(t_sim *sim, int id, const char *msg)
{
	long long	time;

	pthread_mutex_lock(&sim->print_mtx);
	time = get_time_ms() - sim->start_time;
	if (!check_stop(sim))
		printf("%lld %d %s\n", time, id, msg);
	pthread_mutex_unlock(&sim->print_mtx);
}
