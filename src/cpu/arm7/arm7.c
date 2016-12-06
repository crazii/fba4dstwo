/*****************************************************************************
 *
 *   arm7.c
 *   Portable ARM7TDMI CPU Emulator
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     sellenoff@hotmail.com
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *  #3) Thumb support by Ryan Holtz
 *
 *****************************************************************************/

/******************************************************************************
 *  Notes:

    ** This is a plain vanilla implementation of an ARM7 cpu which incorporates my ARM7 core.
       It can be used as is, or used to demonstrate how to utilize the arm7 core to create a cpu
       that uses the core, since there are numerous different mcu packages that incorporate an arm7 core.

       See the notes in the arm7core.c file itself regarding issues/limitations of the arm7 core.
    **
*****************************************************************************/
typedef signed char INT8;
typedef unsigned char UINT8;
typedef signed short INT16;
typedef unsigned short UINT16;
typedef signed int INT32;
typedef unsigned int UINT32;
typedef long long INT64;
typedef unsigned long long UINT64;
typedef unsigned int offs_t;
#define INLINE inline static
#define NULL 0

#include "arm7.h"
#include "state.h"

//#include "debugger.h"


/* Example for showing how Co-Proc functions work */
#define TEST_COPROC_FUNCS 0

/* prototypes */
#if TEST_COPROC_FUNCS
static WRITE32_HANDLER(test_do_callback);
static READ32_HANDLER(test_rt_r_callback);
static WRITE32_HANDLER(test_rt_w_callback);
static void test_dt_r_callback(UINT32 insn, UINT32 *prn, UINT32 (*read32)(UINT32 addr));
static void test_dt_w_callback(UINT32 insn, UINT32 *prn, void (*write32)(UINT32 addr, UINT32 data));
#endif

/* Macros that can be re-defined for custom cpu implementations - The core expects these to be defined */
/* In this case, we are using the default arm7 handlers (supplied by the core)
   - but simply changes these and define your own if needed for cpu implementation specific needs */
#define READ8(addr)         arm7_cpu_read8(addr)
#define WRITE8(addr,data)   arm7_cpu_write8(addr,data)
#define READ16(addr)        arm7_cpu_read16(addr)
#define WRITE16(addr,data)  arm7_cpu_write16(addr,data)
#define READ32(addr)        arm7_cpu_read32(addr)
#define WRITE32(addr,data)  arm7_cpu_write32(addr,data)
#define PTR_READ32          &arm7_cpu_read32
#define PTR_WRITE32         &arm7_cpu_write32

/* Macros that need to be defined according to the cpu implementation specific need */
#define ARM7REG(reg)        arm7.sArmRegister[reg]
#define ARM7                arm7
#define ARM7_ICOUNT         arm7_icount

/* CPU Registers */
typedef struct
{
    ARM7CORE_REGS               // these must be included in your cpu specific register implementation
} ARM7_REGS;

static ARM7_REGS arm7;
int ARM7_ICOUNT;
unsigned int program_read_dword_32le(unsigned int a)
{
	unsigned int p = ARM7.ppMemRead[ a >> ARM7_MEM_SHIFT ];
	if ( p )
		return *(unsigned int*)(p + a);
	else
		return ARM7.ReadHandler(a);
}

unsigned short program_read_word_32le(unsigned int a)
{
	unsigned int p = ARM7.ppMemRead[ a >> ARM7_MEM_SHIFT ];
	if ( p )
		return *(unsigned short *)(p + a);
	else
		return ARM7.ReadHandler(a);
}
unsigned char program_read_byte_32le(unsigned int a)
{
	unsigned int p = ARM7.ppMemRead[ a >> ARM7_MEM_SHIFT ];
	if ( p )
		return *(unsigned char *)(p + a);
	else
		return ARM7.ReadHandler(a);
}
void program_write_dword_32le(unsigned int a, unsigned int d)
{
	
	unsigned int p =ARM7.ppMemWrite[ a >> ARM7_MEM_SHIFT ];

	if ( p )
		*(unsigned int *)(p + a) = d;
	else
		ARM7.WriteHandler(a, d);
}
void program_write_word_32le(unsigned int a, unsigned short d)
{
	unsigned int p = ARM7.ppMemWrite[ a >> ARM7_MEM_SHIFT ];
	if ( p )
		*(unsigned short *)(p + a) = d;
	else
		ARM7.WriteHandler(a, d);
}
void program_write_byte_32le(unsigned int a, unsigned char d)
{
	unsigned int p = ARM7.ppMemWrite[ a >> ARM7_MEM_SHIFT ];
	if ( p )
		*(unsigned char *)(p + a) = d;
	else
		ARM7.WriteHandler(a, d);
}

