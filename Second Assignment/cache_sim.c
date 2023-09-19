#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef enum { dm, fa } cache_map_t;
typedef enum { uc, sc } cache_org_t;
typedef enum { instruction, data } access_t;

typedef struct {
  uint32_t address;
  access_t accesstype;
} mem_access_t;

typedef struct {
  uint64_t accesses;
  uint64_t hits;
  // You can declare additional statistics if
  // you like, however you are now allowed to
  // remove the accesses or hits
} cache_stat_t;

typedef struct {
    int valid;
    uint32_t tag;
} cache_entry_t;
// DECLARE CACHES AND COUNTERS FOR THE STATS HERE

uint32_t cache_size;
uint32_t block_size = 64;
cache_map_t cache_mapping;
cache_org_t cache_org;

// USE THIS FOR YOUR CACHE STATISTICS
cache_stat_t cache_statistics;
void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}
/* Reads a memory access from the trace file and returns
 * 1) access type (instruction or data access
 * 2) memory address
 */
mem_access_t read_transaction(FILE* ptr_file) {
  char type;
  mem_access_t access;

  if (fscanf(ptr_file, "%c %x\n", &type, &access.address) == 2) {
    if (type != 'I' && type != 'D') {
      printf("Unkown access type\n");
      exit(0);
    }
    access.accesstype = (type == 'I') ? instruction : data;
    return access;
  }

  /* If there are no more entries in the file,
   * return an address 0 that will terminate the infinite loop in main
   */
  access.address = 0;
  return access;
}
void directMappedMaskSetup(uint32_t* maskOffset, uint32_t* maskIndex, uint32_t* maskTag){

    *maskOffset = log2l(block_size);
    *maskIndex = log2l(cache_size/block_size);
    *maskTag = 32 - (*maskOffset + *maskIndex);

}
int cache_access_dm(cache_entry_t *cache, mem_access_t access, uint32_t address_mask, uint32_t tag_mask){
  uint32_t index = (access.address >> address_mask) & 0x3F;
  uint32_t tag = (access.address  >> tag_mask) & 0xFFFF;
  if (cache[index].valid == 1 && cache[index].tag == tag){
    return 1;
  }
  else{
    cache[index].tag = tag;
    cache[index].valid = 1;
    return 0;
  }
}
void main(int argc, char** argv) {
  // Reset statistics:
  memset(&cache_statistics, 0, sizeof(cache_stat_t));

  /* Read command-line parameters and initialize:
   * cache_size, cache_mapping and cache_org variables
   */
  /* IMPORTANT: *IF* YOU ADD COMMAND LINE PARAMETERS (you really don't need to),
   * MAKE SURE TO ADD THEM IN THE END AND CHOOSE SENSIBLE DEFAULTS SUCH THAT WE
   * CAN RUN THE RESULTING BINARY WITHOUT HAVING TO SUPPLY MORE PARAMETERS THAN
   * SPECIFIED IN THE UNMODIFIED FILE (cache_size, cache_mapping and cache_org)
   */
  if (argc != 4) { /* argc should be 2 for correct execution */
    printf(
        "Usage: ./cache_sim [cache size: 128-4096] [cache mapping: dm|fa] "
        "[cache organization: uc|sc]\n");
    exit(0);
  } else {
    /* argv[0] is program name, parameters start with argv[1] */

    /* Set cache size */
    cache_size = atoi(argv[1]);

    /* Set Cache Mapping */
    if (strcmp(argv[2], "dm") == 0) {
      cache_mapping = dm;
    } else if (strcmp(argv[2], "fa") == 0) {
      cache_mapping = fa;
    } else {
      printf("Unknown cache mapping\n");
      exit(0);
    }

    /* Set Cache Organization */
    if (strcmp(argv[3], "uc") == 0) {
      cache_org = uc;
    } else if (strcmp(argv[3], "sc") == 0) {
      cache_org = sc;
    } else {
      printf("Unknown cache organization\n");
      exit(0);
    }
  }

  /* Open the file mem_trace.txt to read memory accesses */
  FILE* ptr_file;
  ptr_file = fopen("mem_trace.txt", "r");
  if (!ptr_file) {
    printf("Unable to open the trace file\n");
    exit(1);
  }
  uint32_t num_blocks = cache_size / block_size;
  uint32_t split_num_blocks = (cache_size/2)/block_size;
  cache_entry_t* cache = malloc(sizeof(cache_entry_t) * num_blocks);

  cache_entry_t* cache_sc_data = malloc(sizeof(cache_entry_t) * split_num_blocks);

  cache_entry_t* cache_sc_instruction = malloc(sizeof(cache_entry_t) * split_num_blocks);

  for(int i = 0; i < num_blocks; i++){
      cache[i].valid = 0; //means invalid
      cache[i].tag = 0;
  }
  uint32_t offsetMask, indexMask, tagMask;
  directMappedMaskSetup(&offsetMask, &indexMask, &tagMask);
  printf("offsetMask: %"PRIu32"\t", offsetMask);
  printBits(sizeof(offsetMask), &offsetMask);
  printf("\nindexMask:  %"PRIu32"\t", indexMask);
  printBits(sizeof(indexMask), &indexMask );
  printf("\ntagMask:  %"PRIu32"\t", tagMask);
  printBits(sizeof(tagMask), &tagMask );

  mem_access_t access;
  while (1) {
    access = read_transaction(ptr_file);
    // If no transactions left, break out of loop
    if (access.address == 0) break;
    //printf("%d %x\n", access.accesstype, access.address);
    /* Do a cache access */
    // ADD YOUR CODE HERE
    if(cache_mapping == dm){
      if(cache_org == sc){
        if (access.accesstype == 'I'){  
          if(cache_access_dm(cache_sc_instruction, access, offsetMask, tagMask) == 1){
              cache_statistics.hits++;
          }
        }else{
            if(cache_access_dm(cache_sc_data, access, offsetMask, tagMask) == 1){
              cache_statistics.hits++;
            }
        }
      }else{
        if(cache_access_dm(cache, access, offsetMask, tagMask) == 1){
          cache_statistics.hits++;
        }
      }
    }
    else{
      printf("Fully associative not implemented\n");
      exit(0);
    }
    cache_statistics.accesses++;
  }

  /* Print the statistics */
  // DO NOT CHANGE THE FOLLOWING LINES!
  printf("\nCache Statistics\n");
  printf("-----------------\n\n");
  printf("Accesses: %ld\n", cache_statistics.accesses);
  printf("Hits:     %ld\n", cache_statistics.hits);
  printf("Hit Rate: %.4f\n",
         (double)cache_statistics.hits / cache_statistics.accesses);
  // DO NOT CHANGE UNTIL HERE
  // You can extend the memory statistic printing if you like!

  /* Close the trace file */
  fclose(ptr_file);
}
