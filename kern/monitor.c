// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/tsc.h>
#include <kern/pmap.h>
#include <kern/trap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{"hello", "Display HELLO!", mon_hello},
    {"backtrace","Dislpay backtrace", mon_backtrace},
    {"timer_start","call timer_start", mon_timer_start},
    {"timer_stop","call timer_stop", mon_timer_stop},
	{"page","display allocated pages",mon_page}
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/
int
mon_timer_start(int argc, char **argv, struct Trapframe *tf)
{
    timer_start();
    return 0;
}
int
mon_timer_stop(int argc, char **argv, struct Trapframe *tf)
{
    timer_stop();
    return 0;
}
int
mon_page(int argc, char **argv, struct Trapframe *tf)
{
	for (size_t i = 1; i < npages; i++){
		cprintf("%d", i);
		int is_free = (pages[i].pp_link != NULL);
		size_t i_old = i++;
		if (is_free){
			while (i < npages && (pages[i].pp_link) != NULL){
				i++;
			}
		} else{
			while (i < npages && (pages[i].pp_link == NULL)){
				i++;
			}			
		}
		if (i - 1 != i_old){
			cprintf("..%d", i - 1);
		}
		if (is_free){
			cprintf(" FREE\n");
		}
		else{
			cprintf(" ALLOCATED\n");
		}

	}
	return 0;
}
int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", (uint32_t)_start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n",
            (uint32_t)entry, (uint32_t)entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n",
            (uint32_t)etext, (uint32_t)etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n",
            (uint32_t)edata, (uint32_t)edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n",
            (uint32_t)end, (uint32_t)end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
            (uint32_t)ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
    uint32_t *ebp =(uint32_t*)read_ebp();
    while (ebp){
        cprintf("ebp %08x eip %08x args %08x %08x %08x %08x %08x\n",(uint32_t)ebp, ebp[1],
                ebp[2],ebp[3], ebp[4], ebp[5],ebp [6]);
        struct Eipdebuginfo info;
        debuginfo_eip(ebp[1], &info); 
        //cprintf("\t%s:%u: %.*s+%u\n",info.eip_file, info.eip_line,
               // info.eip_fn_namelen, info.eip_fn_name, ebp[1] - info.eip_fn_addr);
                cprintf("\t%.*s:%u: %.*s+%u\n", 32,
            info.eip_file, info.eip_line,
            info.eip_fn_namelen, info.eip_fn_name,
            ebp[1] - info.eip_fn_addr);
        ebp = (uint32_t*)*ebp;
    }
	// Your code here.
	return 0;
}

int
mon_hello(int argc, char **argv, struct Trapframe *tf)
{
	cprintf("HELLO!\n");
	return 0;
}

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
