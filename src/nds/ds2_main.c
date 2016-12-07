//ds2_main.c

#include <stdio.h>
#include "console.h"
#include "fs_api.h"
#include "ds2io.h"
#include "nds.h"

extern int main(int argc, char* argv[]);

void ds2_main(void)
{
	int err;

	//Initial video and audio and other input and output
	err = ds2io_initb(SND_FRAME_SIZE,22050,0,0);
	if(err) goto _failure;

	//Initial file system
	err = fat_init();
	if(err) goto _failure;

	//go to user main funtion
	main(0, 0);

_failure:
	ds2_setCPUclocklevel(0);
	ds2_plug_exit();
}

