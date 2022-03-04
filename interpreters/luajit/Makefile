############################################################################
# apps/interpreters/luajit/Makefile
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

LUAJIT_PATCHS ?= $(sort $(wildcard 000*.patch))

LUAJIT_VERSION  = 2.1.0-beta4
LUAJIT_UNPACK   = LuaJIT
LUAJIT_TARBALL  = v$(LUAJIT_VERSION).tar.gz
LUAJIT_URL_BASE = https://github.com/jturnsek/LuaJIT/archive
LUAJIT_URL      = $(LUAJIT_URL_BASE)/$(LUAJIT_TARBALL)

MAINSRC = luajit.c

CSRCS += lib_aux.c lib_base.c lib_bit.c lib_buffer.c lib_debug.c
CSRCS += lib_ffi.c lib_init.c lib_io.c lib_jit.c lib_math.c
CSRCS += lib_os.c lib_package.c lib_string.c lib_table.c
CSRCS += lj_alloc.c lj_api.c lj_asm.c lj_assert.c lj_bc.c
CSRCS += lj_bcread.c lj_bcwrite.c lj_buf.c lj_carith.c lj_ccall.c
CSRCS += lj_ccallback.c lj_cconv.c lj_cdata.c lj_char.c lj_clib.c
CSRCS += lj_cparse.c lj_crecord.c lj_ctype.c lj_debug.c lj_dispatch.c
CSRCS += lj_err.c lj_ffrecord.c lj_func.c lj_gc.c lj_gdbjit.c
CSRCS += lj_ir.c lj_lex.c lj_lib.c lj_load.c lj_mcode.c lj_meta.c
CSRCS += lj_obj.c lj_opt_dce.c lj_opt_fold.c lj_opt_loop.c
CSRCS += lj_opt_mem.c lj_opt_narrow.c lj_opt_sink.c lj_opt_split.c
CSRCS += lj_parse.c lj_prng.c lj_profile.c lj_record.c lj_serialize.c
CSRCS += lj_snap.c lj_state.c lj_str.c lj_strfmt.c lj_strfmt_num.c
CSRCS += lj_strscan.c lj_tab.c lj_trace.c lj_udata.c lj_vmevent.c
CSRCS += lj_vmmath.c

ASRCS += lj_vm.S

VPATH += $(LUAJIT_UNPACK)/dynasm
VPATH += $(LUAJIT_UNPACK)/src
VPATH += $(LUAJIT_UNPACK)/src/host
CFLAGS += -fomit-frame-pointer -fno-short-enums -DLUAJIT_OS=LUAJIT_OS_NUTTX
CFLAGS += -D__ARM_ARCH_7M__ -DLUAJIT_NO_UNWIND -DLUAJIT_DISABLE_PROFILE 
CFLAGS += -DLUAJIT_SECURITY_PRNG=0 -DLUAJIT_SECURITY_MCODE=0

PROGNAME  = luajit
PRIORITY  = $(CONFIG_INTERPRETERS_LUAJIT_PRIORITY)
STACKSIZE = $(CONFIG_INTERPRETERS_LUAJIT_STACKSIZE)
MODULE    = $(CONFIG_INTERPRETERS_LUAJIT)

$(LUAJIT_TARBALL):
	$(Q) echo "Downloading $(LUAJIT_TARBALL)"
	$(Q) curl -O -L $(LUAJIT_URL)

$(LUAJIT_UNPACK): $(LUAJIT_TARBALL)
	$(Q) echo "Unpacking $(LUAJIT_TARBALL) to $(LUAJIT_UNPACK)"
	$(Q) tar xzvf $(LUAJIT_TARBALL)
	$(Q) mv LuaJIT-$(LUAJIT_VERSION) $(LUAJIT_UNPACK)
	$(Q) cat $(LUAJIT_PATCHS) | \
		patch -s -N -d $(LUAJIT_UNPACK) -p1

$(LUAJIT_UNPACK)/.patch: $(LUAJIT_UNPACK)
	$(Q) touch $(LUAJIT_UNPACK)/.patch

CCOPTIONS= $(CCDEBUG) $(ASOPTIONS)
LDOPTIONS= $(CCDEBUG) $(LDFLAGS)

HOST_LUA=

HOST_XCFLAGS= -I./LuaJIT/src -I./LuaJIT/src/host
HOST_XLDFLAGS=
HOST_XLIBS=
HOST_ACFLAGS= $(CCOPTIONS) $(HOST_XCFLAGS) $(TARGET_ARCH) $(HOST_CFLAGS)
HOST_ALDFLAGS= $(LDOPTIONS) $(HOST_XLDFLAGS) $(HOST_LDFLAGS)
HOST_ALIBS= $(HOST_XLIBS) $(LIBS) $(HOST_LIBS)

