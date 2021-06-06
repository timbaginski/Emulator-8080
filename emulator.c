#include <stdio.h> 
#include <stdlib.h> 
#include "emulator.h"

/*
 * combine uint8_t into uint16_t 
 */
uint16_t make_word(uint8_t left, uint8_t right) {
  uint16_t res; 
  res = (left << 8) | right; 
  return res; 
}

/* 
 * return the parity of the given value
 */
int parity(uint8_t val) {
  int num_ones = 0; 
  for(int i = 0; i < 8; i++){
    num_ones += (val >> i) & 1; 
  }
  return num_ones % 2 == 0; 
}

/* 
 * return if 1 if there was carry between bit_no and bit_no - 1, 0 otherwise
 */
int carry(int bit, uint8_t a, uint8_t b) {
  uint16_t result = a + b; 
  uint16_t carry = result ^ a ^ b; 
  return carry & (1 << bit); 
}

/* 
 * returns the next byte, updates pc 
 */
uint8_t next_byte(State8080 *state) {
  state->pc++; 
  return state->memory[state->pc]; 
}

/* 
 * purpose: set flags after arithmetic group
 */
void flags_arithmetic(State8080 *state, uint16_t answer){
  state->cc.z = ((answer & 0xff) == 0);
  state->cc.s = ((answer & 0x80) != 0);
  state->cc.p = parity(answer & 0xff);
  state->cc.ac = carry(4, state->b, 1);
}

/* 
 * implement the STAX opcodes by taking necessary registers 
 */
void stax(State8080 *state, uint8_t a, uint8_t b, uint8_t c){
  uint16_t register_pair = make_word(a, b); 
  state->memory[register_pair] = c; 
}

/* 
 * implement the INX opcodes by taking necessary registers
 */
void inx(uint8_t *a, uint8_t *b){
  uint16_t register_pair = make_word(*a, *b); 
  register_pair++; 
  *a = register_pair >> 8; 
  *b = register_pair & 0xff; 
}

/* 
 * implement the INR opcodes by taking state and necessary register 
 */
void inr(State8080 *state, uint8_t *a){
  uint16_t answer = *a + 1;
  flags_arithmetic(state, answer);
  *a = (answer & 0xff);
}
 
/* 
 * implement the DCR opcodes by taking state and necessary register
 */
void dcr(State8080 *state, uint8_t *a){
  uint16_t answer = *a - 1;
  flags_arithmetic(state, answer);
  *a = (answer & 0xff);
}

/* 
 * implement the LXI opcodes by taking state and the necessary register
 */
void lxi(State8080 *state, uint8_t *a, uint8_t *b){
  *a = state->memory[state->pc + 2];
  *b = state->memory[state->pc + 1]; 
  state->pc += 2;
}

/* 
 * implement the MVI opcode using state and the appropriate register
 */
void mvi(State8080 *state, uint8_t *a){
  *a = next_byte(state); 
}

/*
 * implement the RLC opcode using state
 */ 
void rlc(State8080 *state){
  state->cc.cy = state->a >> 7;
  state->a = (state->a << 1) | state->cc.cy;
}

/* 
 * implement the DAD opcode by taking state and the necessary registers
 */
void dad(State8080 *state, uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d){
  uint16_t register_pair = make_word(*a, *b) + make_word(*c, *d);
  state->cc.cy = ((register_pair >> 16) & 1);  
  *a = register_pair >> 8;
  *b = register_pair & 0xff;
}

/* 
 * implement the LDAX opcode by taking the necessary registers
 */
void ldax(State8080 *state, uint8_t *a, uint8_t *b, uint8_t *c){
  *a = state->memory[make_word(*b, *c)]; 
}

/* 
 * implement the dcx opcode by taking the necessary registers
 */
void dcx(uint8_t *a, uint8_t *b){
  uint16_t register_pair = make_word(*a, *b); 
  register_pair--; 
  *a = register_pair >> 8; 
  *b = register_pair & 0xff;   
}

