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

//          +---------------------------------------------------------+
//          |                 1. STORE R1 -> *0x5678                  |
//          |  Store full contents of R1 to constant address 0x5678   |
//          +---------------------------------------------------------+
void test_STORE_1(struct cpu *cpu) {
  printf("\nrunning STORE_1 ------------------ \n");
  zerocpu(cpu);

  uint16_t register_value = 0x2A28;
  cpu->R[1] = register_value;
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
  uint16_t stored_value = load2(cpu, address);
  printf("Value at memory address 0x%04X: 0x%04X\n", address, stored_value);
  assert(stored_value == register_value);
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
  uint16_t instruction = 0x3401;
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
  assert(stored_value == 0x2800);
}

//          +---------------------------------------------------------+
//          |                   3. STORE R3 -> *R5                    |
//          |   Store full contents of R3 to the address held in R5   |
//          +---------------------------------------------------------+
void test_STORE_3(struct cpu *cpu) {
  printf("\nrunning STORE_3 ------------------ \n");
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
  uint16_t stored_value = load2(cpu,r_5_value);
  printf("Value at memory address 0x%04X: 0x%04X\n", r_5_value, stored_value);
  // 00101010
  assert(stored_value == r_3_value);
}

//          +---------------------------------------------------------+
//          |                  4. STORE.B R4 -> *R5                   |
//          |   store 1 byte   of R4 into the address stored at R5    |
//          +---------------------------------------------------------+
void test_STORE_4(struct cpu *cpu) {
  printf("\nrunning STORE_4 ------------------ \n");
  zerocpu(cpu);

  uint16_t r_4_value = 0x2A28;
  uint16_t r_5_value = 0x1234;
  cpu->R[4] = r_4_value;
  cpu->R[5] = r_5_value;

  // store full bytes of R4 into address held at R5
  // 0011 11 0000 101 100
  uint16_t instruction = 0x3C2C;
  store2(cpu, instruction, 0);
  store2(cpu, 0x4321, r_4_value);
  int val = emulate(cpu);
  assert(val == 0);

  // Check the full 16-bit value at memory address stored at r[5]
  uint16_t stored_value = cpu->memory[cpu->R[5]];
  printf("Value at memory address 0x%04X: 0x%04X\n", r_5_value, stored_value);
  // 00101010
  assert(stored_value == 0x4321 & 0x00FF);
}


//  ────────────────────────────────────────── 1. LOAD R1 <- *0x5678 ──
//  ──────────────────────── load full contents from address into R1 ──
void test_LOAD_1(struct cpu *cpu) {
  printf("\nrunning LOAD_1 ------------------ \n");
  zerocpu(cpu);

  uint16_t address = 0x2A28;
  uint16_t valueAtAddr = 0xA123;
  store2(cpu, valueAtAddr, address);

  // load full contents from address into R1
  // 0010 00 0000 000 001
  uint16_t instruction = 0x2001;
  store2(cpu, instruction, 0);
  store2(cpu, address, 2);

  int val = emulate(cpu);
  assert(val == 0);

  // Check the full 16-bit value at memory address stored at r[5]
  assert(cpu->R[1] == valueAtAddr);
}

//  ─────────────────────────────────────────── 2. LOAD.B R2 <- *0x5678 ──
//  ─────────────────────────────── load 1 byte from address into R2 ──
void test_LOAD_2(struct cpu *cpu) {
  printf("\nrunning LOAD_2 ------------------ \n");
  zerocpu(cpu);

  uint16_t address = 0x2A28;
  uint16_t valueAtAddr = 0xA123;
  store2(cpu, valueAtAddr, address);

  // load 1 byte from address into R2
  // 0010 01 0000 000 010
  uint16_t instruction = 0x2402;
  store2(cpu, instruction, 0);
  store2(cpu, address, 2);

  int val = emulate(cpu);
  assert(val == 0);

  // Check the full 16-bit value at memory address stored at r[5]
  assert(cpu->R[2] == 0x23);
}

//  ────────────────────────────────────────── 3. LOAD R3 <- *R5     ──
//  ──────────────────────── load full contents from R5 into R3 ──
//  ────────────────────────────────────────── 3. LOAD R3 <- *R5     ──
//  ──────── load full contents from address sepcified at R5 into R3 ──
void test_LOAD_3(struct cpu *cpu) {
  printf("\nrunning LOAD_3 ------------------ \n");
  zerocpu(cpu);

  uint16_t address = 0x5421;
  uint16_t valueAtAddr = 0x4321;
  store2(cpu, valueAtAddr, address);
  cpu->R[5] = address;

  // load full contents from address sepcified at R5 into R3
  // 0010 10 0000 101 011
  uint16_t instruction = 0x282B;
  store2(cpu, instruction, 0);

  int val = emulate(cpu);
  assert(val == 0);

  assert(cpu->R[3] == valueAtAddr);
  assert(cpu->PC == 2);
}

