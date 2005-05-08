/*
 * Copyright (c) 2005 The DragonFly Project.  All rights reserved.
 * 
 * This code is derived from software contributed to The DragonFly Project
 * by Matthew Dillon <dillon@backplane.com>
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
 * 
 * $DragonFly: src/lib/libsys/genhooks/output_i386.c,v 1.1 2005/05/08 18:14:56 dillon Exp $
 */
/*
 * OUTPUT_I386.C
 *
 * Generate IA32 system call section code for the userland mapping 
 * area and for the actual map file.
 */

#include "defs.h"

void
output_user(FILE *fo)
{
    sys_info *sys;
    int i;

    fprintf(fo, "\t.section DragonFly_%s,\"ax\", @nobits\n", sys_sectname);
    fprintf(fo, "\t.p2align 12,0\n");

    for (i = 0; i < sys_count; ++i) {
	sys = sys_array[i];
	fprintf(fo, "\t.org\t%d * 64\n", i);
	fprintf(fo, "\t.globl __syscall_%d\n"
		    "__syscall_%d:\n",
		    i);
	if (sys) {
	    fprintf(fo, "\t.globl __syscall_%s\n"
			"__syscall_%s:\n",
			sys->func_ret->var_name,
			sys->func_ret->var_name);
	}
	fprintf(fo, "\n");
    }
    fprintf(fo, "\t.org\t%d * 64\n", i);
    fprintf(fo, "\t.p2align 12,0\n");
}

void
output_lib(FILE *fo)
{
    sys_info *sys;
    int i;

    fprintf(fo, "\t.section DragonFly_%s,\"ax\"\n", sys_sectname);
    fprintf(fo, "\t.p2align 12,0\n");

    for (i = 0; i < sys_count; ++i) {
	sys = sys_array[i];
	fprintf(fo, "\t.org\t%d * 64\n", i);
	fprintf(fo, "\t.globl __syscall_%d\n"
		    "__syscall_%d:\n",
		    i);
	if (sys) {
	    fprintf(fo, "\t.globl __syscall_%s\n"
			"__syscall_%s:\n",
			sys->func_ret->var_name,
			sys->func_ret->var_name);
	    fprintf(fo, "\tlea\t0x%x,%%eax\n"
			"\tint\t$0x80\n"
			"\tjb\t__syscall_errno_return\n"
			"\tret\n",
			i);
	}
	fprintf(fo, "\n");
    }
    fprintf(fo, "\t.org\t%d * 64\n", i);
    fprintf(fo, "__syscall_errno_return:\n"
		"\tmovl\t%%gs:12,%%edx\n"	/* XXX hardwired */
		"\tmovl\t%%eax,(%%edx)\n"
		"\tmovl\t$-1,%%eax\n"
		"\tmovl\t$-1,%%edx\n"
		"\tret\n");
    fprintf(fo, "\t.p2align 12,0\n");
}