/* 
 * implement the RRC opcode
 */
void rrc(State8080 *state){
  state->cc.cy = state->a << 7;
  state->a = (state->a << 1) | (state->cc.cy << 7);
}

/* 
 * implement the RAL opcode 
 */
void ral(State8080 *state){
  uint8_t temp; 
  temp = state->cc.cy; 
  state->cc.cy = state->a >> 7;
  state->a = (state->a << 1) | temp;  
}

/* 
 * implement the RAR opcode
 */
void rar(State8080 *state){
  uint8_t temp;
  temp = state->cc.cy; 
  state->cc.cy = state->a & 1; 
  state->a = (state->a >> 1) | (temp << 7); 
}

/* 
 * Implement shld opcode
 */
void shld(State8080 *state){
  uint16_t address = make_word(state->memory[state->pc + 1], state->memory[state->pc + 2]); 
  state->memory[address] = state->l; 
  state->memory[address + 1] = state->h; 
  state->pc += 2; 
}

/* 
 * Implement daa opcode
 */
void daa(State8080 *state){
  uint16_t res; 
  uint8_t smallest_four = state->a & 0xff; 
  if(smallest_four > 9 || state->cc.ac){
    res = state->a + 6;
    flags_arithmetic(state, res); 
    state->a = res & 0xff;
  }
  uint8_t most_four = state->a >> 4;
  if(most_four > 9 || state->cc.cy){
    most_four += 6; 
  } 
  res = (most_four << 4) | smallest_four; 
  flags_arithmetic(state, res); 
  state->a = res & 0xff;
}

/* 
 * Implement the lhld opcode
 */
void lhld(State8080 *state){
  uint16_t address = make_word(state->memory[state->pc + 1], state->memory[state->pc + 2]);
  state->memory[address] = state->l; 
  state->memory[address + 1] = state->h; 
  state->pc += 2; 
}

/* 
 * Implement the cma opcode
 */
void cma(uint8_t *a){
  *a = ~(*a); 
}

/* 
 * Implement lxi for the stack pointer
 */
void lxi_sp(State8080 *state){
  uint8_t byte2 = state->memory[state->pc + 1]; 
  uint8_t byte3 = state->memory[state->pc + 2]; 
  state->sp = make_word(byte2, byte3); 
  state->pc += 2; 
}

/* 
 * Implement sta opcode 
 */
void sta(State8080 *state, uint8_t *a){
  uint16_t address = make_word(state->memory[state->pc + 1], state->memory[state->pc + 2]);
  state->memory[address] = state->a; 
  state->pc += 2;
}

/* 
 * Implement inx for the stack pointer
 */
void inx_sp(State8080 *state){
  state->sp = state->sp + 1; 
}

/* 
 * Implement inr for memory 
 */
void inr_memory(State8080 *state, uint8_t *a, uint8_t *b){
  uint16_t address = make_word(*a, *b); 
  uint16_t answer; 
  answer = state->memory[address] + 1; 
  state->memory[address] = answer & 0xff; 
  flags_arithmetic(state, answer); 
   
}

/* 
 * Implement dcr for memory
 */
void dcr_memory(State8080 *state, uint8_t *a, uint8_t *b){
  uint16_t address = make_word(*a, *b); 
  uint16_t answer; 
  answer = state->memory[address] - 1; 
  state->memory[address] = answer & 0xff; 
  flags_arithmetic(state, answer); 
}

/* 
 * Implement MVI opcode for memory
 */
void mvi_memory(State8080 *state, uint8_t *a, uint8_t *b){
  uint16_t address = make_word(*a, *b); 
  state->memory[address] = state->memory[state->pc + 1]; 
  state->pc++;  
}

/* 
 * purpose: obtain the current opcode, emulate accordingly 
 * input: State8080 state
 */
int emulate(State8080 *state) {
  unsigned char *opcode = &state->memory[state->pc]; 

  switch(*opcode) {
    case 0x00:
        break; 
    
    case 0x01:
	lxi(state, &state->b, &state->c); 
	break;
    
    case 0x02: 
	stax(state, state->b, state->c, state->a); 
        break; 
    
    case 0x03: 
	inx(&state->b, &state->c);  
        break;
   
    case 0x04:
	inr(state, &state->b);       
        break;	
  
    case 0x05: 
	dcr(state, &state->b); 
	break;
    
    case 0x06: 
        mvi(state, &state->b);  
	break;

    case 0x07: 
        rlc(state); 
        break; 

    case 0x08: 
	break; 

    case 0x09: 
	dad(state, &state->h, &state->l, &state->b, &state->c); 
        break; 	

    case 0x0a:
	ldax(state, &state->a, &state->b, &state->c); 	
	break;

    case 0x0b: 
	dcx(&state->b, &state->c);  
        break;
    
    case 0x0c: 
	inr(state, &state->c);  
	break;
    
    case 0x0d: 
	dcr(state, &state->c); 
	break; 

    case 0x0e: 
        mvi(state, &state->c); 
	break; 

    case 0x0f:
	rrc(state); 
	break;

    case 0x10: 
	break; 

    case 0x11: 
	lxi(state, &state->d, &state->e); 
        break;	

    case 0x12:
        stax(state, state->d, state->e, state->a); 	
	break;
   
    case 0x13: 
        inx(&state->d, &state->e);
	break; 

    case 0x14:
        inr(state, &state->d);
	break; 

    case 0x15: 
	dcr(state, &state->d); 
	break; 

    case 0x16: 
        mvi(state, &state->d); 
	break; 

    case 0x17: 
        ral(state); 
	break; 

    case 0x18:
	break; 

    case 0x19: 
	dad(state, &state->h, &state->l, &state->d, &state->e); 
	break; 

    case 0x1a:
        ldax(state, &state->a, &state->d, &state->e); 
	break; 

    case 0x1b: 
        dcx(&state->d, &state->e); 
	break; 

    case 0x1c:
        inr(state, &state->e); 
	break; 

    case 0x1d: 
	dcr(state, &state->e);
        break; 

    case 0x1e: 
        mvi(state, &state->h); 
        break;

    case 0x1f: 
        rar(state); 
	break;

    case 0x20: 
	break; 

    case 0x21: 
	lxi(state, &state->h, &state->l); 
	break; 

    case 0x22: 
	shld(state); 
	break;

    case 0x23: 
	inx(&state->h, &state->l); 
	break; 

    case 0x24: 
	inr(state, &state->h); 
	break; 

    case 0x25:
        dcr(state, &state->h);	
	break;

    case 0x26: 
        mvi(state, &state->l); 
	break; 

    case 0x27: 
	daa(state); 
	break; 

    case 0x28:
	break; 

    case 0x29:
	dad(state, &state->h, &state->l, &state->h, &state->l); 
	break;

    case 0x2a:
        lhld(state); 
	break; 

    case 0x2b:
	dcx(&state->h, &state->l); 
	break; 

    case 0x2c: 
	inr(state, &state->l); 
	break; 

    case 0x2d:
	dcr(state, &state->l);
	break;

    case 0x2e:
	mvi(state, &state->l); 
	break; 

    case 0x2f: 
	cma(&state->a); 
	break; 

    case 0x30: 
	break; 

    case 0x31:
	lxi_sp(state); 
	break; 

    case 0x32:
	sta(state, &state->a); 
	break;

    case 0x33:
	inx_sp(state); 
	break;

    case 0x34: 
        inr_memory(state, &state->h, &state->l); 
	break; 

    case 0x35:
	dcr_memory(state, &state->h, &state->l); 
	break;

    case 0x36:
        mvi_memory(state, &state->h, &state->l); 
	break;

  }
   
  state->pc += 1; 
  return 0; 
}

int main(int argc, const char *argv[]) {
  return 0; 
}
