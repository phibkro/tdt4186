// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

uint64 MAX_PAGES = 0;
uint64 FREE_PAGES = 0;

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run
{
    struct run *next;
};

struct
{
    struct spinlock lock;
    struct run *freelist;
} kmem;

// Page Reference Counter
// (max PA - min PA) / PAGE_SIZE = max PAGES
uint8 pgrc[(PHYSTOP - KERNBASE) / PGSIZE];

// Page Reference Count Init
void pgrc_init(void)
{
    memset(pgrc, 0, sizeof(pgrc));
}

// Page Reference Count Get
// Get the reference count for the page associated with the physical address
uint64 pgrc_get(void *pa)
{
    return pgrc[PA2PGRI((uint64)pa)];
}

// Increase the shared memory counter
void pgrc_add(void *pa)
{
    pgrc[PA2PGRI((uint64)pa)]++;
}

// Decrease the shared memory counter
// If the counter is 0 call the free function
void pgrc_remove(void *pa)
{
    if (pgrc_get(pa) > 0)
        pgrc[PA2PGRI((uint64)pa)]--;

    if (pgrc_get(pa) != 0) return;
        page_free(pa);
}

// Frees the page tied to the physical address
void page_free(void *pa)
{
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    struct run *r = (struct run *)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    FREE_PAGES++;
    release(&kmem.lock);
}

void kinit()
{
    initlock(&kmem.lock, "kmem");
    freerange(end, (void *)PHYSTOP);
    pgrc_init();
    MAX_PAGES = FREE_PAGES;
}

void freerange(void *pa_start, void *pa_end)
{
    char *p;
    p = (char *)PGROUNDUP((uint64)pa_start);
    for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    {
        kfree(p);
    }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa)
{
    if (MAX_PAGES != 0)
        assert(FREE_PAGES < MAX_PAGES);

    if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
        panic("kfree");

    pgrc_remove(pa);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
    assert(FREE_PAGES > 0);
    struct run *r;

    acquire(&kmem.lock);
    r = kmem.freelist;
    if (r)
        kmem.freelist = r->next;
    release(&kmem.lock);

    if (r)
    {
        memset((char *)r, 5, PGSIZE); // fill with junk
        pgrc_add((void *)r);
    }
    FREE_PAGES--;
    return (void *)r;
}
