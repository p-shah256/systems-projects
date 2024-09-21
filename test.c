#include <assert.h>
#include <stdint.h>
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
  printf("\nrunning SET_2 ------------------ \n");
  zerocpu(cpu);
  store2(cpu, 0x1001, 0);
  store2(cpu, 0x1234, 2);
  int val = emulate(cpu);

  assert(val == 0);
  assert((cpu->R[1] = 0x1234));
}
void test_SET_1(struct cpu *cpu) {
  printf("\nrunning SET_1 ------------------ \n");
  zerocpu(cpu);
  store2(cpu, 0x1002, 0);
  store2(cpu, 0x1234, 2);
  int val = emulate(cpu);

  assert(val == 0);
  assert((cpu->R[2] = 0x1234));
}

// store variants

// 3. STORE R3 -> *R5
// Store full contents of R3 to the address held in R5
//
// 4. STORE.B R4 -> *R5
// store contents of R4 into the address stored at R5

//          +---------------------------------------------------------+
//          |                 1. STORE R1 -> *0x5678                  |
//          |  Store full contents of R1 to constant address 0x5678   |
//          +---------------------------------------------------------+
void test_STORE_1(struct cpu *cpu) {
  printf("\nrunning STORE_1 ------------------ \n");
  zerocpu(cpu);

  uint16_t register_value = 0x2A28;
  cpu->R[1] = 0x2A28;
  // store full of R1
  // 0011 00 0000 000001
  uint16_t instruction = 0x3001; // STORE R1
  // into constant address
  // 0x5A69
  uint16_t address = 0x5A69;

  // Store the instruction
  store2(cpu, instruction, 0);
  // Store the address
  store2(cpu, address, 2);

  int val = emulate(cpu);
  assert(val == 0);

  // Check the full 16-bit value at 0x5A69
  uint16_t stored_value = load2(cpu, 0x5A69);
  printf("Value at memory address 0x%04X: 0x%04X\n", address, stored_value);
  assert(stored_value == 0x2B28);
}

//          +---------------------------------------------------------+
//          |                2. STORE.B R2 -> *0x5678                 |
//          |     Store a single byte from Rx to constant address     |
//          +---------------------------------------------------------+
void test_STORE_2(struct cpu *cpu) {
  printf("\nrunning STORE_2 ------------------ \n");
  zerocpu(cpu);

  uint16_t register_value = 0x2A28;
  cpu->R[1] = 0x2A28;
  // store 1 byte of R1
  // 0011 01 0000 000 001
  uint16_t instruction = 0x3001;
  store2(cpu, instruction, 0);
  // into constant address
  // 0x5A69
  uint16_t address = 0x5A69;
  store2(cpu, address, 2);

  int val = emulate(cpu);
  assert(val == 0);

  // Check the full 16-bit value at 0x5A69
  uint16_t stored_value = load2(cpu, 0x5A69);
  printf("Value at memory address 0x%04X: 0x%04X\n", address, stored_value);
  // 00101010
  assert(stored_value == 0x2A);
}

//          +---------------------------------------------------------+
//          |                   3. STORE R3 -> *R5                    |
//          |   Store full contents of R3 to the address held in R5   |
//          +---------------------------------------------------------+
void test_STORE_3(struct cpu *cpu) {
  printf("\nrunning STORE_2 ------------------ \n");
  zerocpu(cpu);

  uint16_t r_3_value = 0x2A28;
  uint16_t r_5_value = 0x1234;
  cpu->R[3] = r_3_value;
  cpu->R[5] = r_5_value;

  // store full bytes of R3 into address held at R5
  // 0011 10 0000 101 011
  uint16_t instruction = 0x382B;
  store2(cpu, instruction, 0);

  int val = emulate(cpu);
  assert(val == 0);

  // Check the full 16-bit value at memory address stored at r[5]
  uint16_t stored_value = load2(cpu, r_5_value);
  printf("Value at memory address 0x%04X: 0x%04X\n", r_5_value, stored_value);
  // 00101010
  assert(stored_value == r_3_value);
}

char memory[64 * 1024];
struct cpu cpu;

int main(int argc, char **argv) {
  cpu.memory = memory;

  /* test1(&cpu); */
  test_SET_1(&cpu);
  test_SET_2(&cpu);
  test_STORE_1(&cpu);
  test_STORE_2(&cpu);
  test_STORE_3(&cpu);

  printf("all tests PASS\n");
}