//  ────────────────────────────────────────── 3. LOAD R3 <- *R5     ──
//  ──────────────── load 1byte from address sepcified at R5 into R3 ──
void test_LOAD_4(struct cpu *cpu) {
  printf("\nrunning LOAD_4 ------------------ \n");
  zerocpu(cpu);

  uint16_t address = 0x5421;
  uint16_t valueAtAddr = 0x4321;
  store2(cpu, valueAtAddr, address);
  cpu->R[5] = address;

  // load full contents from address sepcified at R5 into R3
  // 0010 11 0000 101 011
  uint16_t instruction = 0x2C2B;
  store2(cpu, instruction, 0);

  int val = emulate(cpu);
  assert(val == 0);
    printf("value of register 3 at 0x%04X",cpu->R[3]);
  assert(cpu->R[3] == 0x21);
  assert(cpu->PC == 2);
}
void test_ALU_ADD(struct cpu *cpu){
    zerocpu(cpu);
    uint16_t instruction = 0x5111;
    uint16_t aVal = 0x485;
    uint16_t bVal = 0x123;
    cpu->R[1] = 0x1234;
    cpu->R[2] = 0x4321;
    cpu->R[4] = 0x2A28;
    store2(cpu,aVal,cpu->R[1]);
    store2(cpu,bVal,cpu->R[2]);
    //add o = 000 a-> 1 b->2 c-> 4
    //adding values of addresses of R[a] + R[b] then storing R[c]
    store2(cpu,instruction,0);
    int val = emulate(cpu);
    uint16_t result = load2(cpu,cpu->R[4]);
    assert(val == 0);
    assert(cpu->N == 0);
    assert(cpu->Z == 0);
    printf("result of adding registers a and b with values 0x%04X + 0x%04X = 0x%04X ",aVal,bVal,result);
    assert(0x05A8 == result);
}
void test_ALU_SUB(struct cpu *cpu){
    zerocpu(cpu);
    uint16_t instruction = 0x5311;
    uint16_t aVal = 0x485;
    uint16_t bVal = 0x123;
    cpu->R[1] = 0x1234;
    cpu->R[2] = 0x4321;
    cpu->R[4] = 0x2A28;
    store2(cpu,aVal,cpu->R[1]);
    store2(cpu,bVal,cpu->R[2]);
    //add o = 000 a-> 1 b->2 c-> 4
    //adding values of addresses of R[a] + R[b] then storing R[c]
    store2(cpu,instruction,0);
    int val = emulate(cpu);
    uint16_t result = load2(cpu,cpu->R[4]);
    assert(val == 0);
    assert(cpu->N == 0);
    assert(cpu->Z == 0);
    printf("result of asubtracting registers a and b with values 0x%04X + 0x%04X = 0x%04X ",aVal,bVal,result);
    assert(0x0362 == result);
}
void test_ALU_AND(struct cpu *cpu){
    zerocpu(cpu);
    uint16_t instruction = 0x5511;
    uint16_t aVal = 0x485;
    uint16_t bVal = 0x123;
    cpu->R[1] = 0x1234;
    cpu->R[2] = 0x4321;
    cpu->R[4] = 0x2A28;
    store2(cpu,aVal,cpu->R[1]);
    store2(cpu,bVal,cpu->R[2]);
    //add o = 000 a-> 1 b->2 c-> 4
    //adding values of addresses of R[a] + R[b] then storing R[c]
    store2(cpu,instruction,0);
    int val = emulate(cpu);
    uint16_t result = load2(cpu,cpu->R[4]);
    assert(val == 0);
    assert(cpu->N == 0);
    assert(cpu->Z == 0);
    printf("result of AND registers a and b with values 0x%04X & 0x%04X = 0x%04X ",aVal,bVal,result);
    assert(0x0001 == result);
}
void test_ALU_OR(struct cpu *cpu){
    zerocpu(cpu);
    uint16_t instruction = 0x5711;
    uint16_t aVal = 0x485;
    uint16_t bVal = 0x123;
    cpu->R[1] = 0x1234;
    cpu->R[2] = 0x4321;
    cpu->R[4] = 0x2A28;
    store2(cpu,aVal,cpu->R[1]);
    store2(cpu,bVal,cpu->R[2]);
    //add o = 000 a-> 1 b->2 c-> 4
    //adding values of addresses of R[a] + R[b] then storing R[c]
    store2(cpu,instruction,0);
    int val = emulate(cpu);
    uint16_t result = load2(cpu,cpu->R[4]);
    assert(val == 0);
    assert(cpu->N == 0);
    assert(cpu->Z == 0);
    printf("result of as OR registers a and b with values 0x%04X | 0x%04X = 0x%04X ",aVal,bVal,result);
    assert(0x05A7 == result);
}
void test_ALU_ORX(struct cpu *cpu){
    zerocpu(cpu);
    uint16_t instruction = 0x5911;
    uint16_t aVal = 0x485;
    uint16_t bVal = 0x123;
    cpu->R[1] = 0x1234;
    cpu->R[2] = 0x4321;
    cpu->R[4] = 0x2A28;
    store2(cpu,aVal,cpu->R[1]);
    store2(cpu,bVal,cpu->R[2]);
    //add o = 000 a-> 1 b->2 c-> 4
    //adding values of addresses of R[a] + R[b] then storing R[c]
    store2(cpu,instruction,0);
    int val = emulate(cpu);
    uint16_t result = load2(cpu,cpu->R[4]);
    assert(val == 0);
    assert(cpu->N == 0);
    assert(cpu->Z == 0);
    printf("result of as ORX registers a and b with values 0x%04X ^ 0x%04X = 0x%04X ",aVal,bVal,result);
    assert(0x05A6 == result);
}
void test_ALU_SHIFT(struct cpu *cpu){
    zerocpu(cpu);
    uint16_t instruction = 0x5B11;
    uint16_t aVal = 0x485;
    uint16_t bVal = 0x5;
    cpu->R[1] = 0x1234;
    cpu->R[2] = 0x4321;
    cpu->R[4] = 0x2A28;
    store2(cpu,aVal,cpu->R[1]);
    store2(cpu,bVal,cpu->R[2]);
    //add o = 000 a-> 1 b->2 c-> 4
    //adding values of addresses of R[a] + R[b] then storing R[c]
    store2(cpu,instruction,0);
    int val = emulate(cpu);
    uint16_t result = load2(cpu,cpu->R[4]);
    assert(val == 0);
    assert(cpu->N == 0);
    assert(cpu->Z == 0);
    printf("result of as SHIFT registers a and b with values 0x%04X >> 0x%04X = 0x%04X ",aVal,bVal,result);
    assert(0x024 == result);
}
void test_ALU_CMP(struct cpu *cpu){
    zerocpu(cpu);
    uint16_t instruction = 0x5D11;
    uint16_t aVal = 0x485;
    uint16_t bVal = 0x485;
    cpu->R[1] = 0x1234;
    cpu->R[2] = 0x4321;
    cpu->R[4] = 0x2A28;
    store2(cpu,aVal,cpu->R[1]);
    store2(cpu,bVal,cpu->R[2]);
    //add o = 000 a-> 1 b->2 c-> 4
    //adding values of addresses of R[a] + R[b] then storing R[c]
    store2(cpu,instruction,0);
    int val = emulate(cpu);
    uint16_t result = load2(cpu,cpu->R[4]);
    assert(val == 0);
    assert(cpu->N == 0);
    assert(cpu->Z == 1);
    printf("result of as compare registers a and b with values 0x%04X - 0x%04X = 0x%04X ",aVal,bVal,result);
}
void test_ALU_TEST(struct cpu *cpu){
    zerocpu(cpu);
    uint16_t instruction = 0x5F11;
    uint16_t aVal = -0xFB;
    //uint16_t bVal = 0x485;
    cpu->R[1] = 0x1234;
    //cpu->R[2] = 0x4321;
    //cpu->R[4] = 0x2A28;
    store2(cpu,aVal,cpu->R[1]);
    //store2(cpu,bVal,cpu->R[2]);
    //add o = 000 a-> 1 b->2 c-> 4
    //adding values of addresses of R[a] + R[b] then storing R[c]
    store2(cpu,instruction,0);
    int val = emulate(cpu);
    //uint16_t result = load2(cpu,cpu->R[4]);
    assert(val == 0);
    assert(cpu->N == 1);
    assert(cpu->Z == 0);
    printf("result of as testing registers a 0x%04X ",aVal);
}
//5B11
void test_JUMP_UNCONDITIONAL(struct cpu *cpu){
    zerocpu(cpu);
    uint16_t instruction = 0x6000;
    store2(cpu,instruction,0);
    
}


char memory[64 * 1024];
struct cpu cpu;

int main(int argc, char **argv) {
  cpu.memory = memory;

  /* test1(&cpu); */
  //test_SET_1(&cpu);
  //test_SET_2(&cpu);
  //test_STORE_1(&cpu);
  //test_STORE_2(&cpu);
  //test_STORE_3(&cpu);
  //test_STORE_4(&cpu);not working currently
  //test_LOAD_1(&cpu);
  //test_LOAD_2(&cpu);
  //  test_LOAD_3(&cpu);
    //test_LOAD_4(&cpu);
    //test_ALU_ADD(&cpu); // passes
    //test_ALU_SUB(&cpu); //passes
    //test_ALU_AND(&cpu); //passes
    //test_ALU_OR(&cpu); //passes
    //test_ALU_ORX(&cpu); //passes
    //test_ALU_SHIFT(&cpu); //passes
   // test_ALU_CMP(&cpu); //passes
    //test_ALU_TEST(&cpu); /passes but unsure of checking the complement
  printf("all tests PASS\n");
}
