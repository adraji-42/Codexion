/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adraji <adraji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 10:55:34 by adraji            #+#    #+#             */
/*   Updated: 2026/04/21 10:55:34 by adraji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	swap_req(t_req *a, t_req *b)
{
	t_req	temp;

	temp = *a;
	*a = *b;
	*b = temp;
}

void	heap_push(t_heap *h, t_req req)
{
	if (h->size >= CAP)
		return ;
	h->arr[h->size] = req;
	h->size++;
	if (h->sim->sched == EDF && h->size == 2)
	{
		if (h->arr[0].deadline > h->arr[1].deadline)
			swap_req(&h->arr[0], &h->arr[1]);
	}
}

void	heap_pop_id(t_heap *h, int id)
{
	int	i;

	i = 0;
	while (i < h->size && h->arr[i].coder_id != id)
		i++;
	if (i >= h->size)
		return ;
	if (i == 0 && h->size == 2)
		h->arr[0] = h->arr[1];
	h->size--;
}

t_req	heap_peek(t_heap *h)
{
	if (h->size > 0)
		return (h->arr[0]);
	return ((t_req){-1, -1, -1});
}
