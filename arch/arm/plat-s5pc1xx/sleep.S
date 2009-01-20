/* linux/arch/arm/plat-s3c64xx/sleep.S
 *
 * Copyright (c) 2004 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * S3C6410 Power Manager (Suspend-To-RAM) support
 *
 * Based on PXA/SA1100 sleep code by:
 *	Nicolas Pitre, (c) 2002 Monta Vista Software Inc
 *	Cliff Brake, (c) 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <linux/linkage.h>
#include <asm/assembler.h>
#include <mach/hardware.h>
#include <mach/map.h>

#include <plat/regs-gpio.h>
#include <plat/regs-clock.h>
#include <mach/regs-mem.h>
#include <plat/regs-serial.h>

#define S5P_INFORM0 0xE0108400

/* CONFIG_DEBUG_RESUME is dangerous if your bootloader does not
 * reset the UART configuration, only enable if you really need this!
*/
	.text

	/* s5pc100_cpu_save
	 *
	 * save enough of the CPU state to allow us to re-start
	 * pm.c code. as we store items like the sp/lr, we will
	 * end up returning from this function when the cpu resumes
	 * so the return value is set to mark this.
	 *
	 * This arangement means we avoid having to flush the cache
	 * from this code.
	 *
	 * entry:
	 *	r0 = pointer to save block
	 *
	 * exit:
	 *	r0 = 0 => we stored everything
	 *	     1 => resumed from sleep
	*/

ENTRY(s5pc100_cpu_save)
	stmfd	sp!, { r4 - r12, lr }

	@@ store co-processor registers

	mrc	p15, 0, r4, c13, c0, 0	@ Read FCSE PID
	mrc	p15, 0, r5, c3, c0, 0	@ Read Domain ID
	mrc	p15, 0, r6, c2, c0, 0	@ Read translation table base address
	mrc	p15, 0, r7, c1, c0, 0	@ Read control register

	stmia	r0, { r4 - r13 }

	mov	r0, #0
	ldmfd	sp, { r4 - r12, pc }

	@@ return to the caller, after having the MMU
	@@ turned on, this restores the last bits from the
	@@ stack
resume_with_mmu:
	mov	r0, #1

	/* delete added mmu table list */
	ldr	r9 ,=0xC0000000
	bic	r8, r8, #0xF0000000
	orr	r8, r8, r9
	str	r12, [r8]

	ldmfd	sp!, { r4 - r12, pc }

	.ltorg

	@@ the next bits sit in the .data segment, even though they
	@@ happen to be code... the s5pc100_sleep_save_phys needs to be
	@@ accessed by the resume code before it can restore the MMU.
	@@ This means that the variable has to be close enough for the
	@@ code to read it... since the .text segment needs to be RO,
	@@ the data segment can be the only place to put this code.

	.data

	.global	s5pc100_sleep_save_phys
s5pc100_sleep_save_phys:
	.word	0


	/* sleep magic, to allow the bootloader to check for an valid
	 * image to resume to. Must be the first word before the
	 * s5pc100_cpu_resume entry.
	*/

	.word	0x2bedf00d

	/* s5pc100_cpu_resume
	 *
	 * resume code entry for bootloader to call
	 *
	 * we must put this code here in the data segment as we have no
	 * other way of restoring the stack pointer after sleep, and we
	 * must not write to the code segment (code is read-only)
	*/

ENTRY(s5pc100_cpu_resume)
	mov	r0, #PSR_I_BIT | PSR_F_BIT | SVC_MODE
	msr	cpsr_c, r0

	@@ load UART to allow us to print the two characters for
	@@ resume debug

	mov	r1, #0
	mcr	p15, 0, r1, c8, c7, 0		@@ invalidate TLBs
	mcr	p15, 0, r1, c7, c5, 0		@@ invalidate I Cache

	ldr	r0, s5pc100_sleep_save_phys	@ address of restore block
	ldmia	r0, { r4 - r13 }

	mcr	p15, 0, r4, c13, c0, 0		@ PID
	mcr	p15, 0, r5, c3, c0, 0		@ Domain ID
	mcr	p15, 0, r6, c2, c0, 0		@ translation table base

	/* calculate first section address into r8 */
	mov	r8, r6
	ldr	r10, =0x3fff
	bic	r8, r8, r10
	ldr	r11, =S5P_INFORM0
	ldr	r9, [r11, #0]
	mov	r9, r9 ,LSR #18
	bic	r9, r9, #0x3
	orr	r8, r8, r9

	/* calculate mmu list value into r9 */
	mov 	r9, r9, LSL #18
	ldr	r10, =0x40e
	orr	r9, r9, r10

	/* back up originally data */

	ldr	r12, [r8]

	/* Added list about mmu */
	str	r9, [r8]

	ldr	r2, =resume_with_mmu
	mcr	p15, 0, r7, c1, c0, 0		@ turn on MMU, etc
	nop					@ second-to-last before mmu
	mov	pc, r2				@ go back to virtual address

	.ltorg
