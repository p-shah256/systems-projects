/*
 * file:        emulate-soln.c
 * description: solution to Lab 1
 */

#include <stdio.h>
#include <stdlib.h>

#include "lab1.h"
/*struct cpu {
 uint8_t *memory;
 uint16_t R[8]; R[1] r[2]
 uint16_t PC;
 uint16_t SP;
 int Z;
 int N;
};
 */
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

int emulate(struct cpu *cpu)
{
    uint16_t insn = load2(cpu, cpu->PC);
    int op = (insn >> 9) & 7; //operation
    int c = (insn >> 6) & 7;
    int b = (insn >> 3) & 7;
    int a = insn & 7;
    printf("instruction is : 0x%04X\n",(insn & 0xF000));
    if ((insn & 0xF000) == 0x1000) {
      /* SET */
        //load value from r1 into
        cpu->R[a] = load2(cpu,cpu->PC);
        cpu->PC = cpu->PC+4;
        return 0;
        //set
    }
    //LOAD x2000 if loading constant from 16 bit word
    else if ((insn & 0xF000) == 0x2000) {
        int is_indirect = ((insn & 0x0800) != 0);
        int is_byte = ((insn & 0x0400) != 0);
        //LOAD
        //int is_indirect = ((insn & 0x0800) != 0);
        //int is_byte = ((insn & 0x0400) != 0);
        printf("indirect: %d isByte: %d",is_indirect,is_byte);
        printf("store type : 0x%02X",((insn >> 8) & 0xFF));
        printf("a:%d",a);
        //printf("");
        //LOAD
        //printf()
        if(is_indirect == 0 && is_byte == 0){
            uint16_t valFromAddr = load2(cpu,cpu->PC+2);
            cpu->R[a] = load2(cpu,valFromAddr);
            cpu->PC = cpu->PC+4;
        }
        else if(is_byte == 1 && is_indirect == 0){
            uint16_t valFromAddr = load2(cpu,cpu->PC+2);
            cpu->R[a] = cpu->memory[valFromAddr];
            cpu->PC = cpu->PC+4;
        }
        else if(is_byte == 0 && is_indirect == 1 ){
            //0x2800
            cpu->R[a] = load2(cpu,cpu->R[b]);
            cpu->PC = cpu->PC+2;
        }
        else{
            //if indirec and is byte are 0 then 0x2C00
            cpu->R[a] = cpu->memory[(cpu->R[b])];
            cpu->PC = cpu->PC+2;
        }
        
        return 0;
    }
    else if((insn & 0xF000) == 0x3000){
        //STORE
        printf("store type : 0x%02X",((insn >> 8) & 0xFF));
        printf("a:%d",a);
        
        if((insn & 0xFF00) >> 8 == 0x30){
            uint16_t addrVal = load2(cpu,cpu->PC+2);
            uint16_t rVal = cpu->R[a];
            //cpu->memory[addrVal] = cpu->R[a];
            store2(cpu,rVal,addrVal);
            cpu->PC = cpu->PC + 4;
        }
        else if(((insn & 0xFF00) >> 8) == 0x34){
                    //cpu->memory[cpu->PC+2] = cpu->memory[cpu->R[a]+1] << 8;
            uint16_t addrVal = load2(cpu,cpu->PC+2);
            uint16_t rVal = cpu->R[a];
            rVal = rVal & 0xFF;
            cpu->memory[addrVal+1] = rVal;
            cpu->PC = cpu->PC + 4;
            }
        else if(((insn & 0xFF00) >> 8) == 0x3C){
                //uint16_t addrVal = load2(cpu,cpu->PC+2);
            uint16_t rAddress = cpu->R[a];
            uint16_t addressToLoadInto = cpu->R[b];
            uint16_t valueAtRSrc = load2(cpu, rAddress);
            cpu->memory[cpu->R[b]] = valueAtRSrc & 0x00FF;
            cpu->PC = cpu->PC + 2;
            }
        else{
                    //0x3800
            uint16_t rVal = cpu->R[a];
            uint16_t addressToLoadInto = cpu->R[b];
            store2(cpu, rVal,addressToLoadInto);
            cpu->PC = cpu->PC + 2;
            }
        
        return 0;
    }
    else if((insn & 0xF000) == 0x4000){
        //MOVE
        //printf("s:%d d:%d",a,b);
        uint8_t s = insn & 0x0F;
        uint8_t d = (insn >> 4) & 0x0F;
        printf("s: %d d: %d",s,d);
        if(d == 8){
            cpu->SP = cpu->R[s];
        }
        else if(s == 8){
            cpu->R[d] = cpu->SP;
        }
        else{
            cpu->R[d] = cpu->R[s];

        }
        cpu->PC = cpu->PC + 2;
        //d &= 0x0F;
        return 0;
    }
    else if((insn & 0xF000) == 0x5000){
        //ALU
        uint16_t shiftA;
        uint16_t shiftB;
        uint16_t res;
        int aluOp = (insn & 0x0E00);
        switch (aluOp) {
            case 0x0200:
                //SUB
                //uint16_t
                store2(cpu,load2(cpu,cpu->R[a]) - load2(cpu,cpu->R[b]),cpu->R[c]);
                res = load2(cpu,cpu->R[c]);
                if(res == 0){
                    cpu->Z = 1;
                }
                else{
                    cpu->Z = 0;
                }
                if((res & 0x8000) != 0){
                    cpu->N = 1;
                }
                else{
                    cpu->N = 0;
                }
                break;
            case 0x0400:
                //AND
                store2(cpu,load2(cpu,cpu->R[a]) & load2(cpu,cpu->R[b]),cpu->R[c]);
                res = load2(cpu,cpu->R[c]);
                if(res == 0){
                    cpu->Z = 1;
                }
                else{
                    cpu->Z = 0;
                }
                if((res & 0x8000) != 0){
                    cpu->N = 1;
                }
                else{
                    cpu->N = 0;
                }
                break;
            case 0x0600:
                //OR
                store2(cpu,load2(cpu,cpu->R[a]) | load2(cpu,cpu->R[b]),cpu->R[c]);
                res = load2(cpu,cpu->R[c]);
                if(res == 0){
                    cpu->Z = 1;
                }
                else{
                    cpu->Z = 0;
                }
                if((res & 0x8000) != 0){
                    cpu->N = 1;
                }
                else{
                    cpu->N = 0;
                }
                break;
            case 0x0800:
                //XOR
                store2(cpu,load2(cpu,cpu->R[a]) ^ load2(cpu,cpu->R[b]),cpu->R[c]);
                res = load2(cpu,cpu->R[c]);
                if(res == 0){
                    cpu->Z = 1;
                }
                else{
                    cpu->Z = 0;
                }
                if((res & 0x8000) != 0){
                    cpu->N = 1;
                }
                else{
                    cpu->N = 0;
                }
                break;
            case 0x0A00:
                //SHIFT R
                shiftA =load2(cpu,cpu->R[a]);
                shiftB =load2(cpu,cpu->R[b]);
                store2(cpu, (shiftA >> shiftB) ,cpu->R[c]);
                res = load2(cpu,cpu->R[c]);
                if(res == 0){
                    cpu->Z = 1;
                }
                else{
                    cpu->Z = 0;
                }
                if((res & 0x8000) != 0){
                    cpu->N = 1;
                }
                else{
                    cpu->N = 0;
                }
                break;
            case 0x0C00:
                //CMP
                res = load2(cpu,cpu->R[a]) - load2(cpu,cpu->R[b]);
                if(res == 0){
                    cpu->Z = 1;
                }
                else{
                    cpu->Z = 0;
                }
                if((res & 0x8000) != 0){
                    cpu->N = 1;
                }
                else{
                    cpu->N = 0;
                }
                break;
            case 0x0E00:
                //TEST
                res = load2(cpu,cpu->R[a]);
                int is_negative = (res & 0x8000) != 0;
                if(res == 0){
                    cpu->Z = 1;
                }
                else{
                    cpu->Z = 0;
                }
                if(is_negative){
                    cpu->N = 1;
                }
                else{
                    cpu->N = 0;
                }
                break;
            default:
                store2(cpu,load2(cpu,cpu->R[b]) + load2(cpu,cpu->R[a]),cpu->R[c]);
                res = load2(cpu,cpu->R[c]);
                if(res == 0){
                    cpu->Z = 1;
                }
                else{
                    cpu->Z = 0;
                }
                if((res & 0x8000) != 0){
                    cpu->N = 1;
                }
                else{
                    cpu->N = 0;
                }
                break;
        }
        cpu->PC = cpu->PC+2;
        return 0;
    }
    else if((insn & 0xF000) == 0x6000){
        //JMP_ABS
        int op_val = op;
        if(op_val >= 7){
            return 1;
        }
        switch(op_val){
            case 0x00:
                //JMP
                cpu->PC = load2(cpu,cpu->PC+2);
                break;
            case 0x01:
                //JMP_Z
                if(cpu->Z == 1){
                    cpu->PC = load2(cpu,cpu->PC+2);
                }
                else{
                    cpu->PC = cpu->PC+4;
                }
                break;
            case 0x02:
                //JMP_NZ
                if(cpu->Z != 0){
                    cpu->PC = load2(cpu,cpu->PC+2);

                }
                else{
                    cpu->PC = cpu->PC+4;

                }
                break;
            case 0x03:
                //JMP_LT
                if(cpu->N == 1){
                    cpu->PC = load2(cpu,cpu->PC+2);
                }
                else{
                    cpu->PC = cpu->PC+4;

                }
                break;
            case 0x04:
                //JMP_GT
                if(cpu->N == 0 && cpu->Z == 0){
                    cpu->PC = load2(cpu,cpu->PC+2);
                }
                else{
                    cpu->PC = cpu->PC+4;

                }
                break;
            case 0x05:
                //JMP_LE
                if(cpu->N == 1 || cpu->Z == 1){
                    cpu->PC = load2(cpu,cpu->PC+2);
                }
                else{
                    cpu->PC = cpu->PC+4;

                }
                break;
            case 0x06:
                //JMP_GE
                if(cpu->N == 0){
                    cpu->PC = load2(cpu,cpu->PC+2);
                }
                else{
                    cpu->PC = cpu->PC+4;

                }
                break;
            default:
                //illegal instruction or error otherwise
                cpu->PC = cpu->PC+4;
                break;
        }
        return 0;
    }
    else if((insn & 0xF000) == 0x7000){
        int op_val = op;
        if(op_val >= 7){
            return 1;
        }
        switch(op_val){
            case 0x00:
                //JMP
                cpu->PC = cpu->R[a];
                break;
            case 0x01:
                //JMP_Z
                if(cpu->Z == 1){
                    cpu->PC = cpu->R[a];
                }
                else{
                    cpu->PC = cpu->PC+2;
                }
                break;
            case 0x02:
                //JMP_NZ
                if(cpu->Z != 0){
                    cpu->PC = cpu->R[a];

                }
                else{
                    cpu->PC = cpu->PC+2;

                }
                break;
            case 0x03:
                //JMP_LT
                if(cpu->N == 1){
                    cpu->PC = cpu->R[a];
                }
                else{
                    cpu->PC = cpu->PC+2;

                }
                break;
            case 0x04:
                //JMP_GT
                if(cpu->N == 0 && cpu->Z == 0){
                    cpu->PC = cpu->R[a];
                }
                else{
                    cpu->PC = cpu->PC+2;

                }
                break;
            case 0x05:
                //JMP_LE
                if(cpu->N == 1 || cpu->Z == 1){
                    cpu->PC = cpu->R[a];
                }
                else{
                    cpu->PC = cpu->PC+2;

                }
                break;
            case 0x06:
                //JMP_GE
                if(cpu->N == 0){
                    cpu->PC = cpu->R[a];
                }
                else{
                    cpu->PC = cpu->PC+2;

                }
                break;
            default:
                //illegal instruction or error otherwise
                cpu->PC = cpu->PC+2;
                break;
        }
        return 0;
    }
    else if((insn & 0xF000) == 0x8000){
        //CALL absolute
        cpu->SP = cpu->SP - 2;
        uint16_t ValToStore = cpu->PC+4;
        store2(cpu,ValToStore,cpu->SP);
        cpu->PC = load2(cpu,cpu->PC+2);
        return 0;
    }
    else if((insn & 0xF000) == 0x9000){
        //CALL register
        cpu->SP = cpu->SP - 2;
        uint16_t ValToStore = cpu->PC+2;
        store2(cpu,ValToStore,cpu->SP);
        cpu->PC = cpu->R[a];
        return 0;
    }
    else if((insn & 0xF000) == 0xA000 ){
        //RET
        cpu->PC = load2(cpu,cpu->SP);
        cpu->SP = cpu->SP + 2;
        return 0;
    }
    else if((insn & 0xF000) == 0xB000){
        //PUSH
        uint16_t res;
        cpu->SP = cpu->SP - 2;
        res = cpu->R[a];
        store2(cpu,res,cpu->SP);
        cpu->PC = cpu->PC+2;
        return 0;
    }
    else if((insn & 0xF000) == 0xC000){
        //POP
        uint16_t res;
        res = load2(cpu,cpu->SP);
        cpu->R[a] = res;
        cpu->SP = cpu->SP + 2;
        cpu->PC = cpu->PC+2;
        return 0;
    }
    else if((insn & 0xF000) == 0xD000){
        //IN
        cpu->R[a] = fgetc(stdin);
        cpu->PC = cpu->PC+2;
        return 0;
    }
    else if((insn & 0xF000) == 0xE000){
        //OUT
        fputc(cpu->R[a], stdout);
        cpu->PC = cpu->PC+2;
        return 0;
    }
    else if((insn & 0xF000) == 0xF000){
        //HALT
        return 1;
    }
    
    //

    return 1;
    /* your code here */
}
