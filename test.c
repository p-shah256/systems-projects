#include <stdio.h>
#include <assert.h>

#include "lab1.h"

extern void store2(struct cpu *cpu, uint16_t data, uint16_t addr);
extern uint16_t load2(struct cpu *cpu, uint16_t addr);

/* set all registers, flags, and memory to zero
 */
void zerocpu(struct cpu *cpu)
{
    cpu->Z = cpu->N = 0;
    cpu->PC = cpu->SP = 0;
    for (int i = 0; i < 8; i++)
        cpu->R[i] = 0;
    memset(cpu->memory, 0, 64*1024);
}

/* 0000 : 50f1      : ADD R3 + R6 -> R1
 */
void test1(struct cpu *cpu)
{
    zerocpu(cpu);
    cpu->R[3] = 5;
    cpu->R[6] = 10;
    store2(cpu, 0x5073, 0);

    int val = emulate(cpu);
    assert(val == 0);
    assert(cpu->R[1] == 15);
}

void test_SET(struct cpu *cpu)
{
    zerocpu(cpu);
    store2(cpu, 0x1001, 0);
    store2(cpu, 0x1234, 2);
    int val = emulate(cpu);

    printf("emulate() returned: %d\n", val);
    // Print the value of R1
    printf("R1 now contains: 0x%04X\n", cpu->R[1]);
    // Print the current PC value
    printf("PC is now: %d\n", cpu->PC);
    printf("SET instruction test completed.\n");
}

char memory[64*1024];
struct cpu cpu;

int main(int argc, char **argv)
{
    cpu.memory = memory;
    
    /* test1(&cpu); */
    test_SET(&cpu);

    printf("all tests PASS\n");
}
