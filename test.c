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
void testAdd1(struct cpu *cpu)
{
    zerocpu(cpu);
    cpu->R[3] = 5;
    cpu->R[6] = 10;
    store2(cpu, 0x5073, 0);

    int val = emulate(cpu);
    assert(val == 0);
    assert(cpu->R[1] == 15);
}
void testSet(struct cpu *cpu ){
    zerocpu(cpu);
    store2(cpu, 0x1001,0);
    store2(cpu,0x1234,2);
    int val = emulate(cpu);
    assert(val == 0);
    printf("0x%04X\n",cpu->R[1]);
    assert(cpu->R[1] == 4097);
    assert(cpu->PC == 4);
}
void testLoad16bitWord(struct cpu *cpu){
    zerocpu(cpu);
    store2(cpu,0x2000,0);
    store2(cpu,0x1234,2);
    int val = emulate(cpu);
    assert(val ==0);
    printf("0x%04X\n",cpu->R[1]);
    assert(cpu->R[0] == 0x1234);
    assert(cpu->PC ==4);
}
void testLoad8bitByte(struct cpu *cpu){
    zerocpu(cpu);
    store2(cpu,0x2400,0);
    store2(cpu,0x1234,2);
    int val = emulate(cpu);
    assert(val ==0);
    printf("0x%04X\n",cpu->R[1]);
    assert(cpu->R[0] == 0x1234);
    assert(cpu->PC ==4);
}
void testLoad16bitByteFromRegister(struct cpu *cpu){
    zerocpu(cpu);
    store2(cpu,0x2800,0);
    store2(cpu,0x1234,2);
    int val = emulate(cpu);
    assert(val ==0);
    printf("0x%04X\n",cpu->R[1]);
    assert(cpu->R[0] == 0x1234);
    assert(cpu->PC ==4);
}
void testLoad8bitByteFromRegister(struct cpu *cpu){
    zerocpu(cpu);
    store2(cpu,0x2C00,0);
    store2(cpu,0x1234,2);
    int val = emulate(cpu);
    assert(val ==0);
    printf("0x%04X\n",cpu->R[1]);
    assert(cpu->R[0] == 0x1234);
    assert(cpu->PC ==4);
}
void testMove(struct cpu *cpu){
    zerocpu(cpu);
    store2(cpu,0x4088,0);
    store2(cpu,0x1234,1);
    int val = emulate(cpu);
    assert(val == 0);
}
char memory[64*1024];
struct cpu cpu;

int main(int argc, char **argv)
{
    cpu.memory = memory;
    
    //testAdd1(&cpu);
    testSet(&cpu);
    testLoad16bitWord(&cpu);
    testLoad8bitByte(&cpu);
    testMove(&cpu);
    printf("all tests PASS\n");
}
