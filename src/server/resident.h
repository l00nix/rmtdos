#ifndef __RMTDOS_SERVER_RESIDENT_H
#define __RMTDOS_SERVER_RESIDENT_H

#include "lib16/x86.h"

extern int resident_uninstall_check();

extern void restore_interrupt_handlers();

extern void int08_handler();

extern void int28_handler();

extern void int2f_handler(struct CpuRegs *regs);

#endif // __RMTDOS_SERVER_RESIDENT_H
