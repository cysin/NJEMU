/******************************************************************************

    m68k_musashi.c

    Musashi 68000 glue layer, selectable via USE_MUSASHI_M68K.

******************************************************************************/

#include "emumain.h"

#ifdef USE_MUSASHI_M68K

#include "cpu/Musashi/m68k.h"

/* --------------------------------------------------------------------------
 * Musashi memory callbacks (global entry points Musashi expects)
 * -------------------------------------------------------------------------- */

static int musashi_icount;

unsigned int m68k_read_memory_8(unsigned int address)      { return m68000_read_memory_8(address); }
unsigned int m68k_read_memory_16(unsigned int address)     { return m68000_read_memory_16(address); }
unsigned int m68k_read_memory_32(unsigned int address)     { return ((unsigned int)m68000_read_memory_16(address) << 16) | m68000_read_memory_16(address + 2); }

unsigned int m68k_read_immediate_16(unsigned int address)  { return m68k_read_memory_16(address); }
unsigned int m68k_read_immediate_32(unsigned int address)  { return m68k_read_memory_32(address); }

unsigned int m68k_read_pcrelative_8(unsigned int address)  { return m68k_read_memory_8(address); }
unsigned int m68k_read_pcrelative_16(unsigned int address) { return m68k_read_memory_16(address); }
unsigned int m68k_read_pcrelative_32(unsigned int address) { return m68k_read_memory_32(address); }

unsigned int m68k_read_disassembler_8(unsigned int address)  { return m68k_read_memory_8(address); }
unsigned int m68k_read_disassembler_16(unsigned int address) { return m68k_read_memory_16(address); }
unsigned int m68k_read_disassembler_32(unsigned int address) { return m68k_read_memory_32(address); }

void m68k_write_memory_8(unsigned int address, unsigned int value)  { m68000_write_memory_8(address, (UINT8)value); }
void m68k_write_memory_16(unsigned int address, unsigned int value) { m68000_write_memory_16(address, (UINT16)value); }
void m68k_write_memory_32(unsigned int address, unsigned int value)
{
    m68000_write_memory_16(address, (UINT16)(value >> 16));
    m68000_write_memory_16(address + 2, (UINT16)value);
}

void m68k_write_memory_32_pd(unsigned int address, unsigned int value)
{
    /* Predecrement writes high word first, then low word. */
    m68000_write_memory_16(address + 2, (UINT16)value);
    m68000_write_memory_16(address, (UINT16)(value >> 16));
}

/* --------------------------------------------------------------------------
 * IRQ handling
 * -------------------------------------------------------------------------- */

void m68000_set_irq_line(int irqline, int state)
{
    if (irqline == IRQ_LINE_NMI)
        irqline = 7;

    if (state == CLEAR_LINE)
        m68k_set_irq(0);
    else
        m68k_set_irq(irqline);
}

void m68000_set_irq_callback(int (*callback)(int line))
{
    (void)callback; /* Musashi autovectors for us; no-op for now. */
}

/* --------------------------------------------------------------------------
 * Core lifecycle
 * -------------------------------------------------------------------------- */

void m68000_init(void)
{
    m68k_init();
    m68k_set_cpu_type(M68K_CPU_TYPE_68000);
    musashi_icount = 0;
}

void m68000_reset(void)
{
    m68k_pulse_reset();
}

void m68000_exit(void)
{
}

int m68000_execute(int cycles)
{
    musashi_icount = cycles;
    int executed = m68k_execute(cycles);
    musashi_icount -= executed;
    if (musashi_icount < 0)
        musashi_icount = 0;
    return executed;
}

UINT32 m68000_get_reg(int regnum)
{
    switch (regnum)
    {
    case M68K_PC:  return m68k_get_reg(NULL, M68K_REG_PC);
    case M68K_USP: return m68k_get_reg(NULL, M68K_REG_USP);
    case M68K_MSP: return m68k_get_reg(NULL, M68K_REG_ISP);
    case M68K_SR:  return m68k_get_reg(NULL, M68K_REG_SR);
    case M68K_D0:  return m68k_get_reg(NULL, M68K_REG_D0);
    case M68K_D1:  return m68k_get_reg(NULL, M68K_REG_D1);
    case M68K_D2:  return m68k_get_reg(NULL, M68K_REG_D2);
    case M68K_D3:  return m68k_get_reg(NULL, M68K_REG_D3);
    case M68K_D4:  return m68k_get_reg(NULL, M68K_REG_D4);
    case M68K_D5:  return m68k_get_reg(NULL, M68K_REG_D5);
    case M68K_D6:  return m68k_get_reg(NULL, M68K_REG_D6);
    case M68K_D7:  return m68k_get_reg(NULL, M68K_REG_D7);
    case M68K_A0:  return m68k_get_reg(NULL, M68K_REG_A0);
    case M68K_A1:  return m68k_get_reg(NULL, M68K_REG_A1);
    case M68K_A2:  return m68k_get_reg(NULL, M68K_REG_A2);
    case M68K_A3:  return m68k_get_reg(NULL, M68K_REG_A3);
    case M68K_A4:  return m68k_get_reg(NULL, M68K_REG_A4);
    case M68K_A5:  return m68k_get_reg(NULL, M68K_REG_A5);
    case M68K_A6:  return m68k_get_reg(NULL, M68K_REG_A6);
    case M68K_A7:  return m68k_get_reg(NULL, M68K_REG_A7);
    default:       return 0;
    }
}

void m68000_set_reg(int regnum, UINT32 val)
{
    switch (regnum)
    {
    case M68K_PC:  m68k_set_reg(M68K_REG_PC, val); break;
    case M68K_USP: m68k_set_reg(M68K_REG_USP, val); break;
    case M68K_MSP: m68k_set_reg(M68K_REG_ISP, val); break;
    case M68K_SR:  m68k_set_reg(M68K_REG_SR, val); break;
    case M68K_D0:  m68k_set_reg(M68K_REG_D0, val); break;
    case M68K_D1:  m68k_set_reg(M68K_REG_D1, val); break;
    case M68K_D2:  m68k_set_reg(M68K_REG_D2, val); break;
    case M68K_D3:  m68k_set_reg(M68K_REG_D3, val); break;
    case M68K_D4:  m68k_set_reg(M68K_REG_D4, val); break;
    case M68K_D5:  m68k_set_reg(M68K_REG_D5, val); break;
    case M68K_D6:  m68k_set_reg(M68K_REG_D6, val); break;
    case M68K_D7:  m68k_set_reg(M68K_REG_D7, val); break;
    case M68K_A0:  m68k_set_reg(M68K_REG_A0, val); break;
    case M68K_A1:  m68k_set_reg(M68K_REG_A1, val); break;
    case M68K_A2:  m68k_set_reg(M68K_REG_A2, val); break;
    case M68K_A3:  m68k_set_reg(M68K_REG_A3, val); break;
    case M68K_A4:  m68k_set_reg(M68K_REG_A4, val); break;
    case M68K_A5:  m68k_set_reg(M68K_REG_A5, val); break;
    case M68K_A6:  m68k_set_reg(M68K_REG_A6, val); break;
    case M68K_A7:  m68k_set_reg(M68K_REG_A7, val); break;
    default: break;
    }
}

int *m68000_get_icount_ptr(void)
{
    return &musashi_icount;
}

#endif /* USE_MUSASHI_M68K */
