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


// This struct is used for simulate the cache parts
typedef struct {
    int *valid; 
    uint32_t *tag; 
    int last_used; // for fully associative cache, if -1 it means is direct mapped
} cache_entry_t;
// DECLARE CACHES AND COUNTERS FOR THE STATS HERE

uint32_t cache_size;
uint32_t block_size = 64;
cache_map_t cache_mapping;
cache_org_t cache_org;
// USE THIS FOR YOUR CACHE STATISTICS
cache_stat_t cache_statistics;
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

//calculate the mask for the offset and tag
void maskSetup(uint32_t* maskOffset, uint32_t* maskTag){
    *maskOffset = log2l(block_size);
    uint32_t temp = (cache_org == dm) ? log2l(cache_size/block_size) : 0; //it's temp because it's not used for the index calculation
    *maskTag = 32 - temp - *maskOffset;
}

//calculate the index by the mask
uint32_t getIndex(uint32_t address, uint32_t maskIndex){
    return (address >> maskIndex) & (cache_size/block_size - 1);
}

//calculate the tag by the mask
uint32_t getTag(uint32_t address, uint32_t maskTag){
    return (address >> (32 - maskTag));
}

int cache_access_dm(cache_entry_t *cache, mem_access_t access, uint32_t address_mask, uint32_t tag_mask){
  uint32_t index = getIndex(access.address, address_mask);
  uint32_t tag = getTag(access.address, tag_mask);

  if (cache->valid[index] == 1 && cache->tag[index] == tag){//hit
    return 1;
  }else{ //miss, so we update the cache
    cache->tag[index] = tag;
    cache->valid[index] = 1;
    return 0;
  }
  return 0;
}

int cache_access_fa(cache_entry_t *cache,mem_access_t access_t, uint32_t address_mask, uint32_t tag_mask){
  uint32_t tag = getTag(access_t.address, tag_mask);

  for(int i = 0; i < cache_size/block_size; i++){//we have to check all the cache block
    if(cache->valid[i] == 1 && cache->tag[i] == tag){//hit
      return 1;
    }
  }//miss, so we update the cache
  cache->valid[cache->last_used] = 1;
  cache->tag[cache->last_used] = tag; 
  cache->last_used = (cache->last_used + 1);
  if(cache->last_used == cache_size/block_size){//go back to 0 if we reach the end of the cache
    cache->last_used = 0;
  }
  printf("%d\n", cache->last_used);
  return 0;
}

void initCache(cache_entry_t *cache, uint32_t num_blocks){
  cache->valid = (int*)malloc(num_blocks*sizeof(int));
  cache->tag = (uint32_t*)calloc(sizeof(uint32_t),num_blocks);

  if(cache_mapping == fa){
    cache->last_used = 0; //fully associative cache so we start at 0
  }else{
    cache->last_used = -1; // it's direct mapped so -1 means it's not used
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



  if(cache_org == sc) cache_size /= 2; //we have 2 caches organization so we divide by 2
  uint32_t num_blocks = cache_size / block_size;

  cache_entry_t cache;
  cache_entry_t cache_sc_data;
  cache_entry_t cache_sc_instruction;

  if(cache_org == sc){//we need to initialize 2 caches one for data and one for instruction
    initCache(&cache_sc_data, num_blocks);
    initCache(&cache_sc_instruction, num_blocks);
  }else{  
    initCache(&cache, num_blocks);
  }
  uint32_t offsetMask, tagMask; //we need to calculate the mask for the offset and tag

  maskSetup(&offsetMask, &tagMask);
  printf("%d %d\n", offsetMask, tagMask);
  mem_access_t access;
  while (1) {
    access = read_transaction(ptr_file);
    // If no transactions left, break out of loop
    if (access.address == 0) break;
   // printf("%d %x\n", access.accesstype, access.address);
    /* Do a cache access */
    // ADD YOUR CODE HERE
    if(cache_mapping == dm){
      if(cache_org == sc){
        if (access.accesstype == instruction){  
            cache_statistics.hits += cache_access_dm(&cache_sc_instruction, access, offsetMask, tagMask);
        }else{
              cache_statistics.hits += cache_access_dm(&cache_sc_data, access, offsetMask, tagMask);
        }
      }else{
          cache_statistics.hits+= cache_access_dm(&cache, access, offsetMask, tagMask);
      }
    }else{
      if(cache_org == sc){
        if (access.accesstype == instruction){
            cache_statistics.hits+=cache_access_fa(&cache_sc_instruction, access, offsetMask, tagMask);
        }else{
              cache_statistics.hits+=cache_access_fa(&cache_sc_data, access, offsetMask, tagMask);
        }        
      }else{
          cache_statistics.hits+=cache_access_fa(&cache, access, offsetMask, tagMask);
        }
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
  if(cache_org == sc){
    free(cache_sc_data.valid);
    free(cache_sc_data.tag);
    free(cache_sc_instruction.valid);
    free(cache_sc_instruction.tag);
  }else{
    free(cache.valid);
    free(cache.tag);
  }


  /* Close the trace file */
  fclose(ptr_file);
}
