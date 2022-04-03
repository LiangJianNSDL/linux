#
# arch/loongarch/boot/Makefile
#
# Copyright (C) 2020-2022 Loongson Technology Corporation Limited
#

drop-sections := .comment .note .options .note.gnu.build-id
strip-flags   := $(addprefix --remove-section=,$(drop-sections)) -S

targets := vmlinux
quiet_cmd_strip = STRIP	  $@
      cmd_strip = $(STRIP) -s $@

$(obj)/vmlinux: vmlinux FORCE
	$(call if_changed,copy)
	$(call if_changed,strip)

targets += vmlinux.efi
quiet_cmd_eficopy = OBJCOPY $@
      cmd_eficopy = $(OBJCOPY) -O binary $(strip-flags) $< $@

$(obj)/vmlinux.efi: $(obj)/vmlinux FORCE
	$(call if_changed,eficopy)
