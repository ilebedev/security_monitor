#include <sm.h>

// TODO : Implement...

void platform_initialize_memory_protection(sm_state_t *sm) {
  write_csr(CSR_MEVBASE, 0);
  write_csr(CSR_MEVMASK, 0xFFFFFFFFFFFFFFFF);

  uint64_t mmrbm = regions_to_bitmap(&(sm->untrusted_regions));
  write_csr(CSR_MMRBM, mmrbm);

  uint64_t mparbase = SM_STATE_ADDR;
  uint64_t mparmask = REGION_MASK;
  write_csr(CSR_MPARBASE, mparbase);
  write_csr(CSR_MPARMASK, mparmask);
}

void platform_protect_enclave_sm_handler(enclave_metadata_t *enclave_metadata, uintptr_t phys_addr) {
  enclave_metadata->platform_csr.meparbase = phys_addr;
  enclave_metadata->platform_csr.meparmask = 0x3FFF; // ~((1ul << (intlog2(size_handler) + 1)) - 1); // TODO how to not hardcode this?
  return;
}

void platform_memory_protection_enter_enclave(enclave_metadata_t *enclave_metadata) {

  swap_csr(CSR_MEVBASE, enclave_metadata->platform_csr.ev_base);
  swap_csr(CSR_MEVMASK, enclave_metadata->platform_csr.ev_mask);

  // TODO fix this
  uint64_t memrbm = regions_to_bitmap(&(enclave_metadata->regions));
  swap_csr(CSR_MEMRBM, memrbm);

  swap_csr(CSR_MEPARBASE, enclave_metadata->platform_csr.meparbase);
  swap_csr(CSR_MEPARMASK, enclave_metadata->platform_csr.meparmask);

  return;
}

void platform_update_untrusted_regions(sm_state_t* sm, uint64_t index_id, bool flag) {
  sm->untrusted_regions.flags[index_id] = flag;
  uint64_t mmrbm = regions_to_bitmap(&(sm->untrusted_regions));
  write_csr(CSR_MMRBM, mmrbm);
}

void platform_update_enclave_regions(enclave_metadata_t *enclave_metadata, uint64_t index_id, bool flag) {
  enclave_metadata->regions.flags[index_id] = flag;
}
