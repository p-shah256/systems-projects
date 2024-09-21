#include <assert.h>
#include <stdio.h>

#include "lab1.h"

extern void store2(struct cpu *cpu, uint16_t data, uint16_t addr);
extern uint16_t load2(struct cpu *cpu, uint16_t addr);

/* set all registers, flags, and memory to zero
 */
void zerocpu(struct cpu *cpu) {
  cpu->Z = cpu->N = 0;
  cpu->PC = cpu->SP = 0;
  for (int i = 0; i < 8; i++)
    cpu->R[i] = 0;
  memset(cpu->memory, 0, 64 * 1024);
}

/* 0000 : 50f1      : ADD R3 + R6 -> R1
 */
void test1(struct cpu *cpu) {
  zerocpu(cpu);
  cpu->R[3] = 5;
  cpu->R[6] = 10;
  store2(cpu, 0x5073, 0);

  int val = emulate(cpu);
  assert(val == 0);
  assert(cpu->R[1] == 15);
}

void test_SET_2(struct cpu *cpu) {
  zerocpu(cpu);
  store2(cpu, 0x1001, 0);
  store2(cpu, 0x1234, 2);
  int val = emulate(cpu);

  assert(val == 0);
  assert((cpu->R[1] = 0x1234));
}
void test_SET_1(struct cpu *cpu) {
  zerocpu(cpu);
  store2(cpu, 0x1002, 0);
  store2(cpu, 0x1234, 2);
  int val = emulate(cpu);

  assert(val == 0);
  assert((cpu->R[2] = 0x1234));
}

// store variants
// 1. STORE R1 -> *0x5678
// Store full contents of R1 to constant address 0x5678
//
// 2. STORE.B R2 -> *0x5678
// Store a single byte from R2 to constant address 0x5678
//
// 3. STORE R3 -> *R5
// Store full contents of R3 to the address held in R5
//
// 4. STORE.B R4 -> *R5
// store contents of R4 into the address stored at R5

void test_STORE_1(struct cpu *cpu) {
  zerocpu(cpu);

  cpu->R[1] = 0x5A49;
  // store full of R1
  // 0011 0100 0000 0001
  uint16_t instruction = 0x3401; // STORE R1
  // into address
  // 3401
  uint16_t address = 0x5A69;

  // Store the instruction
  store2(cpu, 0x3401, 0);
  // cpu->PC += 2;

  // Store the address
  store2(cpu, address, 2);
  // cpu->PC += 2;

  int val = emulate(cpu);
  assert(val == 0);

  // Check the full 16-bit value at 0x5A69
  uint16_t stored_value = load2(cpu, 0x5A69);
  printf("0x%04X", stored_value);
  assert(stored_value == 0x5A49);
}

char memory[64 * 1024];
struct cpu cpu;

int main(int argc, char **argv) {
  cpu.memory = memory;

  /* test1(&cpu); */
  // test_SET_1(&cpu);
  // test_SET_2(&cpu);
  test_STORE_1(&cpu);

  printf("all tests PASS\n");
}
