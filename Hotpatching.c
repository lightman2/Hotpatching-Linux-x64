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
void new_hello(void)
{
    static int x;
    printf("Hotpatched!, Updation of function on the run. %d\n",x);

}

void hotpatch(void *target, void *replacement)
{

    assert(((uintptr_t)target & 0x07) == 0); // 8-byte aligned?
    void *page = (void *)((uintptr_t)target & ~0xfff);
    mprotect(page, 4096, PROT_WRITE | PROT_EXEC);
    uint32_t rel = (char *)replacement - (char *)target - 5;
    union {
        uint8_t bytes[8];
        uint64_t value;
    } instruction = { {0xe9, rel >> 0, rel >> 8, rel >> 16, rel >> 24} };
    *(uint64_t *)target = instruction.value;
    mprotect(page, 4096, PROT_EXEC);
}



void *worker(void *arg)
{
    (void)arg;
    for (;;) {
        hello();
        usleep(100000);
    }
    return NULL;
}

int main(void)
{
    pthread_t thread;
    pthread_create(&thread, NULL, worker, NULL);
    getchar();
    hotpatch(hello, new_hello);
    pthread_join(thread, NULL);
    return 0;
}

