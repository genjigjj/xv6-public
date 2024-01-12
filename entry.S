# The xv6 kernel starts executing in this file. This file is linked with
# the kernel C code, so it can refer to kernel symbols such as main().
# The boot block (bootasm.S and bootmain.c) jumps to entry below.
        
# Multiboot header, for multiboot boot loaders like GNU Grub.
# http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
#
# Using GRUB 2, you can boot xv6 from a file stored in a
# Linux file system by copying kernel or kernelmemfs to /boot
# and then adding this menu entry:
#
# menuentry "xv6" {
# 	insmod ext2
# 	set root='(hd0,msdos1)'
# 	set kernel='/boot/kernel'
# 	echo "Loading ${kernel}..."
# 	multiboot ${kernel} ${kernel}
# 	boot
# }

#include "asm.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"

# Multiboot header.  Data to direct multiboot loader.
.p2align 2
.text
.globl multiboot_header
multiboot_header:
  #define magic 0x1badb002
  #define flags 0
  .long magic
  .long flags
  .long (-magic-flags)

# By convention, the _start symbol specifies the ELF entry point.
# Since we haven't set up virtual memory yet, our entry point is
# the physical address of 'entry'.
.globl _start
_start = V2P_WO(entry)

# Entering xv6 on boot processor, with paging off.
.globl entry
entry:
  # 1、设置页表是扩展分页
  # 2、存储页目录表的物理地址到 cr3 寄存器中
  # 3、设置 cr0 寄存器的 PG 位，开启分页
  # 做完这三件事情后，分页功能启用，以后所有的地址都是线性地址，必须通过 MMU 转换，才能得到物理地址。
  #CR4 寄存器是 x86 架构中的控制寄存器，用于控制处理器的各种特性和功能。其中 PSE 和 PAE 是 CR4 寄存器中的两个标志位，分别代表了页表扩展（Page Size Extension）和物理地址扩展（Physical Address Extension）。
  #PSE（Page Size Extension）：当 PSE 标志位被设置时，表示启用了大页面（大页表）特性。在启用 PSE 后，x86 处理器可以使用大于默认页面大小（4KB）的页面，通常为 4MB 或 2MB，从而提高内存管理的效率。
  #这对于大内存系统来说尤为重要，因为较大的页面可以减少页面表的数量，从而减少了对内存管理单元的访问次数，提高了内存访问的效率。
  #PAE（Physical Address Extension）：当 PAE 标志位被设置时，表示启用了物理地址扩展特性。在启用 PAE 后，x86 处理器可以访问超过 4GB 的物理内存，最多可以达到 64GB 或更多，
  #具体取决于操作系统和硬件的支持情况。PAE 扩展了物理地址的位数，从 32 位扩展到 36 位或 40 位，使得处理器可以寻址更多的物理内存空间。
  # CR4_PSE 页面规模扩展标志PSE。控制寄存器CR4的第4位为页面规模扩展标志，Pentium以上处理器定义该标志。分页时通常以4K字节为一页，分页扩展PSE置位，表示允许将页面规模扩展为4MB或2MB。PSE为0，页面大小为4KB。
  movl    %cr4, %eax
  orl     $(CR4_PSE), %eax
  movl    %eax, %cr4
  # Set page directory
  movl    $(V2P_WO(entrypgdir)), %eax
  movl    %eax, %cr3
  # Turn on paging.
  movl    %cr0, %eax
  orl     $(CR0_PG|CR0_WP), %eax
  movl    %eax, %cr0

  # Set up the stack pointer.
  movl $(stack + KSTACKSIZE), %esp

  # Jump to main(), and switch to executing at
  # high addresses. The indirect call is needed because
  # the assembler produces a PC-relative instruction
  # for a direct jump.
  mov $main, %eax
  jmp *%eax
  # 声明未初始化的数据通用内存区域，处于.bss段。地址为 0x8010a5c0
.comm stack, KSTACKSIZE
