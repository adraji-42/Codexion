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

static void	set_ts(struct timespec *ts, long long diff_ms)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	tv.tv_usec += (diff_ms % 1000) * 1000;
	ts->tv_sec = tv.tv_sec + (diff_ms / 1000) + (tv.tv_usec / 1000000);
	ts->tv_nsec = (tv.tv_usec % 1000000) * 1000;
}

static void	wait_dongle(t_dongle *d, t_req *req)
{
	struct timespec	ts;
	long long		diff;
	long long		now;

	while (!check_stop(d->sim))
	{
		now = get_time_ms();
		if (!d->is_held && heap_peek(&d->heap).coder_id == req->coder_id)
		{
			if (now >= d->available_at)
				break ;
			diff = d->available_at - now;
			set_ts(&ts, diff);
			pthread_cond_timedwait(&d->cv, &d->mtx, &ts);
		}
		else
			pthread_cond_wait(&d->cv, &d->mtx);
	}
}

t_bool	dongle_take(t_dongle *d, t_coder *c)
{
	t_req	req;
	t_bool	acquired;

	acquired = FALSE;
	req.coder_id = c->id;
	req.created_at = get_time_ms();
	pthread_mutex_lock(&d->sim->state_mtx);
	req.deadline = c->last_compile_start + d->sim->t_burnout;
	pthread_mutex_unlock(&d->sim->state_mtx);
	pthread_mutex_lock(&d->mtx);
	heap_push(&d->heap, req);
	wait_dongle(d, &req);
	heap_pop_id(&d->heap, c->id);
	if (!check_stop(d->sim))
	{
		d->is_held = TRUE;
		acquired = TRUE;
		print_state(d->sim, c->id, "has taken a dongle");
	}
	pthread_mutex_unlock(&d->mtx);
	return (acquired);
}

void	dongle_release(t_dongle *d)
{
	pthread_mutex_lock(&d->mtx);
	d->is_held = FALSE;
	d->available_at = get_time_ms() + d->sim->d_cooldown;
	pthread_cond_broadcast(&d->cv);
	pthread_mutex_unlock(&d->mtx);
}
