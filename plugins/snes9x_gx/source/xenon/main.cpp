#include <stdio.h>
#include <malloc.h>

#include "s9xconfig.h"

#include "snes9x.h"
#include "memmap.h"
#include "s9xdebug.h"
#include "cpuexec.h"
#include "ppu.h"
#include "apu.h"
#include "display.h"
#include "gfx.h"
#include "soundux.h"
#include "spc700.h"
#include "spc7110.h"
#include "controls.h"

#include <console/console.h>
#include <diskio/diskio.h>
#include <xenos/xenos.h>
#include <xenon_sound/sound.h>
#include <xenon_soc/xenon_power.h>

//#include "smc.h"

#include "video.h"

extern unsigned long SNESROMSize;

extern void S9xInitSync();

void ExitApp(void)
{
	printf(" *** ExitApp\n");
	while (1);
}

int ResetRequested = 1, ConfigRequested;

extern "C" {
	void usb_do_poll(void);
	void kmem_init(void);
	void usb_init(void);

#include <input/input.h>
};

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

static inline uint32_t bswap_32(uint32_t t)
{
	return ((t & 0xFF) << 24) | ((t & 0xFF00) << 8) | ((t & 0xFF0000) >> 8) | ((t & 0xFF000000) >> 24);
}

void
emulate ()
{
	while(1) // emulation loop
	{
		int ctrl;
		for (ctrl = 0; ctrl < 4; ++ctrl)
		{
			struct controller_data_s c;
			if (get_controller_data(&c, ctrl))
			{
	//			printf("got controller data!\n");
				int offset = 0x10 + ctrl * 0x10;
	//			printf("a=%d, b=%d, x=%d, y=%d, lb=%d, rb=%d, start=%d, select=%d, up=%d, down=%d, left=%d, right=%d\n", 
	//				c.a, c.b, c.x, c.y, c.lb, c.rb, c.start, c.select, c.up, c.down, c.left, c.right);

				S9xReportButton (offset + 0, c.b);
				S9xReportButton (offset + 1, c.a);
				S9xReportButton (offset + 2, c.x);
				S9xReportButton (offset + 3, c.y);

				S9xReportButton (offset + 4, c.lb);
				S9xReportButton (offset + 5, c.rb);

				S9xReportButton (offset + 6, c.start);
				S9xReportButton (offset + 7, c.select);

				S9xReportButton (offset + 8, c.up);
				S9xReportButton (offset + 9, c.down);
				S9xReportButton (offset + 10, c.left);
				S9xReportButton (offset + 11, c.right);
			}
		}
		S9xMainLoop ();

		usb_do_poll();

		if(ResetRequested)
		{
			S9xSoftReset (); // reset game
			ResetRequested = 0;
		}
		if (ConfigRequested)
		{
			ConfigRequested = 0;
			break;
		}


					/* this all isn't that great... */
		int sample_rate = 48000 * 2;
		int samples_per_frame = Settings.PAL ? sample_rate / 50 : sample_rate / 60;
		
		int samples_guard = 16384;
		
		if (xenon_sound_get_unplayed() < samples_guard)
		{
			so.samples_mixed_so_far = so.play_position = 0;
			
			unsigned char buffer[2048];

#if 0	
			int req_samples = xenon_sound_get_unplayed();
			if ((req_samples + samples_per_frame) < samples_guard)
				req_samples = samples_guard + samples_per_frame;
			else
				req_samples = samples_per_frame;
#endif
			int req_samples = samples_per_frame;

			S9xMixSamples(buffer, req_samples);

			int i;
			for (i = 0; i < req_samples * 2; i += 4)
				*(int*)(buffer + i) = bswap_32(*(int*)(buffer + i));
			xenon_sound_submit(buffer, req_samples * 2);

		}
           

	} // main loop
}
 
#define ASSIGN_BUTTON_TRUE( keycode, snescmd ) \
		S9xMapButton( keycode, cmd = S9xGetCommandT(snescmd), true) 
 
#define ASSIGN_BUTTON_FALSE( keycode, snescmd ) \
		S9xMapButton( keycode, cmd = S9xGetCommandT(snescmd), false) 

void SetDefaultButtonMap ()
{
	int maxcode = 0x10;
	s9xcommand_t cmd;

	/*** Joypad 1 ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 A");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 B");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 X");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Y");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 R");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Start");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Select");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Up");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Down");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Left");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad1 Right");

	maxcode = 0x20;
	/*** Joypad 2 ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 A");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 B");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 X");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Y");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 R");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Start");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Select");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Up");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Down");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Left");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad2 Right");

	maxcode = 0x30;
	/*** Joypad 3 ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 A");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 B");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 X");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Y");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 R");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Start");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Select");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Up");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Down");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Left");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad3 Right");

	maxcode = 0x40;
	/*** Joypad 4 ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 A");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 B");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 X");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Y");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 R");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Start");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Select");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Up");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Down");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Left");
	ASSIGN_BUTTON_FALSE (maxcode++, "Joypad4 Right");

	maxcode = 0x50;
	/*** Superscope ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope Fire");
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope AimOffscreen");
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope Cursor");
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope ToggleTurbo");
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope ToggleTurbo");
	ASSIGN_BUTTON_FALSE (maxcode++, "Superscope Pause");

	maxcode = 0x60;
	/*** Mouse ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Mouse1 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Mouse1 R");
	ASSIGN_BUTTON_FALSE (maxcode++, "Mouse2 L");
	ASSIGN_BUTTON_FALSE (maxcode++, "Mouse2 R");

	maxcode = 0x70;
	/*** Justifier ***/
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier1 Trigger");
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier1 AimOffscreen");
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier1 Start");
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier2 Trigger");
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier2 AimOffscreen");
	ASSIGN_BUTTON_FALSE (maxcode++, "Justifier2 Start");

	maxcode = 0x80;
	S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Superscope"), false);
	S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Mouse1"), false);
	S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Mouse2"), false);
	S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Justifier1"), false);
	S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Justifier2"), false);

	maxcode = 0x90;
	ASSIGN_BUTTON_FALSE (maxcode++, "Screenshot");

	// Plugin 2 Joypads by default
	S9xSetController (0, CTL_JOYPAD, 0, 0, 0, 0);
	S9xSetController (1, CTL_JOYPAD, 1, 0, 0, 0);
	S9xVerifyControllers();
}


int main(void)
{	
	extern void xenos_init();
	xenos_init();
	console_init();

	xenon_thread_startup();
	xenon_make_it_faster(XENON_SPEED_FULL);
	xenon_sleep_thread(1);
	xenon_sleep_thread(2);
	xenon_sleep_thread(3);
	xenon_sleep_thread(4);
	xenon_sleep_thread(5);
	
	printf("SNES9x GX\n");

	kmem_init();
	usb_init();
	// Set defaults
	DefaultSettings ();

	S9xUnmapAllControls ();
	SetDefaultButtonMap ();	 

	// Allocate SNES Memory
 if (!Memory.Init ())
		ExitApp();

	// Allocate APU
	if (!S9xInitAPU ())
		ExitApp();

	// Set Pixel Renderer to match 565
	S9xSetRenderPixelFormat (RGB565);

	// Initialise Snes Sound System
	S9xInitSound (5, TRUE, 1024);

	// Initialise Graphics
		
//	setGFX ();
	videoInit();
	
	if (!S9xGraphicsInit ())
		ExitApp();

	S9xSetSoundMute (TRUE);
	S9xInitSync(); // initialize frame sync

	// Plugin 2 Joypads by default
	S9xSetController (0, CTL_JOYPAD, 0, 0, 0, 0);
	S9xSetController (1, CTL_JOYPAD, 1, 0, 0, 0);
	S9xVerifyControllers();

	printf("Waiting for USB storage...\n");
	
	extern void xenos_init();
	int fd;

	do {
		usb_do_poll();
		
		fd = open("uda:/SNES9X.SMC", O_RDONLY);
	} while (fd < 0);

	struct stat stat;
	fstat(fd, &stat);

	SNESROMSize = stat.st_size;
	if (read(fd, Memory.ROM, SNESROMSize) != SNESROMSize)
	{
		printf("Failed to read rom\n");
		while (1);
	}
	
//	memcpy(Memory.ROM, smc, SNESROMSize = sizeof(smc));

	Memory.LoadROM ("BLANK.SMC");
	Memory.LoadSRAM ("BLANK");
	
	console_close();

	emulate(); // main loop

	while (1);
	return 0;
}

extern "C" {	

char* getcwd(char*, size_t)
{
	return 0;
}

int chdir(const char *f)
{
	return -1;
}


uid_t getuid(void)
{
	return 0;
}

gid_t getgid(void)
{
	return 0;
}

int chown(const char*, uid_t, gid_t)
{
}

int stat(const char*, struct stat*)
{
	return -1;
}

int unlink(const char*)
{
	return -1;
}

void gettimeofday()
{
}

}
