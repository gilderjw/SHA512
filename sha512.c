#include "sha512.h"

int main(int argc, char const *argv[]) {
  if(argc != 2) {
    printf("usage: sha512 <filepath>\n");
    return -1;
  }

  FILE *input = fopen(argv[1], "rb");

  if (!input) {
    printf("Bad filepath\n");
    return -2;
  }

  uint16_t blockcount;
  uint64_t* inputstring = preprocess(input, &blockcount);

  uint64_t buffers[] = {
    0x6A09E667F3BCC908, 0xBB67AE8584CAA73B, 0x3C6EF372FE94F82B, 0xA54FF53A5F1D36F1,
    0x510E527FADE682D1, 0x9B05688C2B3E6C1F, 0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179
  };

  calculateHash(blockcount, inputstring, (uint64_t*) &buffers);

  printf("Final hash: ");
  for (int i = 0; i < 8; i++) {
    printf("%016llx",(unsigned long long) buffers[i]);
  }
  printf("\n");
  return 0;
}

void calculateHash(uint16_t blockcount, uint64_t* inputstring, uint64_t* buffers) {
  uint64_t inits[] = {
    0x6A09E667F3BCC908, 0xBB67AE8584CAA73B, 0x3C6EF372FE94F82B, 0xA54FF53A5F1D36F1,
    0x510E527FADE682D1, 0x9B05688C2B3E6C1F, 0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179
  };

  for (int block = 0; block < blockcount; block++) {
    uint64_t schedule[80];
    getwtschedule(&inputstring[block*16], schedule);

    for (int round = 0; round < 80; round++){
      doRound(buffers, round, schedule[round]);
    }

    for (int i = 0; i < 8; i++) {
      buffers[i] += inits[i];
      inits[i] = buffers[i];
    }
  }
}

uint64_t* preprocess(FILE* input, uint16_t* blockcount) {
  //read the input file
  fseek(input, 0, SEEK_END);
  uint16_t flength = ftell(input);
  uint16_t nflength = 0;
  if((flength % BYTES_IN_WORD) + 16 > BYTES_IN_WORD){
    nflength += BYTES_IN_WORD;
  }

  //pad the buffer to 1024 bits
  nflength += flength + (BYTES_IN_WORD - (flength % BYTES_IN_WORD)); 
  *blockcount = nflength/BYTES_IN_WORD;

  rewind(input);

  printf("%d bytes\n%d paddedbytes\n%d blocks\n", flength, nflength, *blockcount);
  uint64_t *inputstring = (uint64_t*) malloc(nflength);
  memset(inputstring, 0, nflength);

  //padding begins with a 1
  ((char*) inputstring)[flength] = 0x80;

  //set the length of the input
  ((uint64_t*) inputstring)[(nflength/sizeof(uint64_t)) -1] = endianSwap64(flength*8); 

  fread(inputstring, 1, flength, input);
  fclose(input);

  return inputstring;
}

uint64_t rotr(uint64_t input, uint8_t amnt) {
  return (input >> amnt) | 
         (input << (64 - amnt));
}

void doRound(uint64_t* input, uint8_t roundNumber, uint64_t word) {
  uint64_t maj = (input[0] & input[1]) ^ 
                 (input[0] & input[2]) ^ 
                 (input[1] & input[2]);

  uint64_t ch = (input[4] & input[5]) ^
                (~input[4] & input[6]);

  uint64_t suma = rotr(input[0], 28) ^
                  rotr(input[0], 34) ^
                  rotr(input[0], 39);

  uint64_t sume = rotr(input[4], 14) ^
                  rotr(input[4], 18) ^
                  rotr(input[4], 41);

  uint64_t haddthing = input[7] + sume + ch + constants[roundNumber] + word;

  input[7] = input[6];
  input[6] = input[5];
  input[5] = input[4];
  input[4] = input[3] + haddthing;
  input[3] = input[2];
  input[2] = input[1];
  input[1] = input[0];
  input[0] = suma + maj + haddthing;
}

uint64_t endianSwap64(uint64_t in) {
  uint8_t *input = (uint8_t*) &in;

  for(int i = 0; i < sizeof(uint64_t)/2; i++) {
    uint8_t tmp = input[i];
    input[i] = input[sizeof(uint64_t)-1 - i];
    input[sizeof(uint64_t)-1 - i] = tmp;
  }

  return (uint64_t) *((uint64_t*) input);
}

void getwtschedule(uint64_t *m, uint64_t *schedule) {
  for (int i = 0; i < 80; i++) {
    if (i < 16) {
      schedule[i] = endianSwap64(m[i]);
      continue;
    }
    schedule[i] = schedule[i - 16] + 
                  (rotr(schedule[i-15], 1) ^ rotr(schedule[i-15], 8) ^ (schedule[i-15] >> 7)) +
                  schedule[i-7] +
                  (rotr(schedule[i-2], 19) ^ rotr(schedule[i-2], 61) ^ (schedule[i-2] >> 6));
  }
}
