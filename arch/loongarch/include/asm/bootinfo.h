/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020-2022 Loongson Technology Corporation Limited
 */
#ifndef _ASM_BOOTINFO_H
#define _ASM_BOOTINFO_H

#include <linux/types.h>
#include <asm/setup.h>

const char *get_system_type(void);

extern void early_init(void);
extern void early_memblock_init(void);
extern void platform_init(void);

/*
 * Initial kernel command line, usually setup by fw_init_cmdline()
 */
extern char arcs_cmdline[COMMAND_LINE_SIZE];

/*
 * Registers a0, a1, a2 and a3 as passed to the kernel entry by firmware
 */
extern unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;

extern unsigned long initrd_start, initrd_end;

#endif /* _ASM_BOOTINFO_H */