MINILUA_O= host/minilua.o
MINILUA_LIBS= -lm
MINILUA_T= host/minilua
MINILUA_X= $(MINILUA_T)
HOST_LUA= $(MINILUA_X)
DASM_DEP= $(MINILUA_T)

DASM_DIR= ../dynasm
DASM= $(HOST_LUA) $(DASM_DIR)/dynasm.lua
DASM_XFLAGS=
DASM_AFLAGS=
DASM_ARCH= arm

DASM_AFLAGS+= -D ENDIAN_LE
DASM_AFLAGS+= -D JIT
DASM_AFLAGS+= -D FFI
DASM_AFLAGS+= -D DUALNUM
DASM_AFLAGS+= -D FPU
DASM_AFLAGS+= -D HFABI
DASM_AFLAGS+= -D NO_UNWIND
DASM_AFLAGS+= -D VER=70

DASM_FLAGS= $(DASM_XFLAGS) $(DASM_AFLAGS)
DASM_DASC= vm_$(DASM_ARCH).dasc

BUILDVM_O= host/buildvm.o host/buildvm_asm.o host/buildvm_peobj.o \
						host/buildvm_lib.o host/buildvm_fold.o
BUILDVM_T= host/buildvm
BUILDVM_X= $(BUILDVM_T)

HOST_O= $(MINILUA_O) $(BUILDVM_O)
HOST_T= $(MINILUA_T) $(BUILDVM_T)

LJVM_S= lj_vm.S
LJVM_O= lj_vm.o
LJVM_BOUT= $(LJVM_S)
LJVM_MODE= elfasm

LJLIB_O= lib_base.o lib_math.o lib_bit.o lib_string.o lib_table.o \
	 lib_io.o lib_os.o lib_package.o lib_debug.o lib_jit.o lib_ffi.o \
	 lib_buffer.o
LJLIB_C= $(LJLIB_O:.o=.c)

LIB_VMDEF= jit/vmdef.lua
LIB_VMDEFP= $(LIB_VMDEF)

ALL_HDRGEN= lj_bcdef.h lj_ffdef.h lj_libdef.h lj_recdef.h lj_folddef.h \
						host/buildvm_arch.h
ALL_GEN= $(LJVM_S) $(ALL_HDRGEN)

$(MINILUA_T): $(MINILUA_O)
	$(Q)$(HOSTCC) $(HOST_ALDFLAGS) -o $@ $(MINILUA_O) $(MINILUA_LIBS) $(HOST_ALIBS)

host/buildvm_arch.h: $(DASM_DASC) $(DASM_DEP) $(DASM_DIR)/*.lua lj_arch.h lua.h luaconf.h
	$(Q)$(DASM) $(DASM_FLAGS) -o $@ $(DASM_DASC)

host/buildvm.o: $(DASM_DIR)/dasm_armv7m.h

$(BUILDVM_T): $(BUILDVM_O)
	$(Q)$(HOSTCC) -m32 $(HOST_ALDFLAGS) -o $@ $(BUILDVM_O) $(HOST_ALIBS)

$(LJVM_BOUT): $(BUILDVM_T)
	$(Q)$(BUILDVM_X) -m $(LJVM_MODE) -o $@

lj_bcdef.h: $(BUILDVM_T) $(LJLIB_C)
	$(Q)$(BUILDVM_X) -m bcdef -o $@ $(LJLIB_C)

lj_ffdef.h: $(BUILDVM_T) $(LJLIB_C)
	$(Q)$(BUILDVM_X) -m ffdef -o $@ $(LJLIB_C)

lj_libdef.h: $(BUILDVM_T) $(LJLIB_C)
	$(Q)$(BUILDVM_X) -m libdef -o $@ $(LJLIB_C)

lj_recdef.h: $(BUILDVM_T) $(LJLIB_C)
	$(Q)$(BUILDVM_X) -m recdef -o $@ $(LJLIB_C)

$(LIB_VMDEF): $(BUILDVM_T) $(LJLIB_C)
	$(Q)$(BUILDVM_X) -m vmdef -o $(LIB_VMDEFP) $(LJLIB_C)

lj_folddef.h: $(BUILDVM_T) lj_opt_fold.c
	$(Q)$(BUILDVM_X) -m folddef -o $@ lj_opt_fold.c

$(HOST_O): %.o: %.c
	$(Q)$(HOSTCC) -m32 $(HOST_ACFLAGS) -c -o $@ $<

context:: $(LUAJIT_UNPACK)/.patch $(ALL_GEN)

distclean::
	$(call DELDIR, $(LUAJIT_UNPACK))
	$(call DELFILE, $(LUAJIT_TARBALL))

include $(APPDIR)/Application.mk
