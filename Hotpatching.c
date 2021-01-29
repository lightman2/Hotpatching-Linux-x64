#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>
__attribute__ ((ms_hook_prologue))
__attribute__ ((aligned(8)))
__attribute__ ((noinline))
__attribute__ ((noclone))

void hello(void)
{
    __asm("");
    puts("I've entered the function.");
}

__attribute__ ((ms_hook_prologue))
__attribute__ ((aligned(8)))
__attribute__ ((noinline))
__attribute__ ((noclone))

static int x;
void new_hello(void)
{
    printf("Hotpatched!, Updation of function on the run. %d\n",x);
/*    int *a ;*/
/*    *a = 100;*/
	fflush(stdout);
}

int isbigendian()
{
	int end_t = 0;
	int i = 0;
	int i_abc = 0x12345678;
	char *uc_abc=(char*)&i_abc ;
	if(*uc_abc == 0x12)
		return 1;
	else
		return 0;
}

int rev_endian32(int *val)
{
    
    printf("%s %0x\n", __func__, *val);
    *val = (*val>>24)| (*val>>8 &0xff00)| (*val&0xff)<<24 | (*val&0xff00)<<8;
    printf("%s %0x\n", __func__, *val);
    return *val;
}


void hotpatch(void *target, void *replacement)
{
/*    asm("ic	ialluis");*/
    int bigendian = 0;
	printf("tar = 0x%x, rep = 0x%x\n", target, replacement);
	printf("tar = 0x%x, rep = 0x%x\n", *(int*)target, *(int*)replacement);
    assert(((uintptr_t)target & 0x07) == 0); // 8-byte aligned?
    void *page = (void *)((uintptr_t)target & ~0xfff);
    mprotect(page, 4096, PROT_WRITE | PROT_EXEC);
    uint32_t rel = ((char *)replacement - (char *)target )/4-1;//减你妹- 4;
	printf("tar = 0x%0x, rep = 0x%0x rel = 0x%0x\n", *(int*)target, *(int*)replacement, rel);
/*    union {*/
/*        uint8_t bytes[8];*/
/*        uint64_t value;*/
/*    }instruction = { {0xe9, rel >> 0, rel >> 8, rel >> 16, rel >> 24} };*/
    union {
        uint8_t bytes[4];
        uint32_t value;
    }instruction = { {0xea,  rel >> 16, rel >> 8, rel >> 0} };
/*    }instruction = { {0xea,  rel >> 16, rel >> 8, rel >> 0} };*/
/*    }instruction = { {0xe9, rel >> 24, rel >> 16, rel >> 8, rel >> 0} };*/
/*    }instruction = { {0xe9, rel >> 0, rel >> 8, rel >> 16, rel >> 24} };*/
    bigendian  = isbigendian(); 
    if(bigendian == 0)
       rev_endian32(&instruction) ;
/*    *(uint64_t *)target = instruction.value;*/
    *(uint32_t *)target = instruction.value;

	printf("tar = 0x%0x, rep = 0x%0x\n", *(int*)target, *(int*)replacement);
    mprotect(page, 4096, PROT_EXEC);
}



void *worker(void *arg)
{
    (void)arg;
    for (;;) {
		printf("%s %d\n", __func__, __LINE__);
        hello();
		fflush(stdout);
        usleep(100000);
    }
    return NULL;
}
/*#define isb() __asm__ __volatile__ ("isb" : : : "memory")*/
/*#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")*/
/*#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")*/
/*#define dmb() __asm__ __volatile__("dmb sy")*/
/*static inline void __DMB(void) { asm volatile ("dmb"); } */
int main(void)
{
    pthread_t thread;
/*    __DMB();*/
/*    dmb();*/
/* Data Memory Barrier */
    getchar();
    pthread_create(&thread, NULL, worker, NULL);
    printf("%x   ; %x\n", hello , new_hello);
    hotpatch(hello, new_hello);
    printf("%x   ; %x\n", *(int*)hello , *(int*)new_hello);

	sleep(10);
	pthread_join(thread, NULL);
    return 0;
}

