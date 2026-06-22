/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adraji <adraji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 10:55:37 by adraji            #+#    #+#             */
/*   Updated: 2026/04/21 10:55:37 by adraji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	wait_dongels(t_coder *c)
{
	t_timems		target;
	t_timems		now;
	t_timems		wait_ms;

	if (c->first->available_at > c->second->available_at)
		target = c->first->available_at;
	else
		target = c->second->available_at;
	now = get_time_ms();
	if (target <= now)
		wait_ms = 1;
	else
		wait_ms = target - now;
	unlock_dongles(c);
	sys_sleep(wait_ms, c->sim);
	lock_dongles(c);
}

static t_bool	cleanup_and_unlock(t_coder *c)
{
	heap_pop_id(&c->first->heap, c->id);
	if (c->second != c->first)
		heap_pop_id(&c->second->heap, c->id);
	unlock_dongles(c);
	return (FALSE);
}

static t_bool	take_them(t_coder *c)
{
	c->first->is_held = TRUE;
	c->second->is_held = TRUE;
	c->first->available_at = get_time_ms() + c->first->sim->t_compile;
	c->second->available_at = get_time_ms() + c->second->sim->t_compile;
	cleanup_and_unlock(c);
	print_state(c->sim, c->id, "has taken a dongle");
	print_state(c->sim, c->id, "has taken a dongle");
	return (TRUE);
}

t_bool	dongles_take(t_coder *c)
{
	t_req	req;

	req.coder_id = c->id;
	req.created_at = get_time_ms();
	pthread_mutex_lock(&c->mtx);
	req.deadline = c->last_compile_start + c->sim->t_burnout;
	pthread_mutex_unlock(&c->mtx);
	lock_dongles(c);
	heap_push(&c->first->heap, req);
	if (c->second != c->first)
		heap_push(&c->second->heap, req);
	while (!check_stop(c->sim))
	{
		if (both_available(c))
			return (take_them(c));
		wait_dongels(c);
	}
	return (cleanup_and_unlock(c));
}

void	dongles_release(t_coder *c)
{
	t_timems	now;

	lock_dongles(c);
	now = get_time_ms();
	c->first->is_held = FALSE;
	c->first->available_at = now + c->first->sim->d_cooldown;
	pthread_cond_broadcast(&c->first->cv);
	if (c->second != c->first)
	{
		c->second->is_held = FALSE;
		c->second->available_at = now + c->second->sim->d_cooldown;
		pthread_cond_broadcast(&c->second->cv);
	}
	unlock_dongles(c);
}
