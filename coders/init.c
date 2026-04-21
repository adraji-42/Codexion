/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adraji <adraji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 10:55:32 by adraji            #+#    #+#             */
/*   Updated: 2026/04/21 10:55:32 by adraji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_bool	destroy_dongles(t_sim *sim, int i, t_bool mtx_init)
{
	if (mtx_init)
		pthread_mutex_destroy(&sim->dongles[i].mtx);
	while (i-- > 0)
	{
		pthread_mutex_destroy(&sim->dongles[i].mtx);
		pthread_cond_destroy(&sim->dongles[i].cv);
	}
	free(sim->dongles);
	sim->dongles = NULL;
	free(sim->coders);
	sim->coders = NULL;
	return (FALSE);
}

static int	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->num_coders)
	{
		if (pthread_mutex_init(&sim->dongles[i].mtx, NULL))
			return (destroy_dongles(sim, i, FALSE));
		if (pthread_cond_init(&sim->dongles[i].cv, NULL))
			return (destroy_dongles(sim, i, TRUE));
		sim->dongles[i].id = i;
		sim->dongles[i].is_held = FALSE;
		sim->dongles[i].available_at = 0;
		sim->dongles[i].sim = sim;
		sim->dongles[i].heap.size = 0;
		sim->dongles[i].heap.sim = sim;
		i++;
	}
	return (TRUE);
}

static int	init_coders(t_sim *sim)
{
	int	i;
	int	left;
	int	right;

	i = 0;
	while (i < sim->num_coders)
	{
		left = i;
		right = (i + 1) % sim->num_coders;
		sim->coders[i].sim = sim;
		sim->coders[i].id = i + 1;
		sim->coders[i].compile_count = 0;
		if (!(sim->coders[i].id % 2))
		{
			sim->coders[i].d1 = &sim->dongles[left];
			sim->coders[i].d2 = &sim->dongles[right];
		}
		else
		{
			sim->coders[i].d1 = &sim->dongles[right];
			sim->coders[i].d2 = &sim->dongles[left];
		}
		i++;
	}
	return (TRUE);
}

int	init_sim(t_sim *sim)
{
	sim->dongles = malloc(sizeof(t_dongle) * sim->num_coders);
	if (!sim->dongles)
		return (FALSE);
	sim->coders = malloc(sizeof(t_coder) * sim->num_coders);
	if (!sim->coders)
	{
		free(sim->dongles);
		return (FALSE);
	}
	if (pthread_mutex_init(&sim->print_mtx, NULL))
	{
		free(sim->dongles);
		free(sim->coders);
		return (FALSE);
	}
	if (pthread_mutex_init(&sim->state_mtx, NULL))
	{
		pthread_mutex_destroy(&sim->print_mtx);
		free(sim->dongles);
		free(sim->coders);
		return (FALSE);
	}
	return (init_dongles(sim) && init_coders(sim));
}
