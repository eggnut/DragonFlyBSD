/*
 * Copyright (c) 2011-2012 The DragonFly Project.  All rights reserved.
 *
 * This code is derived from software contributed to The DragonFly Project
 * by Matthew Dillon <dillon@dragonflybsd.org>
 * by Venkatesh Srinivas <vsrinivas@dragonflybsd.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of The DragonFly Project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific, prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "hammer2.h"

static void usage(int code);

int DebugOpt;
int VerboseOpt;
int NormalExit = 1;	/* if set to 0 main() has to pthread_exit() */

int
main(int ac, char **av)
{
	const char *sel_path = NULL;
	const char *uuid_str = NULL;
	const char *arg;
	int pfs_type = HAMMER2_PFSTYPE_NONE;
	int quick_opt = 0;
	int all_opt = 0;
	int ecode = 0;
	int ch;

	srandomdev();

	/*
	 * Core options
	 */
	while ((ch = getopt(ac, av, "adqs:t:u:v")) != -1) {
		switch(ch) {
		case 'a':
			all_opt = 1;
			break;
		case 'd':
			DebugOpt = 1;
			break;
		case 'q':
			/*
			 * Quick mode - do not block verifying certain
			 * operations such as (connect).
			 */
			quick_opt = 1;
			break;
		case 's':
			sel_path = optarg;
			break;
		case 't':
			/*
			 * set node type for mkpfs
			 */
			if (strcasecmp(optarg, "ADMIN") == 0) {
				pfs_type = HAMMER2_PFSTYPE_ADMIN;
			} else if (strcasecmp(optarg, "CACHE") == 0) {
				pfs_type = HAMMER2_PFSTYPE_CACHE;
			} else if (strcasecmp(optarg, "COPY") == 0) {
				pfs_type = HAMMER2_PFSTYPE_COPY;
			} else if (strcasecmp(optarg, "SLAVE") == 0) {
				pfs_type = HAMMER2_PFSTYPE_SLAVE;
			} else if (strcasecmp(optarg, "SOFT_SLAVE") == 0) {
				pfs_type = HAMMER2_PFSTYPE_SOFT_SLAVE;
			} else if (strcasecmp(optarg, "SOFT_MASTER") == 0) {
				pfs_type = HAMMER2_PFSTYPE_SOFT_MASTER;
			} else if (strcasecmp(optarg, "MASTER") == 0) {
				pfs_type = HAMMER2_PFSTYPE_MASTER;
			} else {
				fprintf(stderr, "-t: Unrecognized node type\n");
				usage(1);
			}
			break;
		case 'u':
			/*
			 * set uuid for mkpfs, else one will be generated
			 * (required for all except the MASTER node_type)
			 */
			uuid_str = optarg;
			break;
		case 'v':
			++VerboseOpt;
			break;
		default:
			fprintf(stderr, "Unknown option: %c\n", ch);
			usage(1);
			/* not reached */
			break;
		}
	}

	/*
	 * Adjust, then process the command
	 */
	ac -= optind;
	av += optind;
	if (ac < 1) {
		fprintf(stderr, "Missing command\n");
		usage(1);
		/* not reached */
	}

	if (strcmp(av[0], "connect") == 0) {
		/*
		 * Add cluster connection
		 */
		if (ac < 2) {
			fprintf(stderr, "connect: missing argument\n");
			usage(1);
		}
		ecode = cmd_remote_connect(sel_path, av[1]);
	} else if (strcmp(av[0], "disconnect") == 0) {
		/*
		 * Remove cluster connection
		 */
		if (ac < 2) {
			fprintf(stderr, "disconnect: missing argument\n");
			usage(1);
		}
		ecode = cmd_remote_disconnect(sel_path, av[1]);
	} else if (strcmp(av[0], "status") == 0) {
		/*
		 * Get status of PFS and its connections (-a for all PFSs)
		 */
		ecode = cmd_remote_status(sel_path, all_opt);
	} else if (strcmp(av[0], "pfs-list") == 0) {
		/*
		 * List all PFSs
		 */
		ecode = cmd_pfs_list(sel_path);
	} else if (strcmp(av[0], "pfs-create") == 0) {
		/*
		 * Create new PFS using pfs_type
		 */
		if (ac < 2) {
			fprintf(stderr, "pfs-create: requires name\n");
			usage(1);
		}
		ecode = cmd_pfs_create(sel_path, av[1], pfs_type, uuid_str);
	} else if (strcmp(av[0], "pfs-delete") == 0) {
		/*
		 * Delete a PFS by name
		 */
		if (ac < 2) {
			fprintf(stderr, "pfs-delete: requires name\n");
			usage(1);
		}
		ecode = cmd_pfs_delete(sel_path, av[1]);
	} else if (strcmp(av[0], "snapshot") == 0) {
		/*
		 * Create snapshot with optional pfs-type and optional
		 * label override.
		 */
	} else if (strcmp(av[0], "service") == 0) {
		/*
		 * Start the service daemon.  This daemon accepts
		 * connections from local and remote clients, handles
		 * the security handshake, and manages the core messaging
		 * protocol.
		 */
		ecode = cmd_service();
	} else if (strcmp(av[0], "stat") == 0) {
		ecode = cmd_stat(ac - 1, (const char **)&av[1]);
	} else if (strcmp(av[0], "leaf") == 0) {
		/*
		 * Start the management daemon for a specific PFS.
		 *
		 * This will typically connect to the local master node
		 * daemon, register the PFS, and then pass its side of
		 * the socket descriptor to the kernel HAMMER2 VFS via an
		 * ioctl().  The process and/or thread context remains in the
		 * kernel until the PFS is unmounted or the connection is
		 * lost, then returns from the ioctl.
		 *
		 * It is possible to connect directly to a remote master node
		 * instead of the local master node in situations where
		 * encryption is not desired or no local master node is
		 * desired.  This is not recommended because it represents
		 * a single point of failure for the PFS's communications.
		 *
		 * Direct kernel<->kernel communication between HAMMER2 VFSs
		 * is theoretically possible for directly-connected
		 * registrations (i.e. where the spanning tree is degenerate),
		 * but not recommended.  We specifically try to reduce the
		 * complexity of the HAMMER2 VFS kernel code.
		 */
		ecode = cmd_leaf(sel_path);
	} else if (strcmp(av[0], "shell") == 0) {
		/*
		 * Connect to the command line monitor in the hammer2 master
		 * node for the machine using HAMMER2_DBG_SHELL messages.
		 */
		ecode = cmd_shell((ac < 2) ? NULL : av[1]);
	} else if (strcmp(av[0], "rsainit") == 0) {
		/*
		 * Initialize a RSA keypair.  If no target directory is
		 * specified we default to "/etc/hammer2".
		 */
		arg = (ac < 2) ? HAMMER2_DEFAULT_DIR : av[1];
		ecode = cmd_rsainit(arg);
	} else if (strcmp(av[0], "rsaenc") == 0) {
		/*
		 * Encrypt the input symmetrically by running it through
		 * the specified public and/or private key files.
		 *
		 * If no key files are specified data is encoded using
		 * "/etc/hammer2/rsa.pub".
		 *
		 * WARNING: no padding is added, data stream must contain
		 *	    random padding for this to be secure.
		 *
		 * Used for debugging only
		 */
		if (ac == 1) {
			const char *rsapath = HAMMER2_DEFAULT_DIR "/rsa.pub";
			ecode = cmd_rsaenc(&rsapath, 1);
		} else {
			ecode = cmd_rsaenc((const char **)&av[1], ac - 1);
		}
	} else if (strcmp(av[0], "rsadec") == 0) {
		/*
		 * Decrypt the input symmetrically by running it through
		 * the specified public and/or private key files.
		 *
		 * If no key files are specified data is decoded using
		 * "/etc/hammer2/rsa.prv".
		 *
		 * WARNING: no padding is added, data stream must contain
		 *	    random padding for this to be secure.
		 *
		 * Used for debugging only
		 */
		if (ac == 1) {
			const char *rsapath = HAMMER2_DEFAULT_DIR "/rsa.prv";
			ecode = cmd_rsadec(&rsapath, 1);
		} else {
			ecode = cmd_rsadec((const char **)&av[1], ac - 1);
		}
	} else if (strcmp(av[0], "show") == 0) {
		/*
		 * Raw dump of filesystem.  Use -v to check all crc's, and
		 * -vv to dump bulk file data.
		 */
		if (ac != 2) {
			fprintf(stderr, "show: requires device path\n");
			usage(1);
		} else {
			cmd_show(av[1]);
		}
	} else {
		fprintf(stderr, "Unrecognized command: %s\n", av[0]);
		usage(1);
	}

	/*
	 * In DebugMode we may wind up starting several pthreads in the
	 * original process, in which case we have to let them run and
	 * not actually exit.
	 */
	if (NormalExit) {
		return (ecode);
	} else {
		pthread_exit(NULL);
		_exit(2);	/* NOT REACHED */
	}
}

static
void
usage(int code)
{
	fprintf(stderr,
		"hammer2 [-s path] command...\n"
		"    -s path            Select filesystem\n"
		"    -t type            PFS type for pfs-create\n"
		"    -u uuid            uuid for pfs-create\n"
		"\n"
		"    connect <target>   Add cluster link\n"
		"    disconnect <target> Del cluster link\n"
		"    status             Report cluster status\n"
		"    pfs-list           List PFSs\n"
		"    pfs-create <label> Create a PFS\n"
		"    pfs-delete <label> Destroy a PFS\n"
		"    snapshot           Snapshot a PFS\n"
		"    service            Start service daemon\n"
		"    stat [<path>]	Return inode quota & config\n"
		"    leaf               Start pfs leaf daemon\n"
		"    shell [<host>]     Connect to debug shell\n"
		"    rsainit            Initialize rsa fields\n"
		"    show devpath       Raw hammer2 media dump\n"
	);
	exit(code);
}
