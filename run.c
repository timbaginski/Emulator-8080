#include <stdlib.h> 
#include <stdio.h> 
#include <string.h>
#include "emulator.h"

void load_invaders_chunk(char *folder, char chunk, uint8_t *memory) {
  // create the necessary file path
  size_t len = strlen(folder); 
  char *folder_path = (char *) calloc(len + 16, sizeof(*folder_path)); 

  sprintf(folder_path, "%s/invaders.%c", folder, chunk);
  // open file 
  FILE *f = fopen(folder_path, "rb");
  if(f == NULL){
    exit(1); 
  }

  fseek(f, 0, SEEK_END);
  int size = ftell(f); 
  fseek(f, 0, SEEK_SET); 

  int offset = 0; 

  switch (chunk) {
    case 'h':
        offset = 0x0000; 
        break;

    case 'g':
	offset = 0x0800; 
	break;

    case 'f':
	offset = 0x1000; 
        break;

    case 'e':
	offset = 0x1800; 
        break;
      
  }
  fread(memory + offset, size, 1, f);  

  free(folder_path);
  fclose(f);

} 

void load_invaders(uint8_t *memory, char *folder) {
  // load each chunk of invaders into memory
  load_invaders_chunk(folder, 'h', memory); 
  load_invaders_chunk(folder, 'g', memory); 
  load_invaders_chunk(folder, 'f', memory); 
  load_invaders_chunk(folder, 'e', memory);

}

void print_state(State8080 *state){
  printf("a: %d\n", state->a);
  printf("b: %d\n", state->b);
  printf("c: %d\n", state->c);
  printf("d: %d\n", state->d); 
  printf("e: %d\n", state->e); 
  printf("h: %d\n", state->h);
  printf("l: %d\n", state->l); 
  printf("cc.z: %d\n", state->cc.z); 
  printf("cc.s: %d\n", state->cc.s); 
  printf("cc.p: %d\n", state->cc.p); 
  printf("cc.cy: %d\n", state->cc.cy);
  printf("cc.ac: %d\n", state->cc.ac);

}

int main(){
  // initialize state
  State8080 *state; 
  state = (State8080 *) malloc(sizeof(State8080 *)); 
  state->a = 0; 
  state->b = 0; 
  state->c = 0; 
  state->d = 0; 
  state->e = 0;
  state->h = 0; 
  state->l = 0; 
  state->sp = 0; 
  state->pc = 0; 
  state->memory = (uint8_t *) malloc((1 << 15) * sizeof(uint8_t *)); 
  state->int_enable = 0; 
  // initialize flags
  state->cc.z = 0; 
  state->cc.s = 0; 
  state->cc.p = 0; 
  state->cc.cy = 0; 
  state->cc.ac = 0; 
  // load space invaders into memory 
  load_invaders(state->memory, "rom");
  
  // run the file
  for(int i = 0; i < 10; i++){
    printf("opcode: %x\n", state->memory[state->pc]); 
    emulate(state); 
    print_state(state);
  }
  
  free(state->memory);
  free(state); 
  return 0;
}


