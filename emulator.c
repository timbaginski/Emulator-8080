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
 * Implement the stc opcode
 */
void stc(State8080 *state){
  state->cc.cy = 1; 
}

/* 
 * Implement the lda opcode
 */
void lda(State8080 *state){
  uint16_t address = make_word(state->memory[state->pc + 1], state->memory[state->pc + 2]);
  state->a = state->memory[address]; 
  state->pc += 2; 
}

/* 
 * Implement the dcx SP opcodes
 */
void dcx_sp(State8080 *state){
  state->sp--; 
}

/*
 * Implement the CMC opcode
 */
void cmc(State8080 *state){
  state->cc.cy = !state->cc.cy; 
}

/* 
 * Implement the add opcodes
 */
void add(State8080 *state, uint8_t *a, uint8_t *b){
  uint16_t a16 = (uint16_t) *a; 
  uint16_t b16 = (uint16_t) *b;
  uint16_t answer = a16 + b16; 
  flags_arithmetic(state, answer); 
  state->cc.cy = answer > 0xff; 
  state->a = answer & 0xff; 
}

/*
 * Implement the ADC opcodes
 */
void adc(State8080 *state, uint8_t *a, uint8_t *b){
  // turn all into uint_16 so they can be added 
  uint16_t carry = (uint16_t) state->cc.cy; 
  uint16_t a16 = (uint16_t) *a; 
  uint16_t b16 = (int16_t) *b; 
  uint16_t answer = a16 + b16 + carry; 
  flags_arithmetic(state, answer); 
  state->cc.cy = answer > 0xff; 
  state->a = answer & 0xff; 
}

/* 
 * Implement the subtract opcodes
 */
void sub(State8080 *state, uint8_t x){
  uint16_t x16 = (uint16_t) x; 
  uint16_t a16 = (uint16_t)  state->a;
  uint16_t answer = a16 - x16; 
  flags_arithmetic(state, answer); 
  state->cc.cy = answer > 0xff; 
  state->a = answer & 0xff; 
}

/*
 * Implement the sbb opcodes
 */ 
void sbb(State8080 *state, uint8_t x){
  uint16_t x16 = (uint16_t) x; 
  uint16_t a16 = (uint16_t) state->a; 
  uint16_t carry = (uint16_t) state->cc.cy; 
  uint16_t answer = a16 - x16 - carry; 
  flags_arithmetic(state, answer); 
  state->cc.cy = answer > 0xff; 
  state->a = answer & 0xff; 
}

/*
 * Implement the ana opcodes
 */
void ana(State8080 *state, uint8_t x){
  uint16_t x16 = (uint16_t) x; 
  uint16_t a16 = (uint16_t) state->a; 
  uint16_t answer = a16 & x16; 
  flags_arithmetic(state, answer); 
  state->cc.cy = answer > 0xff; 
  state->a = answer & 0xff; 
}

/* 
 * Implement the xra opcodes
 */
void xra(State8080 *state, uint8_t x){
  uint16_t a16 = (uint16_t) state->a; 
  uint16_t x16 = (uint16_t) x; 
  uint16_t answer = a16 ^ x16; 
  flags_arithmetic(state, answer); 
  state->cc.cy = answer > 0xff; 
  state->a = answer & 0xff; 
}

/* 
 * Implement the ora opcodes
 */
void ora(State8080 *state, uint8_t x){
  uint16_t a16 = (uint16_t) state->a; 
  uint16_t x16 = (uint16_t) x; 
  uint16_t answer = a16 | x16; 
  flags_arithmetic(state, answer); 
  state->cc.cy = answer > 0xff; 
  state->a = answer & 0xff; 
}

/* 
 * Implement the cmp opcodes
 */
void cmp(State8080 *state, uint8_t x){
  uint16_t a16 = (uint16_t) state->a; 
  uint16_t x16 = (uint16_t) x;
  uint16_t answer = a16 - x16; 
  flags_arithmetic(state, answer); 
  state->cc.cy = answer > 0xff; 
}

/* 
 * Implement the RET function 
 */
void ret(State8080 *state){
  uint8_t byte1; 
  uint8_t byte2; 
  byte1 = state->memory[state->sp]; 
  byte2 = state->memory[state->sp + 1]; 
  state->pc = make_word(byte1, byte2); 
  state->sp = state->sp + 2; 
}

/* 
 * Implement conditional return opcodes
 */
void ret_cond(State8080 *state, uint8_t cond){
  if(cond){
    ret(state); 
  }
}

/* 
 * Implement pop opcodes
 */
