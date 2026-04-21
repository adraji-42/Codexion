/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adraji <adraji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 10:55:25 by adraji            #+#    #+#             */
/*   Updated: 2026/04/21 10:55:25 by adraji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_bool	incomplit_args(char *program_name)
{
	fprintf(
		stderr, "ERROR:"
		"\nUsage: %s number_of_coders time_to_burnout time_to_compile "
		"time_to_debug time_to_refactor number_of_compiles_required "
		"dongle_cooldown scheduler\n", program_name
		);
	return (FALSE);
}

static t_bool	invalid_arg(char *name, char *value)
{
	fprintf(stderr, "ERROR: Invalid argument '%s' (%s)\n", name, value);
	return (FALSE);
}

static t_bool	parse_int(const char *str, int *out)
{
	long	res;
	int		i;

	res = 0;
	i = 0;
	if (!str[i])
		return (FALSE);
	if (str[i] == '+')
		i++;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (FALSE);
		res = res * 10 + (str[i] - '0');
		if (res > 2147483647)
			return (FALSE);
		i++;
	}
	*out = (int)res;
	return (TRUE);
}

t_bool	parse_args(int ac, char **av, t_sim *s)
{
	if (ac != 9)
		return (incomplit_args(av[0]));
	if (!parse_int(av[1], &s->num_coders) || s->num_coders <= 0)
		return (invalid_arg("number_of_coders", av[1]));
	if (!parse_int(av[2], &s->t_burnout) || s->t_burnout < 0)
		return (invalid_arg("time_to_burnout", av[2]));
	if (!parse_int(av[3], &s->t_compile) || s->t_compile < 0)
		return (invalid_arg("time_to_compile", av[3]));
	if (!parse_int(av[4], &s->t_debug) || s->t_debug < 0)
		return (invalid_arg("time_to_debug", av[4]));
	if (!parse_int(av[5], &s->t_refactor) || s->t_refactor < 0)
		return (invalid_arg("time_to_refactor", av[5]));
	if (!parse_int(av[6], &s->target_compiles) || s->target_compiles <= 0)
		return (invalid_arg("number_of_compiles_required", av[6]));
	if (!parse_int(av[7], &s->d_cooldown) || s->d_cooldown < 0)
		return (invalid_arg("dongle_cooldown", av[7]));
	if (!strcmp(av[8], "fifo"))
		s->sched = FIFO;
	else if (!strcmp(av[8], "edf"))
		s->sched = EDF;
	else
		return (invalid_arg("scheduler", av[8]));
	return (TRUE);
}
