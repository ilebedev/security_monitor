#include <test.h>
#include <api_untrusted.h>

//extern uintptr_t region1;
extern uintptr_t region2;
extern uintptr_t region3;

extern uintptr_t enclave_start;
extern uintptr_t enclave_end;

void test_entry(void) {
  //uint64_t region1_id = addr_to_region_id((uintptr_t) &region1);
  uint64_t region2_id = addr_to_region_id((uintptr_t) &region2);
  uint64_t region3_id = addr_to_region_id((uintptr_t) &region3);

  api_result_t result;

  print_str("\n");

  print_str("sm_block_region\n");
  result = sm_region_block(region3_id);
  if(result != MONITOR_OK) {
    print_str("sm_region_block FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  print_str("sm_region_free\n");
  result = sm_region_free(region3_id);
  if(result != MONITOR_OK) {
    print_str("sm_region_free FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  print_str("sm_region_metadata_create\n");
  result = sm_region_metadata_create(region3_id);
  if(result != MONITOR_OK) {
    print_str("sm_region_metadata_create FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  print_str("sm_region_metadata_create\n");
  uint64_t region_metadata_start = sm_region_metadata_start();

  enclave_id_t enclave_id = ((uintptr_t) &region3) + (PAGE_SIZE * region_metadata_start);
  uint64_t num_mailboxes = 1;

  print_str("sm_enclave_create\n");
  result = sm_enclave_create(enclave_id, 0x0, REGION_MASK, num_mailboxes, true);
  if(result != MONITOR_OK) {
    print_str("sm_enclave_create FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  print_str("sm_region_block\n");
  result = sm_region_block(region2_id);
  if(result != MONITOR_OK) {
    print_str("sm_region_block FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  print_str("sm_region_free\n");
  result = sm_region_free(region2_id);
  if(result != MONITOR_OK) {
    print_str("sm_region_free FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  print_str("sm_region_assign\n");
  result = sm_region_assign(region2_id, enclave_id);
  if(result != MONITOR_OK) {
    print_str("sm_region_assign FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  uintptr_t enclave_handler_address = (uintptr_t) &region2;
  uintptr_t enclave_handler_stack_pointer = enclave_handler_address + HANDLER_LEN + STACK_SIZE;

  print_str("sm_enclave_load_handler\n");
  result = sm_enclave_load_handler(enclave_id, enclave_handler_address);
  if(result != MONITOR_OK) {
    print_str("sm_enclave_load_handler FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  uintptr_t page_table_address = enclave_handler_stack_pointer;

  print_str("sm_enclave_load_page_table\n");
  result = sm_enclave_load_page_table(enclave_id, page_table_address, 0, 3, NODE_ACL);
  if(result != MONITOR_OK) {
    print_str("sm_enclave_load_page_table FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  page_table_address += PAGE_SIZE;

  print_str("sm_enclave_load_page_table\n");
  result = sm_enclave_load_page_table(enclave_id, page_table_address, 0, 2, NODE_ACL);
  if(result != MONITOR_OK) {
    print_str("sm_enclave_load_page_table FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  page_table_address += PAGE_SIZE;

  print_str("sm_enclave_load_page_table\n");
  result = sm_enclave_load_page_table(enclave_id, page_table_address, 0, 1, NODE_ACL);
  if(result != MONITOR_OK) {
    print_str("sm_enclave_load_page_table FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  uintptr_t phys_addr = page_table_address + PAGE_SIZE;
  uintptr_t os_addr = (uintptr_t) &enclave_start;
  uintptr_t virtual_addr = 0;

  int num_pages_enclave = (((uint64_t) &enclave_end) - ((uint64_t) &enclave_start)) / PAGE_SIZE;

  for(int i = 0; i < num_pages_enclave + 1; i++) {

    print_str("sm_enclave_load_page\n");
    result = sm_enclave_load_page(enclave_id, phys_addr, virtual_addr, os_addr, LEAF_ACL);
    if(result != MONITOR_OK) {
      print_str("sm_enclave_load_page FAILED with error code ");
      print_int(result);
      print_str("\n\n");
      test_completed();
    }

    phys_addr    += PAGE_SIZE;
    os_addr      += PAGE_SIZE;
    virtual_addr += PAGE_SIZE;

  }

  print_str("sm_enclave_metadata_pages\n");
  uint64_t size_enclave_metadata = sm_enclave_metadata_pages(num_mailboxes);

  thread_id_t thread_id = enclave_id + (size_enclave_metadata * PAGE_SIZE);
  uintptr_t stack_ptr = PAGE_SIZE * (num_pages_enclave + 1) ;

  print_str("sm_thread_local\n");
  result = sm_thread_load(enclave_id, thread_id, 0x0, stack_ptr, enclave_handler_address, enclave_handler_stack_pointer);
  if(result != MONITOR_OK) {
    print_str("sm_thread_load FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  print_str("sm_enclave_init\n");
  result = sm_enclave_init(enclave_id);
  if(result != MONITOR_OK) {
    print_str("sm_enclave_init FAILED with error code ");
    print_int(result);
    print_str("\n\n");
    test_completed();
  }

  print_str("sm_enclave_enter\n");
  // Prime
  volatile int a = *(int* ) 0x82004040;   
  result = sm_enclave_enter(enclave_id, thread_id);
  // Probe
  unsigned long begin_cyc;
  asm volatile ("rdcycle %0" : "=r" (begin_cyc));
  volatile int b = *(int* ) 0x82004040;   
  unsigned long end_cyc;
  asm volatile ("rdcycle %0" : "=r" (end_cyc));
  print_int((int) (end_cyc - begin_cyc)); 
  print_str("time\n\n");
  print_int(a);
  print_int(b);
  print_str("Test SUCCESSFUL\n\n");
  test_completed();
}
