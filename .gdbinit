set arch riscv:rv64
#set riscv use_compressed_breakpoint off
layout split
foc cmd
set trace-commands on
set logging on
target remote localhost:1234
#add-symbol-file build/sm.elf 0x80000000
symbol-file build/sm.elf
add-symbol-file build/master_test.elf 0x82000000
add-symbol-file build/sm.enclave.elf  0x86000000
add-symbol-file build/enclave.elf     0x0
