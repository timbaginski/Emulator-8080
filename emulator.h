#ifndef __EMULATOR__ 
#define __EMULATOR__ 

#include <stdlib.h> 
#include <stdint.h> 

typedef struct ConditionCodes {
  uint8_t z:1; 
  uint8_t s:1; 
  uint8_t p:1; 
  uint8_t cy:1; 
  uint8_t ac:1; 
  uint8_t pad:3; 
} ConditionCodes; 

typedef struct State8080 {
  uint8_t a; 
  uint8_t b; 
  uint8_t c; 
  uint8_t d; 
  uint8_t e; 
  uint8_t h;
  uint8_t l; 
  uint16_t sp; 
  uint16_t pc; 
  uint8_t *memory; 
  struct ConditionCodes cc; 
  uint8_t int_enable;  
} State8080; 

uint16_t make_word(uint8_t left, uint8_t right);

uint16_t next_word(State8080 *state); 

int parity(uint8_t val); 

int carry(int bit, uint8_t a, uint8_t b); 

uint8_t next_byte(State8080 *state);

void flags_arithmetic(State8080 *state, uint16_t answer); 

void stax(State8080 *state, uint8_t a, uint8_t b, uint8_t c); 

void inx(uint8_t *a, uint8_t *b); 

void inr(State8080 *state, uint8_t *a);

void dcr(State8080 *state, uint8_t *a); 

void lxi(State8080 *state, uint8_t *a, uint8_t *b); 

void mvi(State8080 *state, uint8_t *a); 

void rlc(State8080 *state); 

void dad(State8080 *state, uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d);

void ldax(State8080 *state, uint8_t *a, uint8_t *b, uint8_t *c); 

void dcx(uint8_t *a, uint8_t *b); 

void rrc(State8080 *state); 

void ral(State8080 *state); 

void rar(State8080 *state); 

void shld(State8080 *state); 

void daa(State8080 *state); 

void lhld(State8080 *state); 

void cma(uint8_t *a); 

void lxi_sp(State8080 *state); 

void sta(State8080 *state, uint8_t *a); 

void inx_sp(State8080 *state); 

void inr_memory(State8080 *state, uint8_t *a, uint8_t *b);

void dcr_memory(State8080 *state, uint8_t *a, uint8_t *b); 

void mvi_memory(State8080 *state, uint8_t *a, uint8_t *b); 

void stc(State8080 *state); 

void lda(State8080 *state); 

void dcx_sp(State8080 *state); 

void cmc(State8080 *state);

void add(State8080 *state, uint8_t *a, uint8_t *b); 

void adc(State8080 *state, uint8_t *a, uint8_t *b); 

void sub(State8080 *state, uint8_t x);

void sbb(State8080 *state, uint8_t x);

void ana(State8080 *state, uint8_t x);

void xra(State8080 *state, uint8_t x); 

void ora(State8080 *state, uint8_t x); 

void cmp(State8080 *state, uint8_t x);

void ret(State8080 *state); 

void ret_cond(State8080 *state, uint8_t cond); 

void pop_pair(State8080 *state, uint8_t *hi, uint8_t *lo); 

void jmp(State8080 *state, uint16_t adr);

void jmp_cond(State8080 *state, uint8_t cond); 

void push_word(State8080 *state, uint16_t word); 

void call_adr(State8080 *state, uint16_t adr); 

void swap_ptr(uint8_t *a, uint8_t *b);

int emulate(State8080 *state); 






#endif
