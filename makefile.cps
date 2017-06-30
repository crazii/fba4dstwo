# Makefile for FBA, for use with GNU make & GCC (Cygwin/MinGW)
#
# The first pass makes sure all intermediary targets are present. The second pass updates
# any targets, if necessary. (Intermediary) targets which have their own unique rules
# are generated as required.

unexport

NDS_FW_VERSION = 200
VERSION_MINOR = 1
VERSION_MAJOR = 0

#
#	Flags. Uncomment any of these declarations to enable their function.
#

# Check for changes in header files
# DEPEND = 1

#
#	Declare variables
#

# Specify the name of the executable file, without ".exe"
BINDIR = bin/cps
ELF = ${BINDIR}/fba4dstwo.elf
BIN = $(ELF:.elf=.bin)
NAME = _dstwoplug/fba4dstwo.plg

#ifndef	CPUTYPE
#	CPUTYPE	= i686
#endif

#
#	Specify paths/files
#

objdir	= obj/nds/
srcdir	= src/

alldir	= burn generated nds zlib \
cpu cpu/c68k cpu/cz80 burn/capcom cpu/m6809 cpu/hd6309 cpu/m6800

incdir	= -I. -I$(NDSSDK)/include $(foreach dir,$(alldir),-I$(srcdir)$(dir)) -I$(objdir)generated

lib		=  -lds2a -lds2b -lc -lm -lgcc
#-Wl,-plugin-opt=-pass-through=-lc -Wl,-plugin-opt=-pass-through=-lm -Wl,-plugin-opt=-pass-through=-lgcc
		
drvobj	= d_cps1.o d_cps2.o


depobj	:= 	$(drvobj) \
	   	\
	   	burn.o burn_gun.o load.o \
	   	\
	   	sek.o zet.o eeprom_93cxx.o \
	   	burn_sound.o burn_sound_c.o timer.o \
		burn_ym2151.o \
	   	ym2151.o \
	   	msm6295.o \
		m6800.o m6800_intf.o m6809.o m6809_intf.o hd6309.o hd6309_intf.o\
		cps.o cps_config.o cps_draw.o cps_mem.o cps_obj.o cps_pal.o cps_run.o \
	   	cps2_crpt.o cps_rw.o cps_scr.o cpsr.o cpsrd.o \
	   	cpst.o ctv.o ps.o ps_m.o ps_z.o qs.o qs_c.o qs_z.o \
	   	kabuki.o \
	   	adler32.o compress.o crc32.o deflate.o inffast.o inflate.o inftrees.o trees.o uncompr.o zutil.o \
	   	\
	   	ds2_main.o main.o swapBuffer.o drv.o input.o state.o statec.o unzip.o zipfn.o bzip.o font.o \
	   	roms.o ui.o gui.o snd.o
	   	
autobj += $(depobj)

# burn_sound_mips.o

autdep	= $(depobj:.o=.d)

#a68k.o	= $(objdir)cpu/a68k/a68k.o
#dozea.o	= $(objdir)cpu/doze/dozea.o
#app_gnuc.rc = $(srcdir)generated/app_gnuc.rc
#license.rtf = $(srcdir)generated/license.rtf

a68k.o	= $(objdir)cpu/a68k.mips/a68k.o

#autobj += a68k.o

driverlist.h = $(srcdir)generated/driverlist.h
ctv.h	= $(srcdir)generated/ctv.h
toa_gp9001_func.h = $(srcdir)generated/toa_gp9001_func.h
neo_sprite_func.h = $(srcdir)generated/neo_sprite_func.h
cave_tile_func.h = $(srcdir)generated/cave_tile_func.h
cave_sprite_func.h = $(srcdir)generated/cave_sprite_func.h
psikyo_tile_func.h = $(srcdir)generated/psikyo_tile_func.h
build_details.h = $(srcdir)generated/build_details.h

allobj	= \
	  $(foreach file,$(autobj:.o=.c), \
		$(foreach dir,$(alldir),$(subst $(srcdir),$(objdir), \
		$(firstword $(subst .c,.o,$(wildcard $(srcdir)$(dir)/$(file))))))) \
	  $(foreach file,$(autobj:.o=.cpp), \
		$(foreach dir,$(alldir),$(subst $(srcdir),$(objdir), \
		$(firstword $(subst .cpp,.o,$(wildcard $(srcdir)$(dir)/$(file))))))) \
	  $(foreach file,$(autobj:.o=.s), \
		$(foreach dir,$(alldir),$(subst $(srcdir),$(objdir), \
		$(firstword $(subst .s,.o,$(wildcard $(srcdir)$(dir)/$(file))))))) \
	  $(foreach file,$(autobj:.o=.S), \
		$(foreach dir,$(alldir),$(subst $(srcdir),$(objdir), \
		$(firstword $(subst .S,.o,$(wildcard $(srcdir)$(dir)/$(file))))))) \

