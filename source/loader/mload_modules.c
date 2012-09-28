#include "mload_modules.h"
#include "ehcmodule_5.h"
#include "sdhc_module.h"
#include "odip_frag.h"

static u32 ios_36[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022DDAC, // dvd_read_controlling_data
	0x20201010+1, // handle_di_cmd_reentry (thumb)
	0x20200b9c+1, // ios_shared_alloc_aligned (thumb)
	0x20200b70+1, // ios_shared_free (thumb)
	0x20205dc0+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202b4c+1, // ios_doReadHashEncryptedState (thumb)
	0x20203934+1, // ios_printf (thumb)
};

static u32 ios_38[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022cdac, // dvd_read_controlling_data
	0x20200d38+1, // handle_di_cmd_reentry (thumb)
	0x202008c4+1, // ios_shared_alloc_aligned (thumb)
	0x20200898+1, // ios_shared_free (thumb)
	0x20205b80+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202874+1, // ios_doReadHashEncryptedState (thumb)
	0x2020365c+1, // ios_printf (thumb)
};

static u32 ios_37[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022DD60, // dvd_read_controlling_data
	0x20200F04+1, // handle_di_cmd_reentry (thumb)
	0x2020096C+1, // ios_shared_alloc_aligned (thumb)
	0x2020093C+1, // ios_shared_free (thumb)
	0x20205E54+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202A70+1, // ios_doReadHashEncryptedState (thumb)
	0x2020387C+1, // ios_printf (thumb)
};

static u32 ios_57[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022cd60, // dvd_read_controlling_data
	0x20200f04+1, // handle_di_cmd_reentry (thumb)
	0x2020096c+1, // ios_shared_alloc_aligned (thumb) // no usado
	0x2020093C+1, // ios_shared_free (thumb) // no usado
	0x20205EF0+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202944+1, // ios_doReadHashEncryptedState (thumb)
	0x20203750+1, // ios_printf (thumb)
};

static u32 ios_60[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022cd60, // dvd_read_controlling_data
	0x20200f04+1, // handle_di_cmd_reentry (thumb)
	0x2020096c+1, // ios_shared_alloc_aligned (thumb) // no usado
	0x2020093C+1, // ios_shared_free (thumb) // no usado
	0x20205e00+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202944+1, // ios_doReadHashEncryptedState (thumb)
	0x20203750+1, // ios_printf (thumb)
};

u32 patch_datas[8] ATTRIBUTE_ALIGN(32);

data_elf my_data_elf;
static int my_thread_id = 0;

int load_ehc_module()
{
	if(mload_init() < 0)
		return -1;
	mload_elf((void *)ehcmodule_5, &my_data_elf);
	my_thread_id = mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
	if(my_thread_id < 0)
		return -2;
	usleep(350*1000);

	int is_ios = mload_get_IOS_base();
	//u32 dip_address = 0x1377E000;
	u32 dip_address = 0x1377C000;
	switch(is_ios)
	{
		case 36:
			memcpy(ios_36, odip_frag, 4);		// copy the entry_point
			memcpy(odip_frag, ios_36, 4*10);	// copy the adresses from the array

			mload_seek(dip_address, SEEK_SET);	// copy odip_frag in the starlet
			mload_write(odip_frag,size_odip_frag);

			// enables DIP plugin
			mload_seek(0x20209040, SEEK_SET);
			mload_write(ios_36, 4);
			break;
		case 37:
			memcpy(ios_37, odip_frag, 4);	    // copy the entry_point
			memcpy(odip_frag, ios_37, 4*10);   // copy the adresses from the array

			mload_seek(dip_address, SEEK_SET);	// copy odip_frag in the starlet
			mload_write(odip_frag,size_odip_frag);

			// enables DIP plugin
			mload_seek(0x20209030, SEEK_SET);
			mload_write(ios_37, 4);
			break;
		case 38:
			memcpy(ios_38, odip_frag, 4);	    // copy the entry_point
			memcpy(odip_frag, ios_38, 4*10);   // copy the adresses from the array

			mload_seek(dip_address, SEEK_SET);	// copy odip_frag in the starlet
			mload_write(odip_frag,size_odip_frag);

			// enables DIP plugin
			mload_seek(0x20208030, SEEK_SET);
			mload_write(ios_38, 4);
			break;
		case 57:
			memcpy(ios_57, odip_frag, 4);	    // copy the entry_point
			memcpy(odip_frag, ios_57, 4*10);   // copy the adresses from the array

			mload_seek(dip_address, SEEK_SET);	// copy odip_frag in the starlet
			mload_write(odip_frag,size_odip_frag);

			// enables DIP plugin
			mload_seek(0x20208030, SEEK_SET);
			mload_write(ios_57, 4);
			break;
		case 60:
			memcpy(ios_60, odip_frag, 4);	    // copy the entry_point
			memcpy(odip_frag, ios_60, 4*10);   // copy the adresses from the array

			mload_seek(dip_address, SEEK_SET);	// copy odip_frag in the starlet
			mload_write(odip_frag,size_odip_frag);

			// enables DIP plugin
			mload_seek(0x20208030, SEEK_SET);
			mload_write(ios_60, 4);
			break;
	}
	mload_elf((void *)sdhc_module, &my_data_elf);
	my_thread_id = mload_run_thread(my_data_elf.start, my_data_elf.stack,
			my_data_elf.size_stack, my_data_elf.prio);
	if(my_thread_id < 0)
		return -2;
	mload_close();
	return 0;
}

