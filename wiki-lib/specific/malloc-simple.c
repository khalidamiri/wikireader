#include <malloc.h>
#include <wikilib.h>
#include <msg.h>
#include <string.h>
#include <stdlib.h>

extern uint8_t __START_heap;
extern uint8_t __END_heap;

/* 512kB RAM size */
#define MEM_SIZE	(&__END_heap - &__START_heap)
#define RAM_START	(&__START_heap)
#define PAGE_SIZE 	(512)
#define MALLOC_DEBUG

/* no user-definable options below this line */
#define N_PAGES		(MEM_SIZE / PAGE_SIZE)
#define MALLOC_START	((N_PAGES * sizeof(uint8_t)) + RAM_START)
#define PAGE(n)		((uint8_t *) (MALLOC_START + (n * PAGE_SIZE)))

#define PAGE_INUSE		(1 << 0)
#define PAGE_DOMAIN_SHIFT	(4)
#define PAGE_DOMAIN_MASK	(0xf)
#define PAGE_DOMAIN(n)		((page_ctrl[n] >> PAGE_DOMAIN_SHIFT) & PAGE_DOMAIN_MASK)

uint8_t *page_ctrl = RAM_START;

void malloc_init(void)
{
	memset(page_ctrl, 0, N_PAGES);
}

static inline int
__check_region(unsigned int start, unsigned int num)
{
	unsigned int i;
	for (i = 0; i < num; i++)
		if (page_ctrl[start + i] & PAGE_INUSE)
			return i;

	return num;
}

static inline void
__use_region(unsigned int start, unsigned int num)
{
	unsigned int i;
	unsigned char domain = 0;

	if (start > 0)
		domain = (page_ctrl[start - 1] >> PAGE_DOMAIN_SHIFT) + 1;

	domain %= PAGE_DOMAIN_MASK;

	if ((start + num < N_PAGES - 1) &&
	    (page_ctrl[start + num + 1] >> PAGE_DOMAIN_SHIFT) == domain)
	    domain++;

	domain %= PAGE_DOMAIN_MASK;

	for (i = 0; i < num; i++)
		page_ctrl[start + i] = (domain << PAGE_DOMAIN_SHIFT) | PAGE_INUSE;
}

#ifdef MALLOC_DEBUG
static inline void __malloc_debug(void)
{
	int i, used = 0;

	for (i = 0; i < N_PAGES; i++)
		if (page_ctrl[i] & PAGE_INUSE)
			used++;

	msg(MSG_INFO, "malloc debug: %d pages free, %d used, %d total\n",
			N_PAGES - used, used, N_PAGES);
}
#else /* MALLOC_DEBUG */
#define __malloc_debug() do {} while (0)
#endif

void *malloc(unsigned int size)
{
	unsigned int start, pages = size / PAGE_SIZE;

	if (size % PAGE_SIZE)
		pages++;

	/* we have no virtual memory mapping on this hardware,
	 * so we need continous memory */
	for (start = 0; start < N_PAGES - pages;) {
		int max = __check_region(start, pages);

		if (max == pages) {
			__use_region(start, pages);
			__malloc_debug();
			msg(MSG_INFO, " --- start = %d, mem = %p\n", start, PAGE(start));
			return PAGE(start);
		}

		if (max == 0)
			start++;
		else
			start += max;
	}

	return NULL;
}

void free(void *ptr)
{
	uint32_t page;
	uint8_t domain;

	if (!ptr || (uint8_t *)ptr < MALLOC_START)
		return;

	page = ((uint8_t *)ptr - MALLOC_START) / PAGE_SIZE;
	if (page > N_PAGES)
		return;

	domain = PAGE_DOMAIN(page);

	while (domain == PAGE_DOMAIN(page))
		page_ctrl[page++] &= ~PAGE_INUSE;

	__malloc_debug();
}