#$(a68k.o) $(objdir)cpu/m68k/m68kcpu.o $(objdir)cpu/m68k/m68kopnz.o $(objdir)cpu/m68k/m68kopdm.o $(objdir)cpu/m68k/m68kopac.o $(objdir)cpu/m68k/m68kops.o

alldep	= $(foreach file,$(autobj:.o=.c), \
		$(foreach dir,$(alldir),$(subst $(srcdir),$(objdir), \
		$(firstword $(subst .c,.d,$(wildcard $(srcdir)$(dir)/$(file))))))) \
	  $(foreach file,$(autobj:.o=.cpp), \
		$(foreach dir,$(alldir),$(subst $(srcdir),$(objdir), \
		$(firstword $(subst .cpp,.d,$(wildcard $(srcdir)$(dir)/$(file))))))) \

#-march=$(CPUTYPE) 
#-pedantic 

LTO_PLUGIN = $(shell $(CC) --print-prog-name cyglto_plugin-0.dll)

LDFLAGS	= -flto=8 -nostdlib -static -L. -L$(NDSSDK)/lib
	#--verbose -Wl,--verbose -Wl,-plugin-opt=--verbose
	#-fuse-linker-plugin -Wl,-plugin=$(LTO_PLUGIN)

DEF	:= -DSUB_VERSION=\"CPS\" -DNDS_FW_VERSION=$(NDS_FW_VERSION) -DFILENAME=$(NAME) \
	-DNDS -DUSE_SPEEDHACKS -DNEOGEO_HACKS \
	-DPBPNAME='"$(TARGET)"' -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR)
	#-DFASTCALL -D__fastcall="__attribute__((fastcall))"

#ifdef DEBUG
#	DEF	:= $(DEF) -DFBA_DEBUG
#endif

#ifdef ROM_VERIFY
#	DEF	:= $(DEF) -DROM_VERIFY
#endif

#
#
#	Specify paths
#
#


#
#
#	Specify compiler/linker/assembler
#
#

CC		= mipsel-linux-gcc
CXX		= mipsel-linux-g++
LD		= mipsel-linux-gcc
AS		= mipsel-linux-as
OBJCOPY = mipsel-linux-objcopy
NM		= mipsel-linux-nm
OBJDUMP = mipsel-linux-objdump
FIXUP	= makeplug

CFLAGS	 = $(incdir) $(DEF) \
		-mtune=mips32 -mips32 -O3 -flto -ffat-lto-objects -mno-abicalls -fno-pic -fno-builtin -mno-shared \
	   -mlong-calls -fomit-frame-pointer -msoft-float -G 0 -fno-common\
	   -ffast-math -fsingle-precision-constant -funsafe-math-optimizations \
	   -funroll-loops -funswitch-loops -fprefetch-loop-arrays -Ofast\
	   -Wstrict-aliasing -Wno-write-strings


NATIVE_CC = gcc
NATIVE_CXXFLAGS = -O3


CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti

ASFLAGS	= -O3 -G0 -march=mips32 -mtune=mips32 -mno-shared -msoft-float -fno-lto -fomit-frame-pointer -mlong-calls -mno-abicalls

vpath %.s	$(foreach dir,$(alldir),$(srcdir)$(dir)/ )
vpath %.S	$(foreach dir,$(alldir),$(srcdir)$(dir)/ )
vpath %.cpp	$(foreach dir,$(alldir),$(srcdir)$(dir)/ )
vpath %.c	$(foreach dir,$(alldir),$(srcdir)$(dir)/ )
vpath %.h	$(foreach dir,$(alldir),$(srcdir)$(dir)/ )

vpath %.o 	$(foreach dir,$(alldir),$(objdir)$(dir)/ )
vpath %.d 	$(foreach dir,$(alldir),$(objdir)$(dir)/ )

#
#
#	Rules
#
#

.PHONY:	all init cleandep touch clean swapBuffer.d

ifeq ($(MAKELEVEL),1)
ifdef DEPEND

all:	init $(autdep) $(autobj)
	@$(MAKE) -f makefile.cps -s

else

all:	init $(autobj)
	@$(MAKE) -f makefile.cps -s
endif
else

all:	$(NAME)

endif