void pop_pair(State8080 *state, uint8_t *hi, uint8_t *lo){
  *hi = state->memory[state->sp]; 
  *lo = state->memory[state->sp+1]; 
  state->sp = state->sp + 2; 
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

    case 0x37:
	stc(state); 
	break;

    case 0x38:
	break;

    case 0x39:; 
	uint8_t temp1 = state->sp >> 8; 
	uint8_t temp2 = state->sp & 0xff; 
	dad(state, &state->h, &state->l, &temp1, &temp2); 
	break;
    
    case 0x3a:
	lda(state); 
        break;
     
    case 0x3b:
        dcx_sp(state);	
        break;

    case 0x3c:
        inr(state, &state->a);	
        break;	

    case 0x3d:
	dcr(state, &state->a); 
	break;

    case 0x3e:
	mvi(state, &state->a); 
	break;

    case 0x3f:
	cmc(state);
	break;

    case 0x40:
	state->b = state->b; 
	break; 
 
    case 0x41:
	state->b = state->c; 
	break; 

    case 0x42:
	state->b = state->d; 
	break; 

    case 0x43:
	state->b = state->e; 
	break;

    case 0x44: 
	state->b = state->h; 
	break; 

    case 0x45:
	state->b = state->l; 
	break; 

    case 0x46: 
	state->b = state->memory[make_word(state->h, state->l)]; 
	break; 

    case 0x47:
	state->b = state->a; 
	break;

    case 0x48:
	state->c = state->b; 
	break;

    case 0x49:
        state->c = state->c; 	
	break;

    case 0x4a: 
	state->c = state->d;
	break; 

    case 0x4b: 
	state->c = state->e;
	break;

    case 0x4c: 
	state->c = state->h; 
	break; 

    case 0x4d:
	state->c = state->l; 
	break;

    case 0x4e:
	state->c = state->memory[make_word(state->h, state->l)]; 
	break; 

    case 0x4f:
	state->c = state->a; 
	break; 

    case 0x50:
        state->d = state->b;	
	break;

    case 0x51:
	state->d = state->c;
	break;

    case 0x52:
	state->d = state->d; 
	break; 

    case 0x53:
        state->d = state->e; 	
	break;

    case 0x54: 
	state->d = state->h; 
	break; 

    case 0x55: 
	state->d = state->l; 
	break; 

    case 0x56:
	state->d = state->memory[make_word(state->h, state->l)]; 
	break; 

    case 0x57:
	state->d = state->a;
	break;

    case 0x58:
	state->e = state->b; 
	break; 

    case 0x59:
	state->e = state->c; 
	break; 

    case 0x5a:
	state->e = state->d; 
	break; 

    case 0x5b:
	state->e = state->e; 
	break; 

    case 0x5c:
	state->e = state->h; 
	break; 

    case 0x5d: 
	state->e = state->l; 
	break; 

    case 0x5e: 
	state->e = state->memory[make_word(state->h, state->l)]; 
	break; 

    case 0x5f: 
	state->e = state->a; 
	break; 

    case 0x60: 
	state->h = state->b; 
	break; 

    case 0x61:
	state->h = state->c; 
	break; 

    case 0x62: 
	state->h = state->d; 
	break; 

    case 0x63:
	state->h = state->e; 
	break; 

    case 0x64:
	state->h = state->h; 
	break; 

    case 0x65:
	state->h = state->l; 
	break; 

    case 0x66:
	state->h = state->memory[make_word(state->h, state->l)]; 
	break; 

    case 0x67:
	state->h = state->a; 
	break;

    case 0x68:
        state->l = state->b;	
	break;

    case 0x69:
	state->l = state->c; 
	break; 

    case 0x6a: 
	state->l = state->d; 
	break; 

    case 0x6b:
	state->l = state->e; 
	break; 

    case 0x6c:
	state->l = state->h; 
	break; 

    case 0x6d:
	state->l = state->l; 
	break; 

    case 0x6e: 
	state->l = state->memory[make_word(state->h, state->l)]; 
	break; 

    case 0x6f: 
	state->l = state->a; 
	break; 

    case 0x70: 
	state->memory[make_word(state->h, state->l)] = state->b; 
	break; 

    case 0x71: 
	state->memory[make_word(state->h, state->l)] = state->c; 
	break; 

    case 0x72:
        state->memory[make_word(state->h, state->l)] = state->d;	
	break;

    case 0x73:
        state->memory[make_word(state->h, state->l)] = state->e; 	
	break;

    case 0x74:
        state->memory[make_word(state->h, state->l)] = state->h; 	
	break;

    case 0x75:
	state->memory[make_word(state->h, state->l)] = state->l; 
	break; 

    case 0x76:
	break;

    case 0x77:
	state->memory[make_word(state->h, state->l)] = state->a; 
	break; 

    case 0x78:
	state->a = state->b; 
	break; 

    case 0x79:
	state->a = state->c; 
	break; 

    case 0x7a:
	state->a = state->d; 
	break;

    case 0x7b:
	state->a = state->e; 
	break;

    case 0x7c:
	state->a = state->h; 
	break; 

    case 0x7d:
	state->a = state->l; 
	break; 

    case 0x7e: 
	state->a = state->memory[make_word(state->h, state->l)]; 
	break; 
        
    case 0x7f:
	state->a = state->a;
	break;

    case 0x80:
        add(state, &state->a, &state->b); 	
	break;

    case 0x81:
	add(state, &state->a, &state->c);
	break;

    case 0x82:
	add(state, &state->a, &state->d);
	break; 

    case 0x83:
	add(state, &state->a, &state->e); 
	break; 

    case 0x84:
	add(state, &state->a, &state->h);
	break; 

    case 0x85:
	add(state, &state->a, &state->l); 
	break; 

    case 0x86:
	add(state, &state->a, &state->memory[make_word(state->h, state->l)]); 
	break; 

    case 0x87:
	add(state, &state->a, &state->a); 
	break;

    case 0x88:
        adc(state, &state->a, &state->b);
        break;	

    case 0x89:
	adc(state, &state->a, &state->c); 
	break; 

    case 0x8a:
	adc(state, &state->a, &state->d);
	break; 

    case 0x8b:
	adc(state, &state->a, &state->e); 
	break; 

    case 0x8c:
	adc(state, &state->a, &state->h); 
	break; 

    case 0x8d:
	adc(state, &state->a, &state->l);
	break; 

    case 0x8e:
	adc(state, &state->a, &state->memory[make_word(state->h, state->l)]);
	break;

    case 0x8f:
	adc(state, &state->a, &state->a);
	break; 

    case 0x90:
	sub(state, state->b); 
	break; 

    case 0x91:
	sub(state, state->c); 
	break;

    case 0x92:
	sub(state, state->d); 
	break; 

    case 0x93:
	sub(state, state->e); 
	break; 
    
    case 0x94:
	sub(state, state->h); 
	break; 

    case 0x95:
	sub(state, state->l); 
	break; 

    case 0x96: 
	sub(state, state->memory[make_word(state->h, state->l)]); 
	break; 

    case 0x97:
	sub(state, state->a); 
	break;

    case 0x98:
	sbb(state, state->b); 
	break; 

    case 0x99:
	sbb(state, state->c); 
	break; 

    case 0x9a:
	sbb(state, state->d); 
	break; 

    case 0x9b:
	sbb(state, state->e); 
	break; 

    case 0x9c:
	sbb(state, state->h); 
	break; 

    case 0x9d:
	sbb(state, state->l); 
	break; 

    case 0x9e:
	sbb(state, state->memory[make_word(state->h, state->l)]); 
	break; 

    case 0x9f:
	sbb(state, state->a);
	break;

    case 0xa0:
	ana(state, state->b); 
	break; 

    case 0xa1:
	ana(state, state->c); 
	break; 

    case 0xa2:
	ana(state, state->d); 
	break; 

    case 0xa3:
	ana(state, state->e); 
	break; 

    case 0xa4:
	ana(state, state->h); 
	break; 

    case 0xa5:
	ana(state, state->l); 
	break; 

    case 0xa6:
	ana(state, state->memory[make_word(state->h, state->l)]); 
	break; 

    case 0xa7:
	ana(state, state->a); 
	break; 

    case 0xa8:
	xra(state, state->b); 
	break; 

    case 0xa9:
	xra(state, state->c); 
	break; 

    case 0xaa:
	xra(state, state->d); 
	break; 

    case 0xab:
	xra(state, state->e); 
	break; 

    case 0xac:
	xra(state, state->h); 
	break; 

    case 0xad:
	xra(state, state->l); 
	break; 

    case 0xae:
	xra(state, state->memory[make_word(state->h, state->l)]); 
	break; 

    case 0xaf:
	xra(state, state->a); 
	break;

    case 0xb0:
	ora(state, state->b); 
	break; 
	
    case 0xb1:
	ora(state, state->c); 
	break; 

    case 0xb2:
	ora(state, state->d);
	break; 

    case 0xb3:
	ora(state, state->e); 
        break; 

    case 0xb4:
	ora(state, state->h); 
        break; 

    case 0xb5:
	ora(state, state->l); 
        break; 

    case 0xb6:
	ora(state, state->memory[make_word(state->h, state->l)]); 
	break; 
    
    case 0xb7:
        ora(state, state->a); 
        break; 

    case 0xb8:
	cmp(state, state->b); 
	break;

    case 0xb9:
	cmp(state, state->c); 
	break; 

    case 0xba:
	cmp(state, state->d); 
	break; 

    case 0xbb:
	cmp(state, state->e); 
	break; 

    case 0xbc:
	cmp(state, state->h); 
	break; 

    case 0xbd:
	cmp(state, state->l); 
	break; 

    case 0xbe:
	cmp(state, state->memory[make_word(state->h, state->l)]);
	break; 

    case 0xbf:
	cmp(state, state->a); 
	break; 

    case 0xc0:
	ret_cond(state, !state->cc.z); 
	break;

    case 0xc1:
	pop_pair(state, &state->c, &state->b); 
	break; 
  }
   
  state->pc += 1; 
  return 0; 
}

int main(int argc, const char *argv[]) {
  return 0; 
}
