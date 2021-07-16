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

int main(){
  return 0;
}