#
#
#	Rule for linking the executable
#
#

ifeq ($(MAKELEVEL),2)

LINKS := $(NDSSDK)/specs/link.xn
STARTS := $(NDSSDK)/specs/start.S
STARTO := start.o

$(STARTO): $(STARTS) $(LINKS)
	$(CC) $(CFLAGS) -o $@ -c $(STARTS)

$(ELF):	$(STARTO) $(allobj)
	@echo Linking executable... $(ELF)
	$(CC) $(LDFLAGS) -T $(LINKS) -o $(ELF) $^ $(lib) 
	
$(BIN) : $(ELF)
	$(OBJCOPY) -O binary $< $@
	
$(NAME) : $(BIN) $(ELF)
	@echo Creating nds FBA4DSTWO.plug...
	#$(OBJDUMP) -s $(ELF) > $(BIN:.bin=.dump)
	#$(NM) $(ELF) | sort > $(BIN:.bin=.sym)
	#$(OBJDUMP) -h $(ELF) > $(BIN:.bin=.map)
	@$(FIXUP) $(BIN) $@
	@cp ${BINDIR}/gamelist.txt FBA4DSTWO

ifdef	DEBUG

#	Don't compress when making a debug build

else
ifdef	COMPRESS
	@upx --best $@
endif
endif
endif

ifeq ($(MAKELEVEL),1)
ifdef FORCE_UPDATE
$(build_details.h): FORCE
endif
endif

#
#	Generate the gamelist
#

burn.o burn.d:	driverlist.h

$(driverlist.h): $(drvobj) $(srcdir)scripts/gamelist.pl
ifdef	PERL
	@$(srcdir)scripts/gamelist.pl -o $@ -l ${BINDIR}/gamelist.txt \
		$(filter %.cpp,$(foreach file,$(drvobj:.o=.cpp),$(foreach dir,$(alldir), \
		$(firstword $(wildcard $(srcdir)$(dir)/$(file))))))
else
ifeq ($(MAKELEVEL),2)
	@echo
	@echo Warning: Perl is not available on this system.
	@echo $@ cannot be updated or created!
	@echo
endif
endif

#
#	Fix the .rc file
#

resource.o resource.d:	$(app_gnuc.rc) string.rc version.rc version.h $(build_details.h)

$(license.rtf): $(srcdir)license.txt $(srcdir)scripts/license2rtf.pl

ifdef	PERL
	@$(srcdir)scripts/license2rtf.pl $< -o $(srcdir)generated/$(@F:.rc=.rtf)
else
ifeq ($(MAKELEVEL),2)
	@echo
	@echo Warning: Perl is not available on this system.
	@echo $@ cannot be updated or created!
	@echo
endif
endif

$(app_gnuc.rc): app.rc $(license.rc) $(license.rtf) $(srcdir)scripts/fixrc.pl $(srcdir)burner/win32/resource/fba.ico $(srcdir)burner/win32/resource/about.bmp $(srcdir)burner/win32/resource/preview.bmp $(srcdir)burner/win32/resource/misc.bmp

ifdef	PERL
	@$(srcdir)scripts/fixrc.pl $< -o $@
else
ifeq ($(MAKELEVEL),2)
	@echo
	@echo Warning: Perl is not available on this system.
	@echo $@ cannot be updated or created!
	@echo
endif
endif

#
#	Generate some info on the build details
#

about.o about.d: $(build_details.h)

$(build_details.h):	$(srcdir)scripts/build_details.cpp
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $< -o $(objdir)generated/build_details
	@$(objdir)generated/build_details.exe >$@


#burn_sound_mips.o: burn_sound_mips.s
#	@$(AS) $(ASFLAGS) $(@:.o=.s) -o $@

$(objdir)burn/burn_sound_mips.o: $(srcdir)burn/burn_sound_mips.s
	@$(AS) $(ASFLAGS) $(srcdir)burn/burn_sound_mips.s -o $(objdir)burn/burn_sound_mips.o

#	@$(AS) $(ASFLAGS) -c $< -o $@




#
#	Compile 68000 cores
#

# A68K

$(a68k.o):	mips32r1/fba_make68k.c
	@echo Compiling A68K MC68000 core...
	@gcc -s $< -o $(subst $(srcdir),$(objdir),$(<D))/$(<F:.c=.exe)
	@$(subst $(srcdir),$(objdir),$(<D))/$(<F:.c=.exe) $(@:.o=.s) $(@D)/a68ktbl.inc
	@$(CC) $(CFLAGS) -c -o $@ $(@:.o=.s)


