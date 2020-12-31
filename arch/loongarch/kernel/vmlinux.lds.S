/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/sizes.h>
#include <asm/asm-offsets.h>
#include <asm/thread_info.h>

#define PAGE_SIZE _PAGE_SIZE

/*
 * Put .bss..swapper_pg_dir as the first thing in .bss. This will
 * ensure that it has .bss alignment (64K).
 */
#define BSS_FIRST_SECTIONS *(.bss..swapper_pg_dir)

#include <asm-generic/vmlinux.lds.h>

OUTPUT_ARCH(loongarch)
ENTRY(kernel_entry)
PHDRS {
	text PT_LOAD FLAGS(7);	/* RWX */
	note PT_NOTE FLAGS(4);	/* R__ */
}

jiffies	 = jiffies_64;

SECTIONS
{
	. = VMLINUX_LOAD_ADDRESS;

	_text = .;
	.text : {
		TEXT_TEXT
		SCHED_TEXT
		CPUIDLE_TEXT
		LOCK_TEXT
		KPROBES_TEXT
		IRQENTRY_TEXT
		SOFTIRQENTRY_TEXT
		*(.fixup)
		*(.gnu.warning)
	} :text = 0
	_etext = .;

	EXCEPTION_TABLE(16)

	. = ALIGN(PAGE_SIZE);
	__init_begin = .;
	__inittext_begin = .;

	INIT_TEXT_SECTION(PAGE_SIZE)
	.exit.text : {
		EXIT_TEXT
	}

	__inittext_end = .;

	__initdata_begin = .;

	INIT_DATA_SECTION(16)
	.exit.data : {
		EXIT_DATA
	}

	__initdata_end = .;

	__init_end = .;

	_sdata = .;
	RO_DATA(4096)
	RW_DATA(1 << CONFIG_L1_CACHE_SHIFT, PAGE_SIZE, THREAD_SIZE)

	.sdata : {
		*(.sdata)
	}

	. = ALIGN(SZ_64K);
	_edata =  .;

	BSS_SECTION(0, SZ_64K, 8)

	_end = .;

	STABS_DEBUG
	DWARF_DEBUG

	.gptab.sdata : {
		*(.gptab.data)
		*(.gptab.sdata)
	}
	.gptab.sbss : {
		*(.gptab.bss)
		*(.gptab.sbss)
	}

	DISCARDS
	/DISCARD/ : {
		*(.gnu.attributes)
		*(.options)
		*(.eh_frame)
	}
}
