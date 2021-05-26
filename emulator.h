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

#endif