unsigned int cpu_readop32(unsigned int a)
{
	unsigned int p= ARM7.ppMemRead[ a >> ARM7_MEM_SHIFT ];
	if ( p )
	{
		return *(unsigned int *)(p + a);
	}else
	{
		return 0;
	}
}

unsigned short cpu_readop16(unsigned int a)
{
	unsigned int p= ARM7.ppMemRead[ a >> ARM7_MEM_SHIFT ];
	if ( p )
	{
		return *(unsigned short *)(p + a);
	}
	else
	{
		return 0;
	}
}
/* include the arm7 core */
#include "arm7core.c"

/***************************************************************************
 * CPU SPECIFIC IMPLEMENTATIONS
 **************************************************************************/
static void arm7_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
    // must call core
    //arm7_core_init("arm7", index);

    ARM7.irq_callback = irqcallback;

#if TEST_COPROC_FUNCS
    // setup co-proc callbacks example
    arm7_coproc_do_callback = test_do_callback;
    arm7_coproc_rt_r_callback = test_rt_r_callback;
    arm7_coproc_rt_w_callback = test_rt_w_callback;
    arm7_coproc_dt_r_callback = test_dt_r_callback;
    arm7_coproc_dt_w_callback = test_dt_w_callback;
#endif
}

void arm7_reset(void)
{
    // must call core reset
    arm7_core_reset();
}

static void arm7_exit(void)
{
    /* nothing to do here */
}

int arm7_execute(int cycles)
{
/* include the arm7 core execute code */
#include "arm7exec.c"
}


void arm7_set_irq_line(int irqline, int state)
{
    // must call core
    arm7_core_set_irq_line(irqline,state);
}

static void arm7_get_context(void *dst)
{
    if (dst)
    {
        memcpy(dst, &ARM7, sizeof(ARM7));
    }
}

static void arm7_set_context(void *src)
{
    if (src)
    {
        memcpy(&ARM7, src, sizeof(ARM7));
    }
}

static offs_t arm7_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
    if (T_IS_SET(GET_CPSR))
    {
        return thumb_disasm(buffer, pc, oprom[0] | (oprom[1] << 8)) | 2;
    }
    else
    {
        return arm7_disasm(buffer, pc, oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24)) | 4;
    }
}


// fba interface
int arm7MapArea(unsigned int nStart, unsigned int nEnd, int nMode, unsigned char *Mem)
{
	int s = nStart >> ARM7_MEM_SHIFT;
	int e = (nEnd + ARM7_MEM_MASK)>>ARM7_MEM_SHIFT;
	int i;
	for (i= s; i < e; i++) {
		switch (nMode) {
			case 0:
				ARM7.ppMemRead[i] = (unsigned int)Mem - nStart;
				break;
			case 1:
				ARM7.ppMemWrite[i] = (unsigned int)Mem - nStart;
				break;
			case 2:
				ARM7.ppMemFetch[i] = (unsigned int)Mem - nStart;
				ARM7.ppMemFetchData[i] = (unsigned int)Mem - nStart;
				break;
		}
	}

	return 0;
}


void arm7SetReadHandler(unsigned int (*pHandler)(unsigned int))
{
	ARM7.ReadHandler = pHandler;
}

void arm7SetWriteHandler(void (*pHandler)(unsigned int, unsigned int))
{
	ARM7.WriteHandler = pHandler;
}
int ArmScan(int nAction)
{
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 0;
	}

	char szText[] = "ARM #0";

	ScanVar(&ARM7, (unsigned int)&(ARM7.ppMemRead)-(unsigned int)&ARM7, szText);

	return 0;
}

/* TEST COPROC CALLBACK HANDLERS - Used for example on how to implement only */
#if TEST_COPROC_FUNCS

static WRITE32_HANDLER(test_do_callback)
{
    LOG(("test_do_callback opcode=%x, =%x\n", offset, data));
}
static READ32_HANDLER(test_rt_r_callback)
{
    UINT32 data=0;
    LOG(("test_rt_r_callback opcode=%x\n", offset));
    return data;
}
static WRITE32_HANDLER(test_rt_w_callback)
{
    LOG(("test_rt_w_callback opcode=%x, data from ARM7 register=%x\n", offset, data));
}
static void test_dt_r_callback(UINT32 insn, UINT32 *prn, UINT32 (*read32)(UINT32 addr))
{
    LOG(("test_dt_r_callback: insn = %x\n", insn));
}
static void test_dt_w_callback(UINT32 insn, UINT32 *prn, void (*write32)(UINT32 addr, UINT32 data))
{
    LOG(("test_dt_w_callback: opcode = %x\n", insn));
}
#endif
