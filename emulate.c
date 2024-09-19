/*
 * file:        emulate-soln.c
 * description: solution to Lab 1
 */

#include <stdio.h>
#include <stdlib.h>

#include "lab1.h"

void store2(struct cpu *cpu, uint16_t data, uint16_t addr) {
    cpu->memory[addr] = data & 0xFF;
    cpu->memory[addr+1] = (data >> 8) & 0xFF;
}

uint16_t load2(struct cpu *cpu, uint16_t addr) {
    return (cpu->memory[addr] | (cpu->memory[addr+1] << 8));
}


/* emulate(struct cpu*) - emulate a single instruction
 *     - returns 1 for halt, 0 for no halt 
 */
int emulate(struct cpu *cpu) {
    uint16_t insn = load2(cpu, cpu->PC);
    printf("instruction is 0x%04X\n", insn);
    if ((insn & 0xF000) == 0x1000 ) {
      int op = (insn >> 9) & 7;
      int c = (insn >> 6) & 7;
      int b = (insn >> 3) & 7;
      int a = insn & 7;
      cpu->R[a] = load2(cpu, cpu->PC+2);
      cpu->PC = cpu->PC+4;
    }
    return 1;
    /* your code here */
}
