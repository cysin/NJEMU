/******************************************************************************

	z80.c

	Z80 CPU interface functions

******************************************************************************/

#include "emumain.h"
#ifdef USE_SUP_Z80
/* Avoid clashing with the public interface names by temporarily
 * prefixing the core symbols pulled in from sup/z80. */
#define z80_init sup_z80_init
#define z80_step sup_z80_step
#define z80_gen_nmi sup_z80_gen_nmi
#define z80_gen_int sup_z80_gen_int
#include "../sup/z80/z80.h"
#undef z80_init
#undef z80_step
#undef z80_gen_nmi
#undef z80_gen_int
#endif

/******************************************************************************
	Z80 interface
******************************************************************************/

#ifndef USE_SUP_Z80

void z80_init(void)
{
	Cz80_Init(&CZ80);
#if (EMU_SYSTEM == CPS1)
	Cz80_Set_Fetch(&CZ80, 0x0000, 0xffff, memory_region_user2 ? memory_region_user2 : memory_region_cpu2);
#if CZ80_ENCRYPTED_ROM
	if (memory_region_user2)
		Cz80_Set_Encrypt_Range(&CZ80, 0x0000, 0x8000, memory_region_user2);
#endif
	Cz80_Set_ReadB(&CZ80, z80_read_memory_8);
	Cz80_Set_WriteB(&CZ80, z80_write_memory_8);
#elif (EMU_SYSTEM == CPS2)
	Cz80_Set_Fetch(&CZ80, 0x0000, 0xffff, memory_region_cpu2);
	Cz80_Set_ReadB(&CZ80, &z80_read_memory_8);
	Cz80_Set_WriteB(&CZ80, &z80_write_memory_8);
#elif (EMU_SYSTEM == MVS || EMU_SYSTEM == NCDZ)
	Cz80_Set_Fetch(&CZ80, 0x0000, 0xffff, memory_region_cpu2);
	Cz80_Set_ReadB(&CZ80, &z80_read_memory_8);
	Cz80_Set_WriteB(&CZ80, &z80_write_memory_8);
	Cz80_Set_INPort(&CZ80, &neogeo_z80_port_r);
	Cz80_Set_OUTPort(&CZ80, &neogeo_z80_port_w);
#endif
}

void z80_reset(void)
{
	Cz80_Reset(&CZ80);
}

void z80_exit(void)
{
}

int z80_execute(int cycles)
{
	return Cz80_Exec(&CZ80, cycles);
}

void z80_set_irq_line(int irqline, int state)
{
	Cz80_Set_IRQ(&CZ80, irqline, state);
}

void z80_set_irq_callback(int (*callback)(int line))
{
	Cz80_Set_IRQ_Callback(&CZ80, callback);
}

#else /* USE_SUP_Z80 */

static z80 SupZ80;
static int sup_z80_icount;

static uint8_t sup_read_byte(void *ctx, uint16_t address)
{
	(void)ctx;
	return z80_read_memory_8(address);
}

static void sup_write_byte(void *ctx, uint16_t address, uint8_t data)
{
	(void)ctx;
	z80_write_memory_8(address, data);
}

static uint8_t sup_port_in(z80 *cpu, uint8_t port)
{
	(void)cpu;
	(void)port;
	return 0xff;
}

static void sup_port_out(z80 *cpu, uint8_t port, uint8_t value)
{
	(void)cpu;
	(void)port;
	(void)value;
}

static void sup_setup_callbacks(void)
{
	SupZ80.read_byte  = sup_read_byte;
	SupZ80.write_byte = sup_write_byte;
	SupZ80.port_in    = sup_port_in;
	SupZ80.port_out   = sup_port_out;
	SupZ80.userdata   = NULL;
}

void z80_init(void)
{
	sup_z80_init(&SupZ80);
	sup_setup_callbacks();
}

void z80_reset(void)
{
	sup_z80_init(&SupZ80);
	sup_setup_callbacks();
}

void z80_exit(void)
{
}

int z80_execute(int cycles)
{
	unsigned long start = SupZ80.cyc;
	sup_z80_icount = cycles;

	while (sup_z80_icount > 0)
	{
		unsigned long before = SupZ80.cyc;

		sup_z80_step(&SupZ80);

		unsigned long used = SupZ80.cyc - before;
		if (used == 0)
			used = 1;

		sup_z80_icount -= (int)used;

		if (sup_z80_icount < 0)
			sup_z80_icount = 0;
	}

	return (int)(SupZ80.cyc - start);
}

void z80_set_irq_line(int irqline, int state)
{
	if (state == CLEAR_LINE)
	{
		SupZ80.int_pending = 0;
		SupZ80.nmi_pending = 0;
		return;
	}

	if (irqline == IRQ_LINE_NMI)
		sup_z80_gen_nmi(&SupZ80);
	else
		sup_z80_gen_int(&SupZ80, 0xff);
}

void z80_set_irq_callback(int (*callback)(int line))
{
	(void)callback;
}

#endif /* USE_SUP_Z80 */

int *z80_get_icount_ptr(void)
{
#ifdef USE_SUP_Z80
	return &sup_z80_icount;
#else
	return &CZ80.ICount;
#endif
}
