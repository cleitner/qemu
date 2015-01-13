##############################################################################
#
# qemu_bootrom: 
# needed to boot like a zynq in QEMU
#
##############################################################################
ifndef qemu_bootrom.guard
qemu_bootrom.guard := 1
$(eval $(call enter_build_component,qemu_bootrom))

qemu_bootrom.license	:= Festo
qemu_bootrom.maintainer	:= clei

qemu_bootrom.deploy_dir	:= $(FBUILD_DEPLOY_BASE_DIR)/qemu_bootrom

qemu_bootrom.build_result	:= \
	$(qemu_bootrom.build_dir)/qemu_bootrom.elf

qemu_bootrom.provides	:= \
	$(qemu_bootrom.deploy_dir)/qemu_bootrom.elf

qemu_bootrom.cflags		+= -march=armv7-a -mcpu=cortex-a9 -fpic -g -O0
qemu_bootrom.ldflags 	+= -nostartfiles -nodefaultlibs -nostdlib -Wl,-Ttext=0xFFFF0000

##############################################################################

$(qemu_bootrom.build_dir)/qemu_bootrom.elf: $(qemu_bootrom.src_dir)/bootrom.c $(qemu_bootrom.src_dir)/print.c
	$(echo-build)
	$(Q)mkdir -p $(@D)
	$(Q)$(qemu_bootrom.cc) $(qemu_bootrom.cflags) $(qemu_bootrom.ldflags) -o $@ $^

$(qemu_bootrom.deploy_dir)/qemu_bootrom.elf: $(qemu_bootrom.build_dir)/qemu_bootrom.elf
	$(echo-build)
	$(Q)mkdir -p $(@D)
	$(Q)cp $< $@

##############################################################################
$(eval $(call leave_build_component))
endif # guard

