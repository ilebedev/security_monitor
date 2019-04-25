#ifndef SECURITY_MONITOR_DATA_STRUCTURES_H
#define SECURITY_MONITOR_DATA_STRUCTURES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sha3/sha3.h>
#include <constants.h>

typedef uint64_t phys_ptr_t;
typedef uint64_t hash_t[8];

#define NUM_REGISTERS 32

typedef uint64_t core_states_t[NUM_REGISTERS];

// ATOMIC FLAG

typedef struct {
   uint64_t flag;
   uint64_t pad[7];
}atomic_flag_t;

#define aquireLock(lock) ({ unsigned long __tmp; \
      asm volatile ("amoswap.w.aq %[result], %[value], (%[address])": [result] "=r"(__tmp) : [value] "r"(1), [address] "r"(&(lock.flag))); \
      __tmp; })

#define releaseLock(lock) ({ \
      asm volatile ("amoswap.w.r1 x0, x0, (%[address])":: [address] "r"(&(lock.flag))); })

// ENCLAVE AND THREAD IDs

typedef phys_ptr_t enclave_id_t;
typedef phys_ptr_t thread_id_t;

// MAILBOX

typedef int64_t mailbox_id_t;

typedef struct{
   enclave_id_t sender;
   bool has_message;
   uint8_t message[MAILBOX_SIZE];
}mailbox_t;

// ENCLAVE

typedef struct {
   bool initialized;
   bool debug;
   int64_t thread_count;
   int64_t dram_bitmap;
   sha3_ctx_t sha3_ctx;
   hash_t measurement;
   int64_t mailbox_count;
   mailbox_t *mailbox_array;
   uintptr_t last_phys_addr_loaded;
   uintptr_t eptbr; 
   uintptr_t evbase;
   uintptr_t evmask;
}enclave_t;

// THREAD

typedef struct {
   atomic_flag_t is_scheduled;
   bool aex_present;
   uintptr_t untrusted_pc;
   uintptr_t untrusted_sp;
   uintptr_t page_table_ptr;
   uintptr_t entry_pc;
   uintptr_t entry_sp;
   uintptr_t fault_pc;
   uintptr_t fault_sp;
   core_states_t untrusted_state;
   core_states_t fault_state;
   core_states_t aex_state;
}thread_t;

// METADATE PAGE MAP

typedef uint64_t metadata_page_map_entry_t;

#define ENTRY_OWNER_ID_OFFSET 12

typedef metadata_page_map_entry_t *metadata_page_map_t; // Array of size NUM_METADATA_PAGES_PER_REGION

typedef enum { // TODO must fit into 12 bits
   metadata_invalid = 0,
   metadata_free    = 1,
   metadata_enclave = 2,
   metadata_thread  = 3,
}metadata_page_t;

// CORE

typedef struct {
   enclave_id_t owner;
   bool has_enclave_schedule;
   thread_id_t cur_thread;
   atomic_flag_t lock;
}core_t;

// DRAM REGION

typedef XLENINT dram_region_id_t;

typedef enum {
   dram_region_invalid = 0,
   dram_region_free = 1,
   dram_region_blocked = 2,
   dram_region_locked = 3,
   dram_region_owned = 4,
} dram_region_state_t;

typedef enum {
   untrusted_region = 0,
   enclave_region = 1,
   metadata_region = 2,
   security_monitor_region = 3,
}dram_region_type_t;

typedef struct {
   enclave_id_t owner;
   dram_region_type_t type;
   dram_region_state_t state;
   atomic_flag_t lock;
}dram_region_t;

// SECURITY MONITOR

typedef struct {
   core_t cores[NUM_CORES];
   dram_region_t regions[NUM_REGIONS];
   const hash_t signing_enclave_measurement;
}security_monitor_t;

#endif // SECURITY_MONITOR_DATA_STRUCTURES_H
