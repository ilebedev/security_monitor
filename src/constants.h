#ifndef CONSTANTS_H
#define CONSTANTS_H

int intlog2(int n) {
   int cnt = 0;
   while(n >>= 1) {
      cnt++;
   }
   return cnt;
}

#define XLENINT uint64_t
#define SIZE_DRAM (0x80000000)
#define SIZE_PAGE (0x1000)
#define NUM_CORES 2
#define NUM_REGIONS 64

#define SIZE_REGION (SIZE_DRAM / NUM_REGIONS)
#define NUM_METADATA_PAGES_PER_REGION (SIZE_REGION/(sizeof(metadata_page_map_entry_t) + SIZE_PAGE))

#define MAILBOX_SIZE 128

#endif // CONSTANTS_H
