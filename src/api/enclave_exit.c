#include <sm.h>
#include <kernel/encoding.h>
api_result_t sm_internal_enclave_exit() { // TODO: noreturn

  // Caller is authenticated and authorized by the trap routing logic : the trap handler and MCAUSE unambiguously identify the caller, and the trap handler does not route unauthorized API calls.

  // Validate inputs
  // ---------------

  /*

  */

  // Get the core's lock
  // Update core's metadata
  // Lock thread's metadata region
  // Throw away current context
  // Restore untrusted CPU context
  // Unlock thread
  // Resume untrusted context

  // NOTE: Inputs are now deemed valid.

  // Apply state transition
  // ----------------------

  sm_state_t * sm = get_sm_state_ptr();

  uint64_t core_id = read_csr(mhartid);
  sm_core_t *core_metadata = &(sm->cores[core_id]);

  region_map_t locked_regions = (const region_map_t){ 0 };

  // <TRANSACTION>
  // Get the curent core's lock
  if(!lock_core(core_id)) {
    return MONITOR_CONCURRENT_CALL;
  }

  enclave_id_t enclave_id = core_metadata->owner;
  uint64_t region_id_enclave = addr_to_region_id(enclave_id);
  enclave_metadata_t * enclave_metadata = (enclave_metadata_t *)(enclave_id);

  thread_id_t thread_id = core_metadata->thread;
  uint64_t region_id_thread = addr_to_region_id(thread_id);
  thread_metadata_t *thread_metadata = (thread_metadata_t *) thread_id;


  // Clean the core's metadata
  core_metadata->owner = OWNER_UNTRUSTED;
  core_metadata->thread = 0;

  unlock_core(core_id);
  // <\TRANSACTION>


  // <TRANSACTION>
  // Lock the thread's metadata region
  if(!add_lock_region(region_id_enclave, &locked_regions)) {
    return MONITOR_CONCURRENT_CALL;
  }

  if(!add_lock_region(region_id_thread, &locked_regions)) {
    unlock_regions(&locked_regions);
    return MONITOR_CONCURRENT_CALL;
  }

  thread_metadata->aex_present = false; // TODO: Usefull?

  // Set MSTATUS
  uint64_t mstatus_tmp = read_csr(mstatus);

  // Set TVM to 1, MPP to 1 (S mode), MPIE to 0, SIE to 1 and UIE to 0
  mstatus_tmp |= MSTATUS_TVM_MASK;
  mstatus_tmp &= (~MSTATUS_MPP_MASK);
  mstatus_tmp |= 1ul << MSTATUS_MPP_OFFSET;
  mstatus_tmp &= (~MSTATUS_MPIE_MASK);
  mstatus_tmp |= MSTATUS_SIE_MASK;
  mstatus_tmp &= (~MSTATUS_UIE_MASK);

  write_csr(mstatus, mstatus_tmp);

  // WARNING !!! Big Hack
  // TODO moove this code in platform specific functions

  swap_csr(CSR_MEVBASE, enclave_metadata->platform_csr.ev_base);
  swap_csr(CSR_MEVMASK, enclave_metadata->platform_csr.ev_mask);

  uint64_t memrbm = regions_to_bitmap(&(enclave_metadata->regions));
  swap_csr(CSR_MEMRBM, memrbm); //CSR_MEMRBM

  swap_csr(CSR_MEPARBASE, enclave_metadata->platform_csr.meparbase);
  swap_csr(CSR_MEPARMASK, enclave_metadata->platform_csr.meparmask);

  //platform_memory_protection_exit_enclave(enclave_metadata);

  swap_csr(CSR_MEATP, enclave_metadata->platform_csr.eptbr);

  // Prepare untrusted pc
  write_csr(mepc, thread_metadata->untrusted_pc);

  // Restore trap handler pc
  write_csr(mtvec, thread_metadata->untrusted_fault_pc);

  // get the core's SM pointer and push the untrusted state on the stack
  uintptr_t SM_sp = thread_metadata->untrusted_fault_sp;
  uintptr_t *regs = (uintptr_t *) SM_sp;
  // Save the registers on the stack
  for(int i = 0; i < NUM_REGISTERS; i++) {
    regs[i] = thread_metadata->untrusted_state[i];
  }

  set_csr(mie, MIP_MTIP);
  set_csr(mie, MIP_MSIP);
  set_csr(mie, MIP_SSIP);
  set_csr(mie, MIP_STIP);
  set_csr(mie, MIP_SEIP);


  // Release locks
  unlock_regions(&locked_regions);
  // </TRANSACTION>

  // Restaure registers and perform mret
  register uintptr_t t0 asm ("t0") = SM_sp;
  // Procede to mret \\ TODO : purge the core?
  asm volatile (" \
  mv sp, t0; \n \
  call .restore_regs; \n \
  j .perform_mret"  : : "r" (t0));

  return MONITOR_OK; // Unreachable
}
