// Local APIC ID（Local Advanced Programmable Interrupt Controller Identifier）是指每个处理器核心（CPU）上的本地APIC（Advanced Programmable Interrupt Controller）
// 的唯一标识符。在多核处理器系统中，每个处理器核心都有一个独有的Local APIC ID。
//Local APIC是一种高级可编程中断控制器，用于处理和分发处理器内部和外部中断。它可以通过一个32位寄存器（Local APIC ID Register）来获取其ID值，该ID值通常由BIOS或操作系统在引导时设置。
// 这个ID值在整个系统中必须是唯一的，并且必须与其他处理器核心的ID值不同。
//使用Local APIC ID可以实现更精细的中断控制和调度，例如将特定的中断信号分配给特定的处理器核心处理，避免中断之间的竞争、提高系统性能等。
// 此外，在NUMA（Non-Uniform Memory Access）系统中，Local APIC ID还用于帮助定位物理内存位置，以实现更高效的内存访问。
//总之，Local APIC ID是处理器核心上本地APIC的唯一标识符，可用于实现更精细的中断控制和调度，以及提高系统性能。

// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler 用于保存调度器的上下文
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