# Musashi

$(objdir)cpu/m68k/m68kcpu.o: $(srcdir)cpu/m68k/m68kcpu.c $(objdir)generated/m68kops.h $(srcdir)cpu/m68k/m68k.h $(srcdir)cpu/m68k/m68kconf.h
	@echo Compiling Musashi MC680x0 core \(m68kcpu.c\)...
	@$(CC) $(CFLAGS) -c $(srcdir)cpu/m68k/m68kcpu.c -o $(objdir)cpu/m68k/m68kcpu.o

$(objdir)cpu/m68k/m68kops.o: $(objdir)cpu/m68k/m68kmake.exe $(objdir)generated/m68kops.h $(objdir)generated/m68kops.c $(srcdir)cpu/m68k/m68k.h $(srcdir)cpu/m68k/m68kconf.h
	@echo Compiling Musashi MC680x0 core \(m68kops.c\)...
	@$(CC) $(CFLAGS) -c $(objdir)generated/m68kops.c -o $(objdir)cpu/m68k/m68kops.o

$(objdir)cpu/m68k/m68kopac.o: $(objdir)cpu/m68k/m68kmake.exe $(objdir)generated/m68kops.h $(objdir)generated/m68kopac.c $(srcdir)cpu/m68k/m68k.h $(srcdir)cpu/m68k/m68kconf.h
	@echo Compiling Musashi MC680x0 core \(m68kopac.c\)...
	@$(CC) $(CFLAGS) -c $(objdir)generated/m68kopac.c -o $(objdir)cpu/m68k/m68kopac.o

$(objdir)cpu/m68k/m68kopdm.o: $(objdir)cpu/m68k/m68kmake.exe $(objdir)generated/m68kops.h $(objdir)generated/m68kopdm.c $(srcdir)cpu/m68k/m68k.h $(srcdir)cpu/m68k/m68kconf.h
	@echo Compiling Musashi MC680x0 core \(m68kopdm.c\)...
	@$(CC) $(CFLAGS) -c $(objdir)generated/m68kopdm.c -o $(objdir)cpu/m68k/m68kopdm.o

$(objdir)cpu/m68k/m68kopnz.o: $(objdir)cpu/m68k/m68kmake.exe $(objdir)generated/m68kops.h $(objdir)generated/m68kopnz.c $(srcdir)cpu/m68k/m68k.h $(srcdir)cpu/m68k/m68kconf.h
	@echo Compiling Musashi MC680x0 core \(m68kopnz.c\)...
	@$(CC) $(CFLAGS) -c $(objdir)generated/m68kopnz.c -o $(objdir)cpu/m68k/m68kopnz.o

$(objdir)generated/m68kops.h: $(objdir)cpu/m68k/m68kmake.exe $(srcdir)cpu/m68k/m68k_in.c
	$(objdir)/cpu/m68k/m68kmake $(objdir)generated/ $(srcdir)cpu/m68k/m68k_in.c

$(objdir)cpu/m68k/m68kmake.exe: $(srcdir)cpu/m68k/m68kmake.c
	@echo Compiling Musashi MC680x0 core \(m68kmake.c\)...
	@gcc $(srcdir)cpu/m68k/m68kmake.c -o $(objdir)cpu/m68k/m68kmake.exe

	
#
#	Extra rules for generated header file cvt.h, needed by ctv.cpp
#

ctv.d ctv.o:	$(ctv.h)

$(ctv.h):	ctv_make.cpp
	@echo Generating $(srcdir)generated/$(@F)...
	@$(NATIVE_CC) ${NATIVE_CXXFLAGS} $< \
		-o $(subst $(srcdir),$(objdir),$(<D))/$(<F:.cpp=.exe)
	@$(subst $(srcdir),$(objdir),$(<D))/$(<F:.cpp=.exe) >$@

#
#	Extra rules for generated header file toa_gp9001_func.h, needed by toa_gp9001.cpp
#

toa_gp9001.d toa_gp9001.o: $(toa_gp9001_func.h)

$(toa_gp9001_func.h):	$(srcdir)scripts/toa_gp9001_func.pl
	@$(srcdir)scripts/toa_gp9001_func.pl -o $(toa_gp9001_func.h)

#
#	Extra rules for generated header file neo_sprite_func.h, needed by neo_sprite.cpp
#

neo_sprite.d neo_sprite.o: $(neo_sprite_func.h)

$(neo_sprite_func.h):	$(srcdir)scripts/neo_sprite_func.pl
	@$(srcdir)scripts/neo_sprite_func.pl -o $(neo_sprite_func.h)

