#	Main Makefile for FB Alpha, execute an appropriate system-specific makefile

export

#
#	Declare variables
#

# Make a special build, pass the quoted text as comment (use FORCE_UPDATE declaration below to force recompilation of resources)
# SPECIALBUILD = "This text will appear in the property sheet of the .exe file"



#
#	Flags. Uncomment any of these declarations to enable their function.
#

# Inluclude Unicode support
#UNICODE = 1

# Include symbols and other debug information in the executable
#SYMBOL = 1

# Include features for debugging drivers
#DEBUG	= 1

# Include rom set verifying features (comment this for release builds)
#ROM_VERIFY = 1

# Force recompilation of files that need it (i.e. use __TIME__, __DATE__, SPECIALBUILD).
#FORCE_UPDATE = 1

# Use the __fastcall calling convention when interfacing with A68K/Musashi/Doze
#FASTCALL = 1

# Compress executable with upx (the DEBUG option ignores this)
# COMPRESS = 1

# Perl is available
PERL = 1

#
#	execute an appropriate system-specific makefile
#
test: FORCE
	@$(MAKE) -f makefile.test
psikyo: FORCE

	@$(MAKE) -s -f makefile.psikyo

all: FORCE

	@$(MAKE) -s -f makefile.all
	
mid: FORCE

	@$(MAKE) -s -f makefile.mid
small: FORCE

	@$(MAKE) -s -f makefile.small
cave: FORCE

	@$(MAKE) -s -f makefile.cave
pgm: FORCE

	@$(MAKE) -s -f makefile.pgm
pgm_new: FORCE

	@$(MAKE) -s -f makefile.pgm_new
sega: FORCE

	@$(MAKE) -s -f makefile.sega
cps3: FORCE

	@$(MAKE) -s -f makefile.cps3

clean: FORCE
	-@rm -f -r obj/PSP/psp
	-@rm -f -r obj/PSP/burn/burn*
	-@rm -f -r src/generated/driverlist.h

alltarget : FORCE
	-@rm -f -r obj/PSP/psp
	-@rm -f -r obj/PSP/burn/burn*
	-@rm -f -r src/generated/driverlist.h
	@$(MAKE) -s -f makefile.pgm
	-@rm -f -r obj/PSP/psp
	-@rm -f -r obj/PSP/burn/burn*
	-@rm -f -r src/generated/driverlist.h
	@$(MAKE) -s -f makefile.pgm_new
	-@rm -f -r obj/PSP/psp
	-@rm -f -r obj/PSP/burn/burn*
	-@rm -f -r src/generated/driverlist.h
	@$(MAKE) -s -f makefile.cave
	-@rm -f -r obj/PSP/psp
	-@rm -f -r obj/PSP/burn/burn*
	-@rm -f -r src/generated/driverlist.h
	@$(MAKE) -s -f makefile.cps3
	-@rm -f -r obj/PSP/psp
	-@rm -f -r obj/PSP/burn/burn*
	-@rm -f -r src/generated/driverlist.h
	@$(MAKE) -s -f makefile.sega
	-@rm -f -r obj/PSP/psp
	-@rm -f -r obj/PSP/burn/burn*
	-@rm -f -r src/generated/driverlist.h
	@$(MAKE) -s -f makefile.small
	-@rm -f -r obj/PSP/psp		
	-@rm -f -r obj/PSP/burn/burn*
	-@rm -f -r src/generated/driverlist.h
	@$(MAKE) -s -f makefile.mid
	-@rm -f -r obj/PSP/psp
	-@rm -f -r obj/PSP/burn/burn*
	-@rm -f -r src/generated/driverlist.h
	@$(MAKE) -s -f makefile.all
FORCE:
#	@$(MAKE) -C src/psp/me/mediaengineprx clean
#	@$(MAKE) -C src/psp/me/mediaengineprx
#	@$(MAKE) -C src/psp/me/me_load
#	@cp src/psp/me/mediaengineprx/mediaengine.prx bin/
#	@cp src/psp/me/me_load/me_load.bin bin/