#if 0

#define IOCTL_FAT_MOUNTSD	0xF0
#define IOCTL_FAT_UMOUNTSD	0xF1
#define IOCTL_FAT_MOUNTUSB	0xF2
#define IOCTL_FAT_UMOUNTUSB	0xF3

#define IOCTL_FFS_MODE		0x80

void disable_ffs_patch(void)
{
	
u8 * ffs_data=search_for_ehcmodule_cfg((void *) fatffs_module, size_fatffs_module);
	
	if(ffs_data)
		{
		ffs_data+=12;
		ffs_data[0]=1;
		DCFlushRange((void *) (((u32)ffs_data[0]) & ~31), 32);
		}
	
}

int load_fatffs_module(u8 *discid)
{
static char fs[] ATTRIBUTE_ALIGN(32) = "fat";
s32 hid = -1, fd = -1;
static char file_name[256]  ALIGNED(0x20)="sd:";
int n;
char *p;
s32 ret;

	p=&file_name[0];

	if(mload_init()<0) return -1;

	mload_elf((void *) fatffs_module, &my_data_elf);
	my_thread_id= mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
	if(my_thread_id<0) return -1;

    global_mount &=~0xc;

	if(discid)
		{
		sd_ok=ud_ok=1;
				
		p=get_fat_name(discid);
				
		sd_ok=ud_ok=0;
				
		if(!p) return -1;

		global_mount &=~0xc;

			
		// change 'ud:' by 'usb:'
		if(p[0]=='u') {global_mount|=2;file_name[0]='u';file_name[1]='s';file_name[2]='b';memcpy(file_name+3, (void *)p+2, 253);}			   
		else {global_mount|=1;memcpy(file_name, (void *) p, 256);}

		// copy filename to odip_frag filename area
		mload_seek(*((u32 *) (odip_frag+14*4)), SEEK_SET);	// offset 14 (filename Address - 256 bytes)
		mload_write(file_name, sizeof(file_name));
		mload_close();

		
		}
	else
		{
		if((global_mount & 3)==0) return 0;
		if(global_mount & 1) p[0]='s';
		if(global_mount & 2) p[0]='u';
		}
	usleep(350*1000);

	/* Create heap */
	if (hid < 0) {
		hid = iosCreateHeap(0x100);
		if (hid < 0)
			return -1; 
	}

	/* Open USB device */
	fd = IOS_Open(fs, 0);
	
	if (fd < 0)
		{
		if(hid>=0)
			{
			iosDestroyHeap(hid);
			hid=-1;
			}
		return -1;
		}
  
	n=30; // try 20 times
	while(n>0)
	{
		if((global_mount & 10)==2) {ret=IOS_IoctlvFormat(hid, fd, IOCTL_FAT_MOUNTUSB, ":");if(ret==0) global_mount|=8;}
		else {ret=IOS_IoctlvFormat(hid, fd, IOCTL_FAT_MOUNTSD, ":");if(ret==0) {global_mount|=4;}}
		
		if((global_mount & 7)==3 && ret==0)	 
			{ret=IOS_IoctlvFormat(hid, fd, IOCTL_FAT_MOUNTSD, ":");if(ret==0) global_mount|=4;}

    if((global_mount & 3)==((global_mount>>2) & 3) && (global_mount & 3)) {ret=0;break;} else ret=-1;
	
	//ret=IOS_IoctlvFormat(hid, fd, IOCTL_FAT_MOUNTSD, ":");
	//if(ret==0) break;
	usleep(500*1000);
	n--;
	}

    if (fd >= 0) {
		IOS_Close(fd);
		fd = -1;
	}

	if(hid>=0)
		{
		iosDestroyHeap(hid);
		hid=-1;
		}
	
	if(n==0) return -1;

return 0;
}