#
#	Extra rules for generated header file cave_tile_func.h, needed by cave_tile.cpp
#

cave_tile.d cave_tile.o: $(cave_tile_func.h)

$(cave_tile_func.h):	$(srcdir)scripts/cave_tile_func.pl
	@$(srcdir)scripts/cave_tile_func.pl -o $(cave_tile_func.h)

#
#	Extra rules for generated header file cave_sprite_func.h, needed by cave_sprite.cpp
#

cave_sprite.d cave_sprite.o: $(cave_sprite_func.h)

$(cave_sprite_func.h):	$(srcdir)scripts/cave_sprite_func.pl
	@$(srcdir)scripts/cave_sprite_func.pl -o $(cave_sprite_func.h)

#
#	Extra rules for generated header file psikyo_tile_func.h / psikyo_sprite_func.h, needed by psikyo_tile.cpp / psikyo_sprite.cpp
#

psikyo_tile.d psikyo_tile.o psikyosprite.d psikyo_sprite.o: $(psikyo_tile_func.h)

$(psikyo_tile_func.h):	$(srcdir)scripts/psikyo_tile_func.pl
	$(srcdir)scripts/psikyo_tile_func.pl -o $(psikyo_tile_func.h)

ifeq ($(MAKELEVEL),2)
ifdef DEPEND

include	$(alldep)

endif
endif

#
#	Generic rules for C/C++ files
#

ifeq ($(MAKELEVEL),1)

%.o:	%.cpp
	@echo Compiling $<...
	@$(CC) $(CXXFLAGS) -c $< -o $(subst $(srcdir),$(objdir),$(<D))/$(@F)

%.o:	%.c
	@echo Compiling $<...
	@$(CC) $(CFLAGS) -c $< -o $(subst $(srcdir),$(objdir),$(<D))/$(@F)

%.o:	%.s
	@echo Assembling $<...
	@$(AS) $(ASFLAGS) $< -o $(subst $(srcdir),$(objdir),$(<D))/$(@F)
	
%.o:	%.S
	@echo Assembling $<...
	@$(CC) $(CFLAGS) -c $< -o $(subst $(srcdir),$(objdir),$(<D))/$(@F)
	
else

%.o:	%.c
	@echo Compiling $<...
	@$(CC) $(CFLAGS) -c $< -o $@

%.o:	%.s
	@echo Assembling $<...
	@$(AS) $(ASFLAGS) $< -o $@
	 
%.o:	%.S
	@echo Assembling $<...
	@$(CC) $(CFLAGS) -c $< -o $@

%.o:
	@echo Compiling $<...
	@$(CC) $(CXXFLAGS) -c $< -o $@

endif

#
#	Generate dependencies for C/C++ files
#

ifdef DEPEND

%.d:	%.c
	@echo Generating depend file for $<...
	@$(CC) -MM -MT "$(subst $(srcdir),$(objdir),$(<D))/$(*F).o $(subst $(srcdir),$(objdir),$(<D))/$(@F)" -x c $(CFLAGS) $< >$(subst $(srcdir),$(objdir),$(<D))/$(@F)

%.d:	%.cpp
	@echo Generating depend file for $<...
	@$(CC) -MM -MT "$(subst $(srcdir),$(objdir),$(<D))/$(*F).o $(subst $(srcdir),$(objdir),$(<D))/$(@F)" -x c++ $(CXXFLAGS) $< >$(subst $(srcdir),$(objdir),$(<D))/$(@F)

endif

#
#	Phony targets
#

init:
	@echo Making normal build...
	@echo
	@mkdir -p $(foreach dir, $(alldir),$(objdir)$(dir))
	@mkdir -p $(srcdir)generated
	@mkdir -p ${BINDIR}

cleandep:
	@echo Removing depend files from $(objdir)...
	-@for dir in $(alldir); do rm -f $(objdir)$$dir/*.d; done

touch:
	@echo Marking all targets for $(NAME) as uptodate...
	-@touch $(NAME).exe
	-@touch -c -r $(NAME).exe $(srcdir)/generated/*
	-@for dir in $(alldir); do touch -c  -r $(NAME).exe $(objdir)$$dir/*; done

clean:
	@echo Removing all files from $(objdir)...
	-@rm -f -r $(objdir)
	-@rm -f -r $(ctv.h)

ifdef	PERL
	@echo Removing all files generated with perl scripts...
	-@rm -f -r $(app_gnuc.rc) $(driverlist)
endif


#
#	Rule to force recompilation of any target that depends on it
#

FORCE:
