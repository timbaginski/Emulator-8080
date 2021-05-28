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
 * purpose: obtain the current opcode, emulate accordingly 
 * input: State8080 state
 */
int emulate(State8080 *state) {
  unsigned char *opcode = &state->memory[state->pc]; 
  uint16_t register_pair; 

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
        state->cc.cy = state->a >> 7; 
        state->a = (state->a << 1) | state->cc.cy; 
        break; 

    case 0x08: 
	break; 

    case 0x09: 
        register_pair = make_word(state->h, state->l) + make_word(state->b, state->c); 
	state->h = register_pair >> 8; 
        state->l = register_pair & 0xff; 
        break; 	

    case 0x0a:
        register_pair = make_word(state->b, state->c); 
        state->a = state->memory[register_pair]; 	
	break;

    case 0x0b: 
        register_pair = make_word(state->b, state->c); 
        register_pair--; 
        state->b = (register_pair >> 8); 
        state->c = register_pair & 0xff; 
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
        state->cc.cy = state->a << 7; 
        state->a = (state->a << 1) | (state->cc.cy << 7); 
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
  }
   
  state->pc += 1; 
  return 0; 
}

int main(int argc, const char *argv[]) {
  return 0; 
}
