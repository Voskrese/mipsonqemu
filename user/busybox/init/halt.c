/* vi: set sw=4 ts=4: */
/*
 * Mini halt implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/reboot.h>
#include <config/autoconf.h>
#include "busybox.h"
#include "init_shared.h"

#define OPT_DELAY 1
#define OPT_FORCE 2

extern int halt_main(int argc, char **argv)
{
	unsigned long opt;
	char *delay; /* delay in seconds before halting */

	opt = bb_getopt_ulflags(argc, argv, "d:f", &delay);
	if (opt & OPT_DELAY) {
		sleep(atoi(delay));
	}
#ifdef CONFIG_USER_FLATFSD_FLATFSD
	if (!(opt & OPT_FORCE)) {
		/* Ask flatfsd to halt us safely */
		if (system("exec /bin/flatfsd -H") != -1) {
			exit(0);
		}
	}
#endif

#ifndef CONFIG_INIT
#ifndef RB_HALT_SYSTEM
#define RB_HALT_SYSTEM		0xcdef0123
#endif
	return(bb_shutdown_system(RB_HALT_SYSTEM));
#else
	return kill_init(SIGUSR1);
#endif
}
