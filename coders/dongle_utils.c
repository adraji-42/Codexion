/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adraji <adraji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 03:50:04 by adraji            #+#    #+#             */
/*   Updated: 2026/05/03 18:45:57 by adraji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	lock_dongles(t_coder *c)
{
	pthread_mutex_lock(&c->first->mtx);
	if (c->second != c->first)
		pthread_mutex_lock(&c->second->mtx);
}

void	unlock_dongles(t_coder *c)
{
	if (c->second != c->first)
		pthread_mutex_unlock(&c->second->mtx);
	pthread_mutex_unlock(&c->first->mtx);
}

t_bool	both_available(t_coder *c)
{
	t_timems	now;

	now = get_time_ms();
	if (c->first == c->second)
		return (FALSE);
	return (!c->first->is_held && !c->second->is_held
		&& now >= c->first->available_at && now >= c->second->available_at
		&& heap_peek(&c->first->heap).coder_id == c->id
		&& heap_peek(&c->second->heap).coder_id == c->id);
}
