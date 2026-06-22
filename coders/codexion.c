/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adraji <adraji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 10:55:43 by adraji            #+#    #+#             */
/*   Updated: 2026/04/21 10:55:43 by adraji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	free_sim(t_sim *sim);

static t_bool	join_existing_coders(t_sim *sim, int i)
{
	pthread_mutex_lock(&sim->print_mtx);
	pthread_mutex_lock(&sim->state_mtx);
	sim->stop = TRUE;
	pthread_cond_broadcast(&sim->start_cv);
	if (i < sim->n_coders)
		fprintf(stderr,
			"ERROR: Failed to create thread for coder %d\n", sim->coders[i].id);
	else
		fprintf(stderr,
			"ERROR: Failed to create thread for monitor\n");
	pthread_mutex_unlock(&sim->state_mtx);
	pthread_mutex_unlock(&sim->print_mtx);
	while (i > 0)
		pthread_join(sim->coders[--i].thread, NULL);
	free_sim(sim);
	return (FALSE);
}

static t_bool	start_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n_coders)
	{
		if (pthread_create(
				&sim->coders[i].thread, NULL, coder_routine, &sim->coders[i]))
			return (join_existing_coders(sim, i));
		i++;
	}
	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim))
		return (join_existing_coders(sim, i));
	return (TRUE);
}

static void	cleanup_sim(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->state_mtx);
	sim->stop = TRUE;
	pthread_mutex_unlock(&sim->state_mtx);
	i = 0;
	while (i < sim->n_coders)
		pthread_join(sim->coders[i++].thread, NULL);
}

static void	free_sim(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->n_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].mtx);
		pthread_cond_destroy(&sim->dongles[i].cv);
		pthread_mutex_destroy(&sim->coders[i].mtx);
	}
	free(sim->dongles);
	free(sim->coders);
	pthread_mutex_destroy(&sim->print_mtx);
	pthread_mutex_destroy(&sim->state_mtx);
	pthread_cond_destroy(&sim->start_cv);
}

int	main(int ac, char **av)
{
	t_sim	sim;

	memset(&sim, 0, sizeof(t_sim));
	if (!parse_args(ac, av, &sim))
		return (FAILED);
	if (!init_sim(&sim))
	{
		fprintf(stderr, "ERROR: Init of simulation is FAILED");
		return (FAILED);
	}
	if (!start_threads(&sim))
		return (FAILED);
	pthread_join(sim.monitor, NULL);
	cleanup_sim(&sim);
	free_sim(&sim);
	return (SUCCESS);
}
