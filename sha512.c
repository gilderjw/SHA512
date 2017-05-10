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

  fseek(input, 0, SEEK_END);
  uint16_t flength = ftell(input);
  uint16_t nflength = 0;
  if(flength + 16 > 128){
    nflength += 128;
  }

  nflength += flength + (128 - flength %128); //pad the buffer to 1024 bits
  uint16_t blockcount = nflength/128;


  rewind(input);

  printf("%d bytes\n%d paddedbytes\n%d blocks\n", flength, nflength, blockcount);
  uint64_t *inputstring = (uint64_t*) malloc(nflength);
  memset(inputstring, 0, nflength);

  ((char*) inputstring)[flength] = 0x80; //padding begins with a 1
  ((uint64_t*) inputstring)[(nflength/sizeof(uint64_t)) -1] = endianSwap64(flength*8); //set the length of the input

  fread(inputstring, sizeof(uint64_t), flength, input);
  fclose(input);


  for (int block = 0; block < blockcount; block++) {
    uint64_t* schedule = getwtschedule(&inputstring[block*16]);

    for (int round = 0; round < 80; round++){
      doRound(buffers, round, schedule[round]);
    }

    printf("Final hash: ");
    for (int i = 0; i < 8; i++) {
      buffers[i] += inits[i];
      inits[i] = buffers[i];
      printf("%016llx",(unsigned long long) buffers[i]);
    }
    printf("\n");
  }
  return 0;
}

uint64_t rotr(uint64_t input, uint8_t amnt) {
  return (input >> amnt) | 
         (input << (64 - amnt));
}

void doRound(uint64_t* input, uint8_t roundNumber, uint64_t word) {
  if(roundNumber == 0) {
    printf("Initial digest %llx %llx %llx %llx %llx %llx %llx %llx\n", 
          (unsigned long long) input[0],
          (unsigned long long) input[1],
          (unsigned long long) input[2],
          (unsigned long long) input[3],
          (unsigned long long) input[4],
          (unsigned long long) input[5],
          (unsigned long long) input[6],
          (unsigned long long) input[7]
    );
  }

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

  printf("Round %d %016llx %016llx %016llx %016llx %016llx %016llx %016llx %016llx\n",
    roundNumber, (unsigned long long) input[0],
                 (unsigned long long) input[1],
                 (unsigned long long) input[2],
                 (unsigned long long) input[3],
                 (unsigned long long) input[4],
                 (unsigned long long) input[5],
                 (unsigned long long) input[6],
                 (unsigned long long) input[7]
    );
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

uint64_t* getwtschedule(uint64_t *m) {
  uint64_t* schedule = (uint64_t*) malloc(79*sizeof(uint64_t));
  for (int i = 0; i < 80; i++){
    if (i < 16) {
      schedule[i] = endianSwap64(m[i]);
      printf("%d %016llx\n", i, (unsigned long long) schedule[i]);
      continue;
    }
    schedule[i] = schedule[i - 16] + 
                  (rotr(schedule[i-15], 1) ^ rotr(schedule[i-15], 8) ^ (schedule[i-15] >> 7)) +
                  schedule[i-7] +
                  (rotr(schedule[i-2], 19) ^ rotr(schedule[i-2], 61) ^ (schedule[i-2] >> 6));
    printf("%d %016llx\n", i, (unsigned long long) schedule[i]);
  }

  return schedule;
}


