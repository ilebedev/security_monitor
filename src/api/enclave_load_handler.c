#include <sm.h>

api_result_t sm_internal_enclave_load_handler (enclave_id_t enclave_id, uintptr_t phys_addr) {

  // Validate inputs
  // ---------------

  /*
   - enclave_id must be valid
   - enclave must be ENCLAVE_STATE_CREATED
   - phys_addr must be page alligned
   - phys_addr must be greater than the last physical address loaded (that should be zero)
   - phys_addr must point to a region owned by the enclave
   - the handler should fit in one region
  */

  enclave_metadata_t * enclave_metadata = (enclave_metadata_t *)(enclave_id);
  region_map_t locked_regions = (const region_map_t){ 0 };

  // <TRANSACTION>
  // enclave_id must be valid
  api_result_t result = add_lock_region_iff_valid_enclave(enclave_id, &locked_regions);
  if ( MONITOR_OK != result ) {
   return result;
  }


  // enclave must be ENCLAVE_STATE_CREATED
  if(enclave_metadata->init_state > ENCLAVE_STATE_CREATED) {
   unlock_regions(&locked_regions);
   return MONITOR_INVALID_VALUE;
  }

  // phys_addr must be page alligned
  if(phys_addr % PAGE_SIZE) {
   unlock_regions(&locked_regions);
   return MONITOR_INVALID_VALUE;
  }

  // phys_addr must be greater than the last physical address loaded
  if(enclave_metadata->last_phys_addr_loaded > phys_addr) {
   unlock_regions(&locked_regions);
   return MONITOR_INVALID_VALUE;
  }

  // Check that the handlers fit in one region
  uint64_t size_handler = ((uint64_t) &enclave_handler_end) - ((uint64_t) &enclave_handler_start);

  uintptr_t end_phys_addr = phys_addr + size_handler;

  // Make sure the handlers are not larger than a region
  if(addr_to_region_id(phys_addr) != addr_to_region_id(end_phys_addr)){
   unlock_regions(&locked_regions);
   return MONITOR_INVALID_VALUE;
  }

  // Check that phys_addr points into a DRAM region owned by the enclave
  if(region_owner(addr_to_region_id(phys_addr)) != enclave_id){
   unlock_regions(&locked_regions);
   return MONITOR_INVALID_STATE;
  }

  if (!add_lock_region(addr_to_region_id(phys_addr), &locked_regions)) {
   unlock_regions(&locked_regions);
   return MONITOR_CONCURRENT_CALL;
  }

  // NOTE: Inputs are now deemed valid.

  // Apply state transition
  // ----------------------

  platform_protect_enclave_sm_handler(enclave_metadata, phys_addr);

  // Copy the handlers
  memcpy((void *) phys_addr, (void *) &enclave_handler_start, size_handler);

  // Update the measurement
  //hash_extend(&enclave_metadata->hash_context, &enclave_handler_start, size_handler);

  // Update the enclave state
  enclave_metadata->init_state = ENCLAVE_STATE_HANDLER_LOADED;

  // Update the last physical address loaded
  enclave_metadata->last_phys_addr_loaded = phys_addr;

  // Release locks
  unlock_regions(&locked_regions);
  // </TRANSACTION>

  return MONITOR_OK;
}