int enable_ffs(int mode)
{
static char fs[] ATTRIBUTE_ALIGN(32) = "fat";
 s32 hid = -1, fd = -1;
s32 ret;

	/* Create heap */
	if (hid < 0) {
		hid = iosCreateHeap(0x100);
		if (hid < 0)
			return -1; 
	}

	/* Open USB device */
	fd = IOS_Open(fs, 0);
	
	if (fd < 0)
		{
		if(hid>=0)
			{
			iosDestroyHeap(hid);
			hid=-1;
			}
		return -1;
		}


	ret=IOS_IoctlvFormat(hid, fd, IOCTL_FFS_MODE, "i:", mode);
	

    if (fd >= 0) {
		IOS_Close(fd);
		fd = -1;
	}

	if(hid>=0)
		{
		iosDestroyHeap(hid);
		hid=-1;
		}
	


return ret;
}

void enable_ES_ioctlv_vector(void)
{
	patch_datas[0]=*((u32 *) (odip_frag+16*4));
	mload_set_ES_ioctlv_vector((void *) patch_datas[0]);
}

void Set_DIP_BCA_Datas(u8 *bca_data)
{
	// write in odip_frag bca data area
	mload_seek(*((u32 *) (odip_frag+15*4)), SEEK_SET);	// offset 15 (bca_data area)
	mload_write(bca_data, 64);
	mload_close();
}

void test_and_patch_for_port1()
{
	// test for port 1
	u8 *ehc_data = search_for_ehcmodule_cfg((void *)ehcmodule_5, size_ehcmodule_5);
	if(ehc_data)
	{
		ehc_data+=12;
		use_port1=ehc_data[0];//=0; // fixed to 0
		DCFlushRange((void *) (((u32)ehc_data[0]) & ~31), 32);
	}

	if(use_port1) // release port 0 and use port 1
	{
		u32 dat=0;
		u32 addr;

		// get EHCI base registers
		mload_getw((void *) 0x0D040000, &addr);

		addr&=0xff;
		addr+=0x0D040000;

		mload_getw((void *) (addr+0x44), &dat);
		if((dat & 0x2001)==1) mload_setw((void *) (addr+0x44), 0x2000);
		mload_getw((void *) (addr+0x48), &dat);
		if((dat & 0x2000)==0x2000) mload_setw((void *) (addr+0x48), 0x1001);
	}
}

void free_usb_ports()
{
	u32 dat=0;
	u32 addr;

	// get EHCI base registers
	mload_getw((void *) 0x0D040000, &addr);

	addr&=0xff;
	addr+=0x0D040000;

	mload_getw((void *) (addr+0x44), &dat);
	if((dat & 0x2001)==1) mload_setw((void *) (addr+0x44), 0x2000);
	mload_getw((void *) (addr+0x48), &dat);
	if((dat & 0x2001)==1) mload_setw((void *) (addr+0x48), 0x2000);
	
}

#endif

u8 *search_for_ehcmodule_cfg(u8 *p, int size)
{
	int n;
	for(n = 0; n < size; n++)
	{
		if(!memcmp((void *) &p[n],"EHC_CFG",8) && p[n+8]==0x12 && p[n+9]==0x34 && p[n+10]==0x00 && p[n+11]==0x01)
			return &p[n];
	}
	return NULL;
}
