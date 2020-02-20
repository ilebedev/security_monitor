.option norvc
.section .text.platform_purge_core

# void platform_purge_core (void); // uarch state only
.globl platform_purge_core
platform_purge_core:
  # Purge code goes here
  #csrwi 0x7c9,1
  ret
