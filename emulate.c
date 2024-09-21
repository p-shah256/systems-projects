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
        //printf("");
        //LOAD
        //printf()
        if(is_indirect == 0 && is_byte == 0){
            cpu->R[a] = load2(cpu,cpu->PC+2);

        }
        else if(is_byte == 1 && is_indirect == 0){
            cpu->R[a] = cpu->memory[(cpu->PC+2)+1]<<8;
        }
        else if(is_byte == 0 && is_indirect == 1 ){
            //0x2800
            cpu->R[a] = load2(cpu,cpu->R[b]);
        }
        else{
            //if indirec and is byte are 0 then 0x2C00
            cpu->R[a] = cpu->memory[(cpu->R[b])+1] << 8;
        }
        cpu->PC = cpu->PC+4;
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
            cpu->memory[cpu->PC+2] = cpu->memory[cpu->R[a]+1] << 8;
            cpu->PC = cpu->PC + 4;
        }
        else if(((insn & 0xFF00) >> 8) == 0x3C){
            cpu->R[b] = cpu->memory[(cpu->R[a])+1] << 8;
            cpu->PC = cpu->PC + 2;
        }
        else{
            cpu->R[b] = load2(cpu,cpu->R[a]);
            cpu->PC = cpu->PC + 2;
        }
        
        return 0;
    }
    else if((insn & 0xF000) == 0x4000){
        //MOVE
        //printf("s:%d d:%d",a,b);
        uint8_t s = (insn >> 8 ) & 0x0F;
        uint8_t d = (insn & 0XFF);
        printf("s: %d d: %d",s,d);
        d &= 0x0F;
        cpu->R[d] = cpu->R[s];
        return 0;
    }
    else if((insn & 0xF000) == 0x5000){
        //ALU
        int aluOp = (insn & 0x0E00);
        switch (aluOp) {
            case 0x0200:
                //SUB
                cpu->R[c] = cpu->R[a] - cpu->R[b];
                break;
            case 0x0400:
                //AND
                cpu->R[c] = cpu->R[a] & cpu->R[b];
                break;
            case 0x0600:
                //OR
                cpu->R[b] = cpu->R[a] | cpu->R[b];
                break;
            case 0x0800:
                //XOR
                cpu->R[c] = cpu->R[a] ^ cpu->R[b];
                break;
            case 0x0A00:
                //SHIFT R
                cpu->R[b] = cpu->R[a] >> cpu->R[b];
                break;
            case 0x0C00:
                //CMP
                //uint16_t val = cpu->R[a] - cpu->R[b];
                
                //int is_negative = (val & 0x8000) != 0;
                if(cpu->R[a] - cpu->R[b] < 0){
                    cpu->Z = 0;
                    cpu->N = 1;
                }
                else if(cpu->R[a] - cpu->R[b] == 0){
                    cpu->Z = 1;
                    cpu->N = 0;
                }
                else{
                    cpu->Z = 0;
                    cpu->N = 0;
                }
                break;
            case 0x0E00:
                //TEST
                if(cpu->R[a] > 0){
                    cpu->Z = 0;
                    cpu->N = 0;
                }
                else if(cpu->R[a] == 0){
                    cpu->Z = 1;
                    cpu->N = 0;
                }
                else{
                    cpu->Z = 0;
                    cpu->N = 1;
                }
                break;
            default:
                //ADD
                cpu->R[c] = cpu->R[a] + cpu->R[b];
                break;
        }
        cpu->PC = cpu->PC+2;
        return 0;
    }
    else if((insn & 0xF000) == 0x6000){
        //JMP_ABS
        int op_val = (insn & 0x0F00) << 4;
        if(op_val >= 7){
            return 1;
        }
        switch(op_val){
            case 0x01:
                //JMP_Z
                break;
            case 0x02:
                //JMP_NZ
                break;
            case 0x03:
                //JMP_LT
                break;
            case 0x04:
                //JMP_GT
                break;
            case 0x05:
                //JMP_LE
                break;
            case 0x06:
                //JMP_GE
                break;
            default:
                //Unconditional JMP
                break;
        }
        return 0;
    }
    else if((insn & 0xF000) == 0x8000){
        //CALL absolute
        return 0;
    }
    else if((insn & 0xF000) == 0x9000){
        //CALL register
        return 0;
    }
    else if((insn & 0xF000) == 0xA ){
        //RET
        return 0;
    }
    else if((insn & 0xF000) == 0xB){
        //PUSH
    }
    else if((insn & 0xF000) == 0xC){
        //POP
    }
    else if((insn & 0xF000) == 0xD){
        //IN
    }
    else if((insn & 0xF000) == 0xE){
        //OUT
    }
    else if((insn & 0xF000) == 0xF){
        //HALT
        return 1;
    }
    
    //

    return 1;
    /* your code here */
}
