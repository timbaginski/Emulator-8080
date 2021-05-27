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
 * purpose: obtain the current opcode, emulate accordingly 
 * input: State8080 state
 */
int emulate(State8080 *state) {
  unsigned char *opcode = &state->memory[state->pc]; 
  uint16_t register_pair; 
  uint16_t answer; 

  switch(*opcode) {
    case 0x00:
        break; 
    
    case 0x01:
        state->b = opcode[2]; 
	state->c = opcode[1]; 
	state->pc += 2; 
	break;
    
    case 0x02: 
	register_pair = make_word(state->b, state->c); 
	state->memory[register_pair] = state->a;
        break; 
    
    case 0x03: 
        register_pair = make_word(state->b, state->c); 
        register_pair++; 
        state->b = register_pair >> 8; 
        state->c = register_pair & 0xff; 
        break;
   
    case 0x04:
        answer = state->b + 1; 
	state->cc.z = ((answer & 0xff) == 0); 
        state->cc.s = ((answer & 0x80) != 0); 
        state->cc.p = parity(answer & 0xff); 
	state->cc.ac = carry(4, state->b, 1); 
        state->b = (answer & 0xff);      
        break;	
  
    case 0x05: 
        answer = state->b - 1; 
        state->cc.z = ((answer & 0xff) == 0);
	state->cc.s = ((answer & 0x80) != 0);
        state->cc.p = parity(answer & 0xff);
	state->cc.ac = carry(4, state->b, 1);
	state->b = (answer & 0xff);
	break;
    
    case 0x06: 
        state->b = next_byte(state); 
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
    
    	
  }
   
  
  state->pc += 1; 
  return 0; 
}

int main(int argc, const char *argv[]) {
  return 0; 
}
