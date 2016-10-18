#include "g_local.h"

#if id386 > 0

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __linux__
#include <sys/mman.h>
#include <unistd.h>
#include <limits.h>
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
#endif

typedef struct patchBlock_s {
	int	address;
	int size;
	char * orig_data;
	char * new_data;
} patchBlock_t;

static patchBlock_t patches_win32[1] ={
	{	0x0044115F, 1, "\x7e", "\x7c" }
};

static patchBlock_t patches_linux_et[1] ={
	{	0x080f34f0, 1, "\x86", "\x82" }
};

static patchBlock_t patches_linux_etded[1] ={
	{	0x08062f6c, 1, "\x86", "\x82" }
};


static int UnprotectMemory(int address, int size, int * flags) {
#ifdef WIN32
	return VirtualProtect ((LPVOID)address, size, PAGE_EXECUTE_READWRITE, (PDWORD)flags);
#elif defined  __linux__
	int start = address & ~(PAGE_SIZE-1);
	size = (address + size) & ~(PAGE_SIZE-1) - start + PAGE_SIZE;
	return mprotect( (void *)start, size, PROT_READ|PROT_WRITE|PROT_EXEC ) == 0;
#endif
}

static void ProtectMemory(int address, int size, int flags) {
#ifdef WIN32
	int temp;
	VirtualProtect ((LPVOID)address, size, flags, (PDWORD)&temp);
#elif defined  __linux__
	int start = address & ~(PAGE_SIZE-1);
	size = (address + size) & ~(PAGE_SIZE-1) - start + PAGE_SIZE;
	mprotect( (void *)start, size, PROT_READ|PROT_EXEC );
#endif
}

static void HandlePatchBlock(patchBlock_t * patch, int count) {
	int flags;
	G_LogPrintf("Loading Patches\n");
	while (count--) {
		if (!memcmp((char*)patch->address, patch->orig_data, patch->size)) {
			if (UnprotectMemory( patch->address, patch->size, &flags)) {
				memcpy( (char*)patch->address, patch->new_data, patch->size );
				ProtectMemory( patch->address, patch->size, flags );
			}
		}
		patch++;
	}
}

static int findString(const char * test) {
	int i;
	for (i=0x804b000;i<0xb000000;i++) {
	    if (!(i & 1023)) 
	    G_LogPrintf("At %X\n",i);
		if (!strcmp((char*)i, test))
			return i;
	}
	return 0;
}

void G_PatchEngine(void) {
#ifndef idx64
	uintptr_t syscallLocation = G_GetSyscall();
//	G_Printf("Syscall at %X\n",syscallLocation);

#ifdef WIN32
	if (syscallLocation == 0x445DA0) {
		HandlePatchBlock( patches_win32, sizeof(patches_win32)/ sizeof(patchBlock_t));
	}
#elif defined  __linux__
        if (syscallLocation == 0x80f4a80) {
		HandlePatchBlock( patches_linux_et, sizeof(patches_linux_et)/ sizeof(patchBlock_t));
        } else if (syscallLocation == 0x808678c) {
		HandlePatchBlock( patches_linux_etded, sizeof(patches_linux_etded)/ sizeof(patchBlock_t));
	}
#endif
#endif
}

#endif
