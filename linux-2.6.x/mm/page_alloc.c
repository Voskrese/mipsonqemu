/*
 *  linux/mm/page_alloc.c
 *
 *  Manages the free list, the system allocates free pages here.
 *  Note that kmalloc() lives in slab.c
 *
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 *  Swap reorganised 29.12.95, Stephen Tweedie
 *  Support of BIGMEM added by Gerhard Wichert, Siemens AG, July 1999
 *  Reshaped it to be a zoned allocator, Ingo Molnar, Red Hat, 1999
 *  Discontiguous memory support, Kanoj Sarcar, SGI, Nov 1999
 *  Zone balancing, Kanoj Sarcar, SGI, Jan 2000
 *  Per cpu hot/cold page lists, bulk allocation, Martin J. Bligh, Sept 2002
 *          (lots of bits borrowed from Ingo Molnar & Andrew Morton)
 */

#include <linux/stddef.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/interrupt.h>
#include <linux/pagemap.h>
#include <linux/bootmem.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/pagevec.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/notifier.h>
#include <linux/topology.h>
#include <linux/sysctl.h>
#include <linux/cpu.h>
#include <linux/cpuset.h>
#include <linux/memory_hotplug.h>
#include <linux/nodemask.h>
#include <linux/vmalloc.h>
#include <linux/mempolicy.h>
#include <linux/stop_machine.h>
#include <linux/sort.h>
#include <linux/pfn.h>
#include <linux/backing-dev.h>

#include <asm/tlbflush.h>
#include <asm/div64.h>
#include "internal.h"

/*
 * MCD - HACK: Find somewhere to initialize this EARLY, or make this
 * initializer cleaner
 */
nodemask_t node_online_map __read_mostly = { { [0] = 1UL } };
EXPORT_SYMBOL(node_online_map);
nodemask_t node_possible_map __read_mostly = NODE_MASK_ALL;
EXPORT_SYMBOL(node_possible_map);
unsigned long totalram_pages __read_mostly;
unsigned long totalreserve_pages __read_mostly;
long nr_swap_pages;
int percpu_pagelist_fraction;

static void __free_pages_ok(struct page *page, unsigned int order);

/*
 * results with 256, 32 in the lowmem_reserve sysctl:
 *	1G machine -> (16M dma, 800M-16M normal, 1G-800M high)
 *	1G machine -> (16M dma, 784M normal, 224M high)
 *	NORMAL allocation will leave 784M/256 of ram reserved in the ZONE_DMA
 *	HIGHMEM allocation will leave 224M/32 of ram reserved in ZONE_NORMAL
 *	HIGHMEM allocation will (224M+784M)/256 of ram reserved in ZONE_DMA
 *
 * TBD: should special case ZONE_DMA32 machines here - in those we normally
 * don't need any ZONE_NORMAL reservation
 */
int sysctl_lowmem_reserve_ratio[MAX_NR_ZONES-1] = {
	 256,
#ifdef CONFIG_ZONE_DMA32
	 256,
#endif
#ifdef CONFIG_HIGHMEM
	 32
#endif
};

EXPORT_SYMBOL(totalram_pages);

/*
 * Used by page_zone() to look up the address of the struct zone whose
 * id is encoded in the upper bits of page->flags
 */
struct zone *zone_table[1 << ZONETABLE_SHIFT] __read_mostly;
EXPORT_SYMBOL(zone_table);

static char *zone_names[MAX_NR_ZONES] = {
	 "DMA",
#ifdef CONFIG_ZONE_DMA32
	 "DMA32",
#endif
	 "Normal",
#ifdef CONFIG_HIGHMEM
	 "HighMem"
#endif
};

int min_free_kbytes = 1024;

unsigned long __meminitdata nr_kernel_pages;
unsigned long __meminitdata nr_all_pages;
static unsigned long __initdata dma_reserve;

#ifdef CONFIG_ARCH_POPULATES_NODE_MAP
  /*
   * MAX_ACTIVE_REGIONS determines the maxmimum number of distinct
   * ranges of memory (RAM) that may be registered with add_active_range().
   * Ranges passed to add_active_range() will be merged if possible
   * so the number of times add_active_range() can be called is
   * related to the number of nodes and the number of holes
   */
  #ifdef CONFIG_MAX_ACTIVE_REGIONS
    /* Allow an architecture to set MAX_ACTIVE_REGIONS to save memory */
    #define MAX_ACTIVE_REGIONS CONFIG_MAX_ACTIVE_REGIONS
  #else
    #if MAX_NUMNODES >= 32
      /* If there can be many nodes, allow up to 50 holes per node */
      #define MAX_ACTIVE_REGIONS (MAX_NUMNODES*50)
    #else
      /* By default, allow up to 256 distinct regions */
      #define MAX_ACTIVE_REGIONS 256
    #endif
  #endif

  struct node_active_region __initdata early_node_map[MAX_ACTIVE_REGIONS];
  int __initdata nr_nodemap_entries;
  unsigned long __initdata arch_zone_lowest_possible_pfn[MAX_NR_ZONES];
  unsigned long __initdata arch_zone_highest_possible_pfn[MAX_NR_ZONES];
#ifdef CONFIG_MEMORY_HOTPLUG_RESERVE
  unsigned long __initdata node_boundary_start_pfn[MAX_NUMNODES];
  unsigned long __initdata node_boundary_end_pfn[MAX_NUMNODES];
#endif /* CONFIG_MEMORY_HOTPLUG_RESERVE */
#endif /* CONFIG_ARCH_POPULATES_NODE_MAP */

#ifdef CONFIG_DEBUG_VM
static int page_outside_zone_boundaries(struct zone *zone, struct page *page)
{
	int ret = 0;
	unsigned seq;
	unsigned long pfn = page_to_pfn(page);

	do {
		seq = zone_span_seqbegin(zone);
		if (pfn >= zone->zone_start_pfn + zone->spanned_pages)
			ret = 1;
		else if (pfn < zone->zone_start_pfn)
			ret = 1;
	} while (zone_span_seqretry(zone, seq));

	return ret;
}

static int page_is_consistent(struct zone *zone, struct page *page)
{
#ifdef CONFIG_HOLES_IN_ZONE
	if (!pfn_valid(page_to_pfn(page)))
		return 0;
#endif
	if (zone != page_zone(page))
		return 0;

	return 1;
}
/*
 * Temporary debugging check for pages not lying within a given zone.
 */
static int bad_range(struct zone *zone, struct page *page)
{
	if (page_outside_zone_boundaries(zone, page))
		return 1;
	if (!page_is_consistent(zone, page))
		return 1;

	return 0;
}
#else
static inline int bad_range(struct zone *zone, struct page *page)
{
	return 0;
}
#endif

static void bad_page(struct page *page)
{
	printk(KERN_EMERG "Bad page state in process '%s'\n"
		KERN_EMERG "page:%p flags:0x%0*lx mapping:%p mapcount:%d count:%d\n"
		KERN_EMERG "Trying to fix it up, but a reboot is needed\n"
		KERN_EMERG "Backtrace:\n",
		current->comm, page, (int)(2*sizeof(unsigned long)),
		(unsigned long)page->flags, page->mapping,
		page_mapcount(page), page_count(page));
	dump_stack();
	page->flags &= ~(1 << PG_lru	|
			1 << PG_private |
			1 << PG_locked	|
			1 << PG_active	|
			1 << PG_dirty	|
			1 << PG_reclaim |
			1 << PG_slab    |
			1 << PG_swapcache |
			1 << PG_writeback |
			1 << PG_buddy );
	set_page_count(page, 0);
	reset_page_mapcount(page);
	page->mapping = NULL;
	add_taint(TAINT_BAD_PAGE);
}

/*
 * Higher-order pages are called "compound pages".  They are structured thusly:
 *
 * The first PAGE_SIZE page is called the "head page".
 *
 * The remaining PAGE_SIZE pages are called "tail pages".
 *
 * All pages have PG_compound set.  All pages have their ->private pointing at
 * the head page (even the head page has this).
 *
 * The first tail page's ->lru.next holds the address of the compound page's
 * put_page() function.  Its ->lru.prev holds the order of allocation.
 * This usage means that zero-order pages may not be compound.
 */

static void free_compound_page(struct page *page)
{
	__free_pages_ok(page, (unsigned long)page[1].lru.prev);
}

static void prep_compound_page(struct page *page, unsigned long order)
{
	int i;
	int nr_pages = 1 << order;

	page[1].lru.next = (void *)free_compound_page;	/* set dtor */
	page[1].lru.prev = (void *)order;
	for (i = 0; i < nr_pages; i++) {
		struct page *p = page + i;

		__SetPageCompound(p);
		set_page_private(p, (unsigned long)page);
	}
}

static void destroy_compound_page(struct page *page, unsigned long order)
{
	int i;
	int nr_pages = 1 << order;

	if (unlikely((unsigned long)page[1].lru.prev != order))
		bad_page(page);

	for (i = 0; i < nr_pages; i++) {
		struct page *p = page + i;

		if (unlikely(!PageCompound(p) |
				(page_private(p) != (unsigned long)page)))
			bad_page(page);
		__ClearPageCompound(p);
	}
}

static inline void prep_zero_page(struct page *page, int order, gfp_t gfp_flags)
{
	int i;

	VM_BUG_ON((gfp_flags & (__GFP_WAIT | __GFP_HIGHMEM)) == __GFP_HIGHMEM);
	/*
	 * clear_highpage() will use KM_USER0, so it's a bug to use __GFP_ZERO
	 * and __GFP_HIGHMEM from hard or soft interrupt context.
	 */
	VM_BUG_ON((gfp_flags & __GFP_HIGHMEM) && in_interrupt());
	for (i = 0; i < (1 << order); i++)
		clear_highpage(page + i);
}

/*
 * function for dealing with page's order in buddy system.
 * zone->lock is already acquired when we use these.
 * So, we don't need atomic page->flags operations here.
 */
static inline unsigned long page_order(struct page *page)
{
	return page_private(page);
}

static inline void set_page_order(struct page *page, int order)
{
	set_page_private(page, order);
	__SetPageBuddy(page);
}

static inline void rmv_page_order(struct page *page)
{
	__ClearPageBuddy(page);
	set_page_private(page, 0);
}

/*
 * Locate the struct page for both the matching buddy in our
 * pair (buddy1) and the combined O(n+1) page they form (page).
 *
 * 1) Any buddy B1 will have an order O twin B2 which satisfies
 * the following equation:
 *     B2 = B1 ^ (1 << O)
 * For example, if the starting buddy (buddy2) is #8 its order
 * 1 buddy is #10:
 *     B2 = 8 ^ (1 << 1) = 8 ^ 2 = 10
 *
 * 2) Any buddy B will have an order O+1 parent P which
 * satisfies the following equation:
 *     P = B & ~(1 << O)
 *
 * Assumption: *_mem_map is contiguous at least up to MAX_ORDER
 */
static inline struct page *
__page_find_buddy(struct page *page, unsigned long page_idx, unsigned int order)
{
	unsigned long buddy_idx = page_idx ^ (1 << order);

	return page + (buddy_idx - page_idx);
}

static inline unsigned long
__find_combined_index(unsigned long page_idx, unsigned int order)
{
	return (page_idx & ~(1 << order));
}

/*
 * This function checks whether a page is free && is the buddy
 * we can do coalesce a page and its buddy if
 * (a) the buddy is not in a hole &&
 * (b) the buddy is in the buddy system &&
 * (c) a page and its buddy have the same order &&
 * (d) a page and its buddy are in the same zone.
 *
 * For recording whether a page is in the buddy system, we use PG_buddy.
 * Setting, clearing, and testing PG_buddy is serialized by zone->lock.
 *
 * For recording page's order, we use page_private(page).
 */
static inline int page_is_buddy(struct page *page, struct page *buddy,
								int order)
{
#ifdef CONFIG_HOLES_IN_ZONE
	if (!pfn_valid(page_to_pfn(buddy)))
		return 0;
#endif

	if (page_zone_id(page) != page_zone_id(buddy))
		return 0;

	if (PageBuddy(buddy) && page_order(buddy) == order) {
		BUG_ON(page_count(buddy) != 0);
		return 1;
	}
	return 0;
}

/*
 * Freeing function for a buddy system allocator.
 *
 * The concept of a buddy system is to maintain direct-mapped table
 * (containing bit values) for memory blocks of various "orders".
 * The bottom level table contains the map for the smallest allocatable
 * units of memory (here, pages), and each level above it describes
 * pairs of units from the levels below, hence, "buddies".
 * At a high level, all that happens here is marking the table entry
 * at the bottom level available, and propagating the changes upward
 * as necessary, plus some accounting needed to play nicely with other
 * parts of the VM system.
 * At each level, we keep a list of pages, which are heads of continuous
 * free pages of length of (1 << order) and marked with PG_buddy. Page's
 * order is recorded in page_private(page) field.
 * So when we are allocating or freeing one, we can derive the state of the
 * other.  That is, if we allocate a small block, and both were   
 * free, the remainder of the region must be split into blocks.   
 * If a block is freed, and its buddy is also free, then this
 * triggers coalescing into a block of larger size.            
 *
 * -- wli
 */

static inline void __free_one_page(struct page *page,
		struct zone *zone, unsigned int order)
{
	unsigned long page_idx;
	int order_size = 1 << order;

	if (unlikely(PageCompound(page)))
		destroy_compound_page(page, order);

	page_idx = page_to_pfn(page) & ((1 << MAX_ORDER) - 1);

	VM_BUG_ON(page_idx & (order_size - 1));
	VM_BUG_ON(bad_range(zone, page));

	zone->free_pages += order_size;
	while (order < MAX_ORDER-1) {
		unsigned long combined_idx;
		struct free_area *area;
		struct page *buddy;

		buddy = __page_find_buddy(page, page_idx, order);
		if (!page_is_buddy(page, buddy, order))
			break;		/* Move the buddy up one level. */

		list_del(&buddy->lru);
		area = zone->free_area + order;
		area->nr_free--;
		rmv_page_order(buddy);
		combined_idx = __find_combined_index(page_idx, order);
		page = page + (combined_idx - page_idx);
		page_idx = combined_idx;
		order++;
	}
	set_page_order(page, order);
	list_add(&page->lru, &zone->free_area[order].free_list);
	zone->free_area[order].nr_free++;
}

static inline int free_pages_check(struct page *page)
{
	if (unlikely(page_mapcount(page) |
		(page->mapping != NULL)  |
		(page_count(page) != 0)  |
		(page->flags & (
			1 << PG_lru	|
			1 << PG_private |
			1 << PG_locked	|
			1 << PG_active	|
			1 << PG_reclaim	|
			1 << PG_slab	|
			1 << PG_swapcache |
			1 << PG_writeback |
			1 << PG_reserved |
			1 << PG_buddy ))))
		bad_page(page);
	if (PageDirty(page))
		__ClearPageDirty(page);
	/*
	 * For now, we report if PG_reserved was found set, but do not
	 * clear it, and do not free the page.  But we shall soon need
	 * to do more, for when the ZERO_PAGE count wraps negative.
	 */
	return PageReserved(page);
}

/*
 * Frees a list of pages. 
 * Assumes all pages on list are in same zone, and of same order.
 * count is the number of pages to free.
 *
 * If the zone was previously in an "all pages pinned" state then look to
 * see if this freeing clears that state.
 *
 * And clear the zone's pages_scanned counter, to hold off the "all pages are
 * pinned" detection logic.
 */
static void free_pages_bulk(struct zone *zone, int count,
					struct list_head *list, int order)
{
	spin_lock(&zone->lock);
	zone->all_unreclaimable = 0;
	zone->pages_scanned = 0;
	while (count--) {
		struct page *page;

		VM_BUG_ON(list_empty(list));
		page = list_entry(list->prev, struct page, lru);
		/* have to delete it as __free_one_page list manipulates */
		list_del(&page->lru);
		__free_one_page(page, zone, order);
	}
	spin_unlock(&zone->lock);
}

static void free_one_page(struct zone *zone, struct page *page, int order)
{
	spin_lock(&zone->lock);
	zone->all_unreclaimable = 0;
	zone->pages_scanned = 0;
	__free_one_page(page, zone ,order);
	spin_unlock(&zone->lock);
}

static void __free_pages_ok(struct page *page, unsigned int order)
{
	unsigned long flags;
	int i;
	int reserved = 0;

	for (i = 0 ; i < (1 << order) ; ++i)
		reserved += free_pages_check(page + i);
	if (reserved)
		return;

	if (!PageHighMem(page))
		debug_check_no_locks_freed(page_address(page),PAGE_SIZE<<order);
	arch_free_page(page, order);
	kernel_map_pages(page, 1 << order, 0);

	local_irq_save(flags);
	__count_vm_events(PGFREE, 1 << order);
	free_one_page(page_zone(page), page, order);
	local_irq_restore(flags);
}

/*
 * permit the bootmem allocator to evade page validation on high-order frees
 */
void fastcall __init __free_pages_bootmem(struct page *page, unsigned int order)
{
	if (order == 0) {
		__ClearPageReserved(page);
		set_page_count(page, 0);
		set_page_refcounted(page);
		__free_page(page);
	} else {
		int loop;

		prefetchw(page);
		for (loop = 0; loop < BITS_PER_LONG; loop++) {
			struct page *p = &page[loop];

			if (loop + 1 < BITS_PER_LONG)
				prefetchw(p + 1);
			__ClearPageReserved(p);
			set_page_count(p, 0);
		}

		set_page_refcounted(page);
		__free_pages(page, order);
	}
}


/*
 * The order of subdivision here is critical for the IO subsystem.
 * Please do not alter this order without good reasons and regression
 * testing. Specifically, as large blocks of memory are subdivided,
 * the order in which smaller blocks are delivered depends on the order
 * they're subdivided in this function. This is the primary factor
 * influencing the order in which pages are delivered to the IO
 * subsystem according to empirical testing, and this is also justified
 * by considering the behavior of a buddy system containing a single
 * large block of memory acted on by a series of small allocations.
 * This behavior is a critical factor in sglist merging's success.
 *
 * -- wli
 */
static inline void expand(struct zone *zone, struct page *page,
 	int low, int high, struct free_area *area)
{
	unsigned long size = 1 << high;

	while (high > low) {
		area--;
		high--;
		size >>= 1;
		VM_BUG_ON(bad_range(zone, &page[size]));
		list_add(&page[size].lru, &area->free_list);
		area->nr_free++;
		set_page_order(&page[size], high);
	}
}

/*
 * This page is about to be returned from the page allocator
 */
static int prep_new_page(struct page *page, int order, gfp_t gfp_flags)
{
	if (unlikely(page_mapcount(page) |
		(page->mapping != NULL)  |
		(page_count(page) != 0)  |
		(page->flags & (
			1 << PG_lru	|
			1 << PG_private	|
			1 << PG_locked	|
			1 << PG_active	|
			1 << PG_dirty	|
			1 << PG_reclaim	|
			1 << PG_slab    |
			1 << PG_swapcache |
			1 << PG_writeback |
			1 << PG_reserved |
			1 << PG_buddy ))))
		bad_page(page);

	/*
	 * For now, we report if PG_reserved was found set, but do not
	 * clear it, and do not allocate the page: as a safety net.
	 */
	if (PageReserved(page))
		return 1;

	page->flags &= ~(1 << PG_uptodate | 1 << PG_error |
			1 << PG_referenced | 1 << PG_arch_1 |
			1 << PG_checked | 1 << PG_mappedtodisk);
	set_page_private(page, 0);
	set_page_refcounted(page);
	kernel_map_pages(page, 1 << order, 1);

	if (gfp_flags & __GFP_ZERO)
		prep_zero_page(page, order, gfp_flags);

	if (order && (gfp_flags & __GFP_COMP))
		prep_compound_page(page, order);

	return 0;
}

/* 
 * Do the hard work of removing an element from the buddy allocator.
 * Call me with the zone->lock already held.
 */
static struct page *__rmqueue(struct zone *zone, unsigned int order)
{
	struct free_area * area;
	unsigned int current_order;
	struct page *page;

	for (current_order = order; current_order < MAX_ORDER; ++current_order) {
		area = zone->free_area + current_order;
		if (list_empty(&area->free_list))
			continue;

		page = list_entry(area->free_list.next, struct page, lru);
		list_del(&page->lru);
		rmv_page_order(page);
		area->nr_free--;
		zone->free_pages -= 1UL << order;
		expand(zone, page, order, current_order, area);
		return page;
	}

	return NULL;
}

/* 
 * Obtain a specified number of elements from the buddy allocator, all under
 * a single hold of the lock, for efficiency.  Add them to the supplied list.
 * Returns the number of new pages which were placed at *list.
 */
static int rmqueue_bulk(struct zone *zone, unsigned int order, 
			unsigned long count, struct list_head *list)
{
	int i;
	
	spin_lock(&zone->lock);
	for (i = 0; i < count; ++i) {
		struct page *page = __rmqueue(zone, order);
		if (unlikely(page == NULL))
			break;
		list_add_tail(&page->lru, list);
	}
	spin_unlock(&zone->lock);
	return i;
}

#ifdef CONFIG_NUMA
/*
 * Called from the slab reaper to drain pagesets on a particular node that
 * belongs to the currently executing processor.
 * Note that this function must be called with the thread pinned to
 * a single processor.
 */
void drain_node_pages(int nodeid)
{
	int i;
	enum zone_type z;
	unsigned long flags;

	for (z = 0; z < MAX_NR_ZONES; z++) {
		struct zone *zone = NODE_DATA(nodeid)->node_zones + z;
		struct per_cpu_pageset *pset;

		if (!populated_zone(zone))
			continue;

		pset = zone_pcp(zone, smp_processor_id());
		for (i = 0; i < ARRAY_SIZE(pset->pcp); i++) {
			struct per_cpu_pages *pcp;

			pcp = &pset->pcp[i];
			if (pcp->count) {
				local_irq_save(flags);
				free_pages_bulk(zone, pcp->count, &pcp->list, 0);
				pcp->count = 0;
				local_irq_restore(flags);
			}
		}
	}
}
#endif

#if defined(CONFIG_PM) || defined(CONFIG_HOTPLUG_CPU)
static void __drain_pages(unsigned int cpu)
{
	unsigned long flags;
	struct zone *zone;
	int i;

	for_each_zone(zone) {
		struct per_cpu_pageset *pset;

		pset = zone_pcp(zone, cpu);
		for (i = 0; i < ARRAY_SIZE(pset->pcp); i++) {
			struct per_cpu_pages *pcp;

			pcp = &pset->pcp[i];
			local_irq_save(flags);
			free_pages_bulk(zone, pcp->count, &pcp->list, 0);
			pcp->count = 0;
			local_irq_restore(flags);
		}
	}
}
#endif /* CONFIG_PM || CONFIG_HOTPLUG_CPU */

#ifdef CONFIG_PM

void mark_free_pages(struct zone *zone)
{
	unsigned long pfn, max_zone_pfn;
	unsigned long flags;
	int order;
	struct list_head *curr;

	if (!zone->spanned_pages)
		return;

	spin_lock_irqsave(&zone->lock, flags);

	max_zone_pfn = zone->zone_start_pfn + zone->spanned_pages;
	for (pfn = zone->zone_start_pfn; pfn < max_zone_pfn; pfn++)
		if (pfn_valid(pfn)) {
			struct page *page = pfn_to_page(pfn);

			if (!PageNosave(page))
				ClearPageNosaveFree(page);
		}

	for (order = MAX_ORDER - 1; order >= 0; --order)
		list_for_each(curr, &zone->free_area[order].free_list) {
			unsigned long i;

			pfn = page_to_pfn(list_entry(curr, struct page, lru));
			for (i = 0; i < (1UL << order); i++)
				SetPageNosaveFree(pfn_to_page(pfn + i));
		}

	spin_unlock_irqrestore(&zone->lock, flags);
}

/*
 * Spill all of this CPU's per-cpu pages back into the buddy allocator.
 */
void drain_local_pages(void)
{
	unsigned long flags;

	local_irq_save(flags);	
	__drain_pages(smp_processor_id());
	local_irq_restore(flags);	
}
#endif /* CONFIG_PM */

/*
 * Free a 0-order page
 */
static void fastcall free_hot_cold_page(struct page *page, int cold)
{
	struct zone *zone = page_zone(page);
	struct per_cpu_pages *pcp;
	unsigned long flags;

	if (PageAnon(page))
		page->mapping = NULL;
	if (free_pages_check(page))
		return;

	if (!PageHighMem(page))
		debug_check_no_locks_freed(page_address(page), PAGE_SIZE);
	arch_free_page(page, 0);
	kernel_map_pages(page, 1, 0);

	pcp = &zone_pcp(zone, get_cpu())->pcp[cold];
	local_irq_save(flags);
	__count_vm_event(PGFREE);
	list_add(&page->lru, &pcp->list);
	pcp->count++;
	if (pcp->count >= pcp->high) {
		free_pages_bulk(zone, pcp->batch, &pcp->list, 0);
		pcp->count -= pcp->batch;
	}
	local_irq_restore(flags);
	put_cpu();
}

void fastcall free_hot_page(struct page *page)
{
	free_hot_cold_page(page, 0);
}
	
void fastcall free_cold_page(struct page *page)
{
	free_hot_cold_page(page, 1);
}

/*
 * split_page takes a non-compound higher-order page, and splits it into
 * n (1<<order) sub-pages: page[0..n]
 * Each sub-page must be freed individually.
 *
 * Note: this is probably too low level an operation for use in drivers.
 * Please consult with lkml before using this in your driver.
 */
void split_page(struct page *page, unsigned int order)
{
	int i;

	VM_BUG_ON(PageCompound(page));
	VM_BUG_ON(!page_count(page));
	for (i = 1; i < (1 << order); i++)
		set_page_refcounted(page + i);
}

/*
 * Really, prep_compound_page() should be called from __rmqueue_bulk().  But
 * we cheat by calling it from here, in the order > 0 path.  Saves a branch
 * or two.
 */
static struct page *buffered_rmqueue(struct zonelist *zonelist,
			struct zone *zone, int order, gfp_t gfp_flags)
{
	unsigned long flags;
	struct page *page;
	int cold = !!(gfp_flags & __GFP_COLD);
	int cpu;

again:
	cpu  = get_cpu();
	if (likely(order == 0)) {
		struct per_cpu_pages *pcp;

		pcp = &zone_pcp(zone, cpu)->pcp[cold];
		local_irq_save(flags);
		if (!pcp->count) {
			pcp->count = rmqueue_bulk(zone, 0,
						pcp->batch, &pcp->list);
			if (unlikely(!pcp->count))
				goto failed;
		}
		page = list_entry(pcp->list.next, struct page, lru);
		list_del(&page->lru);
		pcp->count--;
	} else {
		spin_lock_irqsave(&zone->lock, flags);
		page = __rmqueue(zone, order);
		spin_unlock(&zone->lock);
		if (!page)
			goto failed;
	}

	__count_zone_vm_events(PGALLOC, zone, 1 << order);
	zone_statistics(zonelist, zone);
	local_irq_restore(flags);
	put_cpu();

	VM_BUG_ON(bad_range(zone, page));
	if (prep_new_page(page, order, gfp_flags))
		goto again;
	return page;

failed:
	local_irq_restore(flags);
	put_cpu();
	return NULL;
}

#define ALLOC_NO_WATERMARKS	0x01 /* don't check watermarks at all */
#define ALLOC_WMARK_MIN		0x02 /* use pages_min watermark */
#define ALLOC_WMARK_LOW		0x04 /* use pages_low watermark */
#define ALLOC_WMARK_HIGH	0x08 /* use pages_high watermark */
#define ALLOC_HARDER		0x10 /* try to alloc harder */
#define ALLOC_HIGH		0x20 /* __GFP_HIGH set */
#define ALLOC_CPUSET		0x40 /* check for correct cpuset */

/*
 * Return 1 if free pages are above 'mark'. This takes into account the order
 * of the allocation.
 */
int zone_watermark_ok(struct zone *z, int order, unsigned long mark,
		      int classzone_idx, int alloc_flags)
{
	/* free_pages my go negative - that's OK */
	unsigned long min = mark;
	long free_pages = z->free_pages - (1 << order) + 1;
	int o;

	if (alloc_flags & ALLOC_HIGH)
		min -= min / 2;
	if (alloc_flags & ALLOC_HARDER)
		min -= min / 4;

	if (free_pages <= min + z->lowmem_reserve[classzone_idx])
		return 0;
	for (o = 0; o < order; o++) {
		/* At the next order, this order's pages become unavailable */
		free_pages -= z->free_area[o].nr_free << o;

		/* Require fewer higher order pages to be free */
		min >>= 1;

		if (free_pages <= min)
			return 0;
	}
	return 1;
}

/*
 * get_page_from_freeliest goes through the zonelist trying to allocate
 * a page.
 */
static struct page *
get_page_from_freelist(gfp_t gfp_mask, unsigned int order,
		struct zonelist *zonelist, int alloc_flags)
{
	struct zone **z = zonelist->zones;
	struct page *page = NULL;
	int classzone_idx = zone_idx(*z);
	struct zone *zone;

	/*
	 * Go through the zonelist once, looking for a zone with enough free.
	 * See also cpuset_zone_allowed() comment in kernel/cpuset.c.
	 */
	do {
		zone = *z;
		if (unlikely(NUMA_BUILD && (gfp_mask & __GFP_THISNODE) &&
			zone->zone_pgdat != zonelist->zones[0]->zone_pgdat))
				break;
		if ((alloc_flags & ALLOC_CPUSET) &&
				!cpuset_zone_allowed(zone, gfp_mask))
			continue;

		if (!(alloc_flags & ALLOC_NO_WATERMARKS)) {
			unsigned long mark;
			if (alloc_flags & ALLOC_WMARK_MIN)
				mark = zone->pages_min;
			else if (alloc_flags & ALLOC_WMARK_LOW)
				mark = zone->pages_low;
			else
				mark = zone->pages_high;
			if (!zone_watermark_ok(zone , order, mark,
				    classzone_idx, alloc_flags))
				if (!zone_reclaim_mode ||
				    !zone_reclaim(zone, gfp_mask, order))
					continue;
		}

		page = buffered_rmqueue(zonelist, zone, order, gfp_mask);
		if (page) {
			break;
		}
	} while (*(++z) != NULL);
	return page;
}

/*
 * This is the 'heart' of the zoned buddy allocator.
 */
struct page * fastcall
__alloc_pages(gfp_t gfp_mask, unsigned int order,
		struct zonelist *zonelist)
{
	const gfp_t wait = gfp_mask & __GFP_WAIT;
	struct zone **z;
	struct page *page;
	struct reclaim_state reclaim_state;
	struct task_struct *p = current;
	int do_retry;
	int alloc_flags;
	int did_some_progress;

	might_sleep_if(wait);

restart:
	z = zonelist->zones;  /* the list of zones suitable for gfp_mask */

	if (unlikely(*z == NULL)) {
		/* Should this ever happen?? */
		return NULL;
	}

	page = get_page_from_freelist(gfp_mask|__GFP_HARDWALL, order,
				zonelist, ALLOC_WMARK_LOW|ALLOC_CPUSET);
	if (page)
		goto got_pg;

	do {
		wakeup_kswapd(*z, order);
	} while (*(++z));

	/*
	 * OK, we're below the kswapd watermark and have kicked background
	 * reclaim. Now things get more complex, so set up alloc_flags according
	 * to how we want to proceed.
	 *
	 * The caller may dip into page reserves a bit more if the caller
	 * cannot run direct reclaim, or if the caller has realtime scheduling
	 * policy or is asking for __GFP_HIGH memory.  GFP_ATOMIC requests will
	 * set both ALLOC_HARDER (!wait) and ALLOC_HIGH (__GFP_HIGH).
	 */
	alloc_flags = ALLOC_WMARK_MIN;
	if ((unlikely(rt_task(p)) && !in_interrupt()) || !wait)
		alloc_flags |= ALLOC_HARDER;
	if (gfp_mask & __GFP_HIGH)
		alloc_flags |= ALLOC_HIGH;
	if (wait)
		alloc_flags |= ALLOC_CPUSET;

	/*
	 * Go through the zonelist again. Let __GFP_HIGH and allocations
	 * coming from realtime tasks go deeper into reserves.
	 *
	 * This is the last chance, in general, before the goto nopage.
	 * Ignore cpuset if GFP_ATOMIC (!wait) rather than fail alloc.
	 * See also cpuset_zone_allowed() comment in kernel/cpuset.c.
	 */
	page = get_page_from_freelist(gfp_mask, order, zonelist, alloc_flags);
	if (page)
		goto got_pg;

	/* This allocation should allow future memory freeing. */

	if (((p->flags & PF_MEMALLOC) || unlikely(test_thread_flag(TIF_MEMDIE)))
			&& !in_interrupt()) {
		if (!(gfp_mask & __GFP_NOMEMALLOC)) {
nofail_alloc:
			/* go through the zonelist yet again, ignoring mins */
			page = get_page_from_freelist(gfp_mask, order,
				zonelist, ALLOC_NO_WATERMARKS);
			if (page)
				goto got_pg;
			if (gfp_mask & __GFP_NOFAIL) {
				congestion_wait(WRITE, HZ/50);
				goto nofail_alloc;
			}
		}
		goto nopage;
	}

	/* Atomic allocations - we can't balance anything */
	if (!wait)
		goto nopage;

rebalance:
	cond_resched();

	/* We now go into synchronous reclaim */
	cpuset_memory_pressure_bump();
	p->flags |= PF_MEMALLOC;
	reclaim_state.reclaimed_slab = 0;
	p->reclaim_state = &reclaim_state;

	did_some_progress = try_to_free_pages(zonelist->zones, gfp_mask);

	p->reclaim_state = NULL;
	p->flags &= ~PF_MEMALLOC;

	cond_resched();

	if (likely(did_some_progress)) {
		page = get_page_from_freelist(gfp_mask, order,
						zonelist, alloc_flags);
		if (page)
			goto got_pg;
	} else if ((gfp_mask & __GFP_FS) && !(gfp_mask & __GFP_NORETRY)) {
		/*
		 * Go through the zonelist yet one more time, keep
		 * very high watermark here, this is only to catch
		 * a parallel oom killing, we must fail if we're still
		 * under heavy pressure.
		 */
		page = get_page_from_freelist(gfp_mask|__GFP_HARDWALL, order,
				zonelist, ALLOC_WMARK_HIGH|ALLOC_CPUSET);
		if (page)
			goto got_pg;

		out_of_memory(zonelist, gfp_mask, order);
		goto restart;
	}

	/*
	 * Don't let big-order allocations loop unless the caller explicitly
	 * requests that.  Wait for some write requests to complete then retry.
	 *
	 * In this implementation, __GFP_REPEAT means __GFP_NOFAIL for order
	 * <= 3, but that may not be true in other implementations.
	 */
	do_retry = 0;
	if (!(gfp_mask & __GFP_NORETRY)) {
		if ((order <= 3) || (gfp_mask & __GFP_REPEAT))
			do_retry = 1;
		if (gfp_mask & __GFP_NOFAIL)
			do_retry = 1;
	}
	if (do_retry) {
		congestion_wait(WRITE, HZ/50);
		goto rebalance;
	}

nopage:
	if (!(gfp_mask & __GFP_NOWARN) && printk_ratelimit()) {
		printk(KERN_WARNING "%s: page allocation failure."
			" order:%d, mode:0x%x\n",
			p->comm, order, gfp_mask);
		dump_stack();
		show_mem();
	}
got_pg:
	return page;
}

EXPORT_SYMBOL(__alloc_pages);

/*
 * Common helper functions.
 */
fastcall unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
	struct page * page;
	page = alloc_pages(gfp_mask, order);
	if (!page)
		return 0;
	return (unsigned long) page_address(page);
}

EXPORT_SYMBOL(__get_free_pages);

fastcall unsigned long get_zeroed_page(gfp_t gfp_mask)
{
	struct page * page;

	/*
	 * get_zeroed_page() returns a 32-bit address, which cannot represent
	 * a highmem page
	 */
	VM_BUG_ON((gfp_mask & __GFP_HIGHMEM) != 0);

	page = alloc_pages(gfp_mask | __GFP_ZERO, 0);
	if (page)
		return (unsigned long) page_address(page);
	return 0;
}

EXPORT_SYMBOL(get_zeroed_page);

void __pagevec_free(struct pagevec *pvec)
{
	int i = pagevec_count(pvec);

	while (--i >= 0)
		free_hot_cold_page(pvec->pages[i], pvec->cold);
}

fastcall void __free_pages(struct page *page, unsigned int order)
{
	if (put_page_testzero(page)) {
		if (order == 0)
			free_hot_page(page);
		else
			__free_pages_ok(page, order);
	}
}

EXPORT_SYMBOL(__free_pages);

fastcall void free_pages(unsigned long addr, unsigned int order)
{
	if (addr != 0) {
		VM_BUG_ON(!virt_addr_valid((void *)addr));
		__free_pages(virt_to_page((void *)addr), order);
	}
}

EXPORT_SYMBOL(free_pages);

/*
 * Total amount of free (allocatable) RAM:
 */
unsigned int nr_free_pages(void)
{
	unsigned int sum = 0;
	struct zone *zone;

	for_each_zone(zone)
		sum += zone->free_pages;

	return sum;
}

EXPORT_SYMBOL(nr_free_pages);

#ifdef CONFIG_NUMA
unsigned int nr_free_pages_pgdat(pg_data_t *pgdat)
{
	unsigned int sum = 0;
	enum zone_type i;

	for (i = 0; i < MAX_NR_ZONES; i++)
		sum += pgdat->node_zones[i].free_pages;

	return sum;
}
#endif

static unsigned int nr_free_zone_pages(int offset)
{
	/* Just pick one node, since fallback list is circular */
	pg_data_t *pgdat = NODE_DATA(numa_node_id());
	unsigned int sum = 0;

	struct zonelist *zonelist = pgdat->node_zonelists + offset;
	struct zone **zonep = zonelist->zones;
	struct zone *zone;

	for (zone = *zonep++; zone; zone = *zonep++) {
		unsigned long size = zone->present_pages;
		unsigned long high = zone->pages_high;
		if (size > high)
			sum += size - high;
	}

	return sum;
}

/*
 * Amount of free RAM allocatable within ZONE_DMA and ZONE_NORMAL
 */
unsigned int nr_free_buffer_pages(void)
{
	return nr_free_zone_pages(gfp_zone(GFP_USER));
}

/*
 * Amount of free RAM allocatable within all zones
 */
unsigned int nr_free_pagecache_pages(void)
{
	return nr_free_zone_pages(gfp_zone(GFP_HIGHUSER));
}

static inline void show_node(struct zone *zone)
{
	if (NUMA_BUILD)
		printk("Node %ld ", zone_to_nid(zone));
}

void si_meminfo(struct sysinfo *val)
{
	val->totalram = totalram_pages;
	val->sharedram = 0;
	val->freeram = nr_free_pages();
	val->bufferram = nr_blockdev_pages();
	val->totalhigh = totalhigh_pages;
	val->freehigh = nr_free_highpages();
	val->mem_unit = PAGE_SIZE;
}

EXPORT_SYMBOL(si_meminfo);

#ifdef CONFIG_NUMA
void si_meminfo_node(struct sysinfo *val, int nid)
{
	pg_data_t *pgdat = NODE_DATA(nid);

	val->totalram = pgdat->node_present_pages;
	val->freeram = nr_free_pages_pgdat(pgdat);
#ifdef CONFIG_HIGHMEM
	val->totalhigh = pgdat->node_zones[ZONE_HIGHMEM].present_pages;
	val->freehigh = pgdat->node_zones[ZONE_HIGHMEM].free_pages;
#else
	val->totalhigh = 0;
	val->freehigh = 0;
#endif
	val->mem_unit = PAGE_SIZE;
}
#endif

#define K(x) ((x) << (PAGE_SHIFT-10))

/*
 * Show free area list (used inside shift_scroll-lock stuff)
 * We also calculate the percentage fragmentation. We do this by counting the
 * memory on each free list with the exception of the first item on the list.
 */
void show_free_areas(void)
{
	int cpu;
	unsigned long active;
	unsigned long inactive;
	unsigned long free;
	struct zone *zone;

	for_each_zone(zone) {
		if (!populated_zone(zone))
			continue;

		show_node(zone);
		printk("%s per-cpu:\n", zone->name);

		for_each_online_cpu(cpu) {
			struct per_cpu_pageset *pageset;

			pageset = zone_pcp(zone, cpu);

			printk("CPU %4d: Hot: hi:%5d, btch:%4d usd:%4d   "
			       "Cold: hi:%5d, btch:%4d usd:%4d\n",
			       cpu, pageset->pcp[0].high,
			       pageset->pcp[0].batch, pageset->pcp[0].count,
			       pageset->pcp[1].high, pageset->pcp[1].batch,
			       pageset->pcp[1].count);
		}
	}

	get_zone_counts(&active, &inactive, &free);

	printk("Active:%lu inactive:%lu dirty:%lu writeback:%lu "
		"unstable:%lu free:%u slab:%lu mapped:%lu pagetables:%lu\n",
		active,
		inactive,
		global_page_state(NR_FILE_DIRTY),
		global_page_state(NR_WRITEBACK),
		global_page_state(NR_UNSTABLE_NFS),
		nr_free_pages(),
		global_page_state(NR_SLAB_RECLAIMABLE) +
			global_page_state(NR_SLAB_UNRECLAIMABLE),
		global_page_state(NR_FILE_MAPPED),
		global_page_state(NR_PAGETABLE));

	for_each_zone(zone) {
		int i;

		if (!populated_zone(zone))
			continue;

		show_node(zone);
		printk("%s"
			" free:%lukB"
			" min:%lukB"
			" low:%lukB"
			" high:%lukB"
			" active:%lukB"
			" inactive:%lukB"
			" present:%lukB"
			" pages_scanned:%lu"
			" all_unreclaimable? %s"
			"\n",
			zone->name,
			K(zone->free_pages),
			K(zone->pages_min),
			K(zone->pages_low),
			K(zone->pages_high),
			K(zone->nr_active),
			K(zone->nr_inactive),
			K(zone->present_pages),
			zone->pages_scanned,
			(zone->all_unreclaimable ? "yes" : "no")
			);
		printk("lowmem_reserve[]:");
		for (i = 0; i < MAX_NR_ZONES; i++)
			printk(" %lu", zone->lowmem_reserve[i]);
		printk("\n");
	}

	for_each_zone(zone) {
 		unsigned long nr[MAX_ORDER], flags, order, total = 0;

		if (!populated_zone(zone))
			continue;

		show_node(zone);
		printk("%s: ", zone->name);

		spin_lock_irqsave(&zone->lock, flags);
		for (order = 0; order < MAX_ORDER; order++) {
			nr[order] = zone->free_area[order].nr_free;
			total += nr[order] << order;
		}
		spin_unlock_irqrestore(&zone->lock, flags);
		for (order = 0; order < MAX_ORDER; order++)
			printk("%lu*%lukB ", nr[order], K(1UL) << order);
		printk("= %lukB\n", K(total));
	}

	show_swap_cache_info();
}

/*
 * Builds allocation fallback zone lists.
 *
 * Add all populated zones of a node to the zonelist.
 */
static int __meminit build_zonelists_node(pg_data_t *pgdat,
			struct zonelist *zonelist, int nr_zones, enum zone_type zone_type)
{
	struct zone *zone;

	BUG_ON(zone_type >= MAX_NR_ZONES);
	zone_type++;

	do {
		zone_type--;
		zone = pgdat->node_zones + zone_type;
		if (populated_zone(zone)) {
			zonelist->zones[nr_zones++] = zone;
			check_highest_zone(zone_type);
		}

	} while (zone_type);
	return nr_zones;
}

#ifdef CONFIG_NUMA
#define MAX_NODE_LOAD (num_online_nodes())
static int __meminitdata node_load[MAX_NUMNODES];
/**
 * find_next_best_node - find the next node that should appear in a given node's fallback list
 * @node: node whose fallback list we're appending
 * @used_node_mask: nodemask_t of already used nodes
 *
 * We use a number of factors to determine which is the next node that should
 * appear on a given node's fallback list.  The node should not have appeared
 * already in @node's fallback list, and it should be the next closest node
 * according to the distance array (which contains arbitrary distance values
 * from each node to each node in the system), and should also prefer nodes
 * with no CPUs, since presumably they'll have very little allocation pressure
 * on them otherwise.
 * It returns -1 if no node is found.
 */
static int __meminit find_next_best_node(int node, nodemask_t *used_node_mask)
{
	int n, val;
	int min_val = INT_MAX;
	int best_node = -1;

	/* Use the local node if we haven't already */
	if (!node_isset(node, *used_node_mask)) {
		node_set(node, *used_node_mask);
		return node;
	}

	for_each_online_node(n) {
		cpumask_t tmp;

		/* Don't want a node to appear more than once */
		if (node_isset(n, *used_node_mask))
			continue;

		/* Use the distance array to find the distance */
		val = node_distance(node, n);

		/* Penalize nodes under us ("prefer the next node") */
		val += (n < node);

		/* Give preference to headless and unused nodes */
		tmp = node_to_cpumask(n);
		if (!cpus_empty(tmp))
			val += PENALTY_FOR_NODE_WITH_CPUS;

		/* Slight preference for less loaded node */
		val *= (MAX_NODE_LOAD*MAX_NUMNODES);
		val += node_load[n];

		if (val < min_val) {
			min_val = val;
			best_node = n;
		}
	}

	if (best_node >= 0)
		node_set(best_node, *used_node_mask);

	return best_node;
}

static void __meminit build_zonelists(pg_data_t *pgdat)
{
	int j, node, local_node;
	enum zone_type i;
	int prev_node, load;
	struct zonelist *zonelist;
	nodemask_t used_mask;

	/* initialize zonelists */
	for (i = 0; i < MAX_NR_ZONES; i++) {
		zonelist = pgdat->node_zonelists + i;
		zonelist->zones[0] = NULL;
	}

	/* NUMA-aware ordering of nodes */
	local_node = pgdat->node_id;
	load = num_online_nodes();
	prev_node = local_node;
	nodes_clear(used_mask);
	while ((node = find_next_best_node(local_node, &used_mask)) >= 0) {
		int distance = node_distance(local_node, node);

		/*
		 * If another node is sufficiently far away then it is better
		 * to reclaim pages in a zone before going off node.
		 */
		if (distance > RECLAIM_DISTANCE)
			zone_reclaim_mode = 1;

		/*
		 * We don't want to pressure a particular node.
		 * So adding penalty to the first node in same
		 * distance group to make it round-robin.
		 */

		if (distance != node_distance(local_node, prev_node))
			node_load[node] += load;
		prev_node = node;
		load--;
		for (i = 0; i < MAX_NR_ZONES; i++) {
			zonelist = pgdat->node_zonelists + i;
			for (j = 0; zonelist->zones[j] != NULL; j++);

	 		j = build_zonelists_node(NODE_DATA(node), zonelist, j, i);
			zonelist->zones[j] = NULL;
		}
	}
}

#else	/* CONFIG_NUMA */

static void __meminit build_zonelists(pg_data_t *pgdat)
{
	int node, local_node;
	enum zone_type i,j;

	local_node = pgdat->node_id;
	for (i = 0; i < MAX_NR_ZONES; i++) {
		struct zonelist *zonelist;

		zonelist = pgdat->node_zonelists + i;

 		j = build_zonelists_node(pgdat, zonelist, 0, i);
 		/*
 		 * Now we build the zonelist so that it contains the zones
 		 * of all the other nodes.
 		 * We don't want to pressure a particular node, so when
 		 * building the zones for node N, we make sure that the
 		 * zones coming right after the local ones are those from
 		 * node N+1 (modulo N)
 		 */
		for (node = local_node + 1; node < MAX_NUMNODES; node++) {
			if (!node_online(node))
				continue;
			j = build_zonelists_node(NODE_DATA(node), zonelist, j, i);
		}
		for (node = 0; node < local_node; node++) {
			if (!node_online(node))
				continue;
			j = build_zonelists_node(NODE_DATA(node), zonelist, j, i);
		}

		zonelist->zones[j] = NULL;
	}
}

#endif	/* CONFIG_NUMA */

/* return values int ....just for stop_machine_run() */
static int __meminit __build_all_zonelists(void *dummy)
{
	int nid;
	for_each_online_node(nid)
		build_zonelists(NODE_DATA(nid));
	return 0;
}

void __meminit build_all_zonelists(void)
{
	if (system_state == SYSTEM_BOOTING) {
		__build_all_zonelists(NULL);
		cpuset_init_current_mems_allowed();
	} else {
		/* we have to stop all cpus to guaranntee there is no user
		   of zonelist */
		stop_machine_run(__build_all_zonelists, NULL, NR_CPUS);
		/* cpuset refresh routine should be here */
	}
	vm_total_pages = nr_free_pagecache_pages();
	printk("Built %i zonelists.  Total pages: %ld\n",
			num_online_nodes(), vm_total_pages);
}

/*
 * Helper functions to size the waitqueue hash table.
 * Essentially these want to choose hash table sizes sufficiently
 * large so that collisions trying to wait on pages are rare.
 * But in fact, the number of active page waitqueues on typical
 * systems is ridiculously low, less than 200. So this is even
 * conservative, even though it seems large.
 *
 * The constant PAGES_PER_WAITQUEUE specifies the ratio of pages to
 * waitqueues, i.e. the size of the waitq table given the number of pages.
 */
#define PAGES_PER_WAITQUEUE	256

#ifndef CONFIG_MEMORY_HOTPLUG
static inline unsigned long wait_table_hash_nr_entries(unsigned long pages)
{
	unsigned long size = 1;

	pages /= PAGES_PER_WAITQUEUE;

	while (size < pages)
		size <<= 1;

	/*
	 * Once we have dozens or even hundreds of threads sleeping
	 * on IO we've got bigger problems than wait queue collision.
	 * Limit the size of the wait table to a reasonable size.
	 */
	size = min(size, 4096UL);

	return max(size, 4UL);
}
#else
/*
 * A zone's size might be changed by hot-add, so it is not possible to determine
 * a suitable size for its wait_table.  So we use the maximum size now.
 *
 * The max wait table size = 4096 x sizeof(wait_queue_head_t).   ie:
 *
 *    i386 (preemption config)    : 4096 x 16 = 64Kbyte.
 *    ia64, x86-64 (no preemption): 4096 x 20 = 80Kbyte.
 *    ia64, x86-64 (preemption)   : 4096 x 24 = 96Kbyte.
 *
 * The maximum entries are prepared when a zone's memory is (512K + 256) pages
 * or more by the traditional way. (See above).  It equals:
 *
 *    i386, x86-64, powerpc(4K page size) : =  ( 2G + 1M)byte.
 *    ia64(16K page size)                 : =  ( 8G + 4M)byte.
 *    powerpc (64K page size)             : =  (32G +16M)byte.
 */
static inline unsigned long wait_table_hash_nr_entries(unsigned long pages)
{
	return 4096UL;
}
#endif

/*
 * This is an integer logarithm so that shifts can be used later
 * to extract the more random high bits from the multiplicative
 * hash function before the remainder is taken.
 */
static inline unsigned long wait_table_bits(unsigned long size)
{
	return ffz(~size);
}

#define LONG_ALIGN(x) (((x)+(sizeof(long))-1)&~((sizeof(long))-1))

/*
 * Initially all pages are reserved - free ones are freed
 * up by free_all_bootmem() once the early boot process is
 * done. Non-atomic initialization, single-pass.
 */
void __meminit memmap_init_zone(unsigned long size, int nid, unsigned long zone,
		unsigned long start_pfn)
{
	struct page *page;
	unsigned long end_pfn = start_pfn + size;
	unsigned long pfn;

	for (pfn = start_pfn; pfn < end_pfn; pfn++) {
		if (!early_pfn_valid(pfn))
			continue;
		if (!early_pfn_in_nid(pfn, nid))
			continue;
		page = pfn_to_page(pfn);
		set_page_links(page, zone, nid, pfn);
		init_page_count(page);
		reset_page_mapcount(page);
		SetPageReserved(page);
		INIT_LIST_HEAD(&page->lru);
#ifdef WANT_PAGE_VIRTUAL
		/* The shift won't overflow because ZONE_NORMAL is below 4G. */
		if (!is_highmem_idx(zone))
			set_page_address(page, __va(pfn << PAGE_SHIFT));
#endif
	}
}

void zone_init_free_lists(struct pglist_data *pgdat, struct zone *zone,
				unsigned long size)
{
	int order;
	for (order = 0; order < MAX_ORDER ; order++) {
		INIT_LIST_HEAD(&zone->free_area[order].free_list);
		zone->free_area[order].nr_free = 0;
	}
}

#define ZONETABLE_INDEX(x, zone_nr)	((x << ZONES_SHIFT) | zone_nr)
void zonetable_add(struct zone *zone, int nid, enum zone_type zid,
		unsigned long pfn, unsigned long size)
{
	unsigned long snum = pfn_to_section_nr(pfn);
	unsigned long end = pfn_to_section_nr(pfn + size);

	if (FLAGS_HAS_NODE)
		zone_table[ZONETABLE_INDEX(nid, zid)] = zone;
	else
		for (; snum <= end; snum++)
			zone_table[ZONETABLE_INDEX(snum, zid)] = zone;
}

#ifndef __HAVE_ARCH_MEMMAP_INIT
#define memmap_init(size, nid, zone, start_pfn) \
	memmap_init_zone((size), (nid), (zone), (start_pfn))
#endif

static int __cpuinit zone_batchsize(struct zone *zone)
{
	int batch;

	/*
	 * The per-cpu-pages pools are set to around 1000th of the
	 * size of the zone.  But no more than 1/2 of a meg.
	 *
	 * OK, so we don't know how big the cache is.  So guess.
	 */
	batch = zone->present_pages / 1024;
	if (batch * PAGE_SIZE > 512 * 1024)
		batch = (512 * 1024) / PAGE_SIZE;
	batch /= 4;		/* We effectively *= 4 below */
	if (batch < 1)
		batch = 1;

	/*
	 * Clamp the batch to a 2^n - 1 value. Having a power
	 * of 2 value was found to be more likely to have
	 * suboptimal cache aliasing properties in some cases.
	 *
	 * For example if 2 tasks are alternately allocating
	 * batches of pages, one task can end up with a lot
	 * of pages of one half of the possible page colors
	 * and the other with pages of the other colors.
	 */
	batch = (1 << (fls(batch + batch/2)-1)) - 1;

	return batch;
}

inline void setup_pageset(struct per_cpu_pageset *p, unsigned long batch)
{
	struct per_cpu_pages *pcp;

	memset(p, 0, sizeof(*p));

	pcp = &p->pcp[0];		/* hot */
	pcp->count = 0;
	pcp->high = 6 * batch;
	pcp->batch = max(1UL, 1 * batch);
	INIT_LIST_HEAD(&pcp->list);

	pcp = &p->pcp[1];		/* cold*/
	pcp->count = 0;
	pcp->high = 2 * batch;
	pcp->batch = max(1UL, batch/2);
	INIT_LIST_HEAD(&pcp->list);
}

/*
 * setup_pagelist_highmark() sets the high water mark for hot per_cpu_pagelist
 * to the value high for the pageset p.
 */

static void setup_pagelist_highmark(struct per_cpu_pageset *p,
				unsigned long high)
{
	struct per_cpu_pages *pcp;

	pcp = &p->pcp[0]; /* hot list */
	pcp->high = high;
	pcp->batch = max(1UL, high/4);
	if ((high/4) > (PAGE_SHIFT * 8))
		pcp->batch = PAGE_SHIFT * 8;
}


#ifdef CONFIG_NUMA
/*
 * Boot pageset table. One per cpu which is going to be used for all
 * zones and all nodes. The parameters will be set in such a way
 * that an item put on a list will immediately be handed over to
 * the buddy list. This is safe since pageset manipulation is done
 * with interrupts disabled.
 *
 * Some NUMA counter updates may also be caught by the boot pagesets.
 *
 * The boot_pagesets must be kept even after bootup is complete for
 * unused processors and/or zones. They do play a role for bootstrapping
 * hotplugged processors.
 *
 * zoneinfo_show() and maybe other functions do
 * not check if the processor is online before following the pageset pointer.
 * Other parts of the kernel may not check if the zone is available.
 */
static struct per_cpu_pageset boot_pageset[NR_CPUS];

/*
 * Dynamically allocate memory for the
 * per cpu pageset array in struct zone.
 */
static int __cpuinit process_zones(int cpu)
{
	struct zone *zone, *dzone;

	for_each_zone(zone) {

		if (!populated_zone(zone))
			continue;

		zone_pcp(zone, cpu) = kmalloc_node(sizeof(struct per_cpu_pageset),
					 GFP_KERNEL, cpu_to_node(cpu));
		if (!zone_pcp(zone, cpu))
			goto bad;

		setup_pageset(zone_pcp(zone, cpu), zone_batchsize(zone));

		if (percpu_pagelist_fraction)
			setup_pagelist_highmark(zone_pcp(zone, cpu),
			 	(zone->present_pages / percpu_pagelist_fraction));
	}

	return 0;
bad:
	for_each_zone(dzone) {
		if (dzone == zone)
			break;
		kfree(zone_pcp(dzone, cpu));
		zone_pcp(dzone, cpu) = NULL;
	}
	return -ENOMEM;
}

static inline void free_zone_pagesets(int cpu)
{
	struct zone *zone;

	for_each_zone(zone) {
		struct per_cpu_pageset *pset = zone_pcp(zone, cpu);

		/* Free per_cpu_pageset if it is slab allocated */
		if (pset != &boot_pageset[cpu])
			kfree(pset);
		zone_pcp(zone, cpu) = NULL;
	}
}

static int __cpuinit pageset_cpuup_callback(struct notifier_block *nfb,
		unsigned long action,
		void *hcpu)
{
	int cpu = (long)hcpu;
	int ret = NOTIFY_OK;

	switch (action) {
		case CPU_UP_PREPARE:
			if (process_zones(cpu))
				ret = NOTIFY_BAD;
			break;
		case CPU_UP_CANCELED:
		case CPU_DEAD:
			free_zone_pagesets(cpu);
			break;
		default:
			break;
	}
	return ret;
}

static struct notifier_block __cpuinitdata pageset_notifier =
	{ &pageset_cpuup_callback, NULL, 0 };

void __init setup_per_cpu_pageset(void)
{
	int err;

	/* Initialize per_cpu_pageset for cpu 0.
	 * A cpuup callback will do this for every cpu
	 * as it comes online
	 */
	err = process_zones(smp_processor_id());
	BUG_ON(err);
	register_cpu_notifier(&pageset_notifier);
}

#endif

static __meminit
int zone_wait_table_init(struct zone *zone, unsigned long zone_size_pages)
{
	int i;
	struct pglist_data *pgdat = zone->zone_pgdat;
	size_t alloc_size;

	/*
	 * The per-page waitqueue mechanism uses hashed waitqueues
	 * per zone.
	 */
	zone->wait_table_hash_nr_entries =
		 wait_table_hash_nr_entries(zone_size_pages);
	zone->wait_table_bits =
		wait_table_bits(zone->wait_table_hash_nr_entries);
	alloc_size = zone->wait_table_hash_nr_entries
					* sizeof(wait_queue_head_t);

 	if (system_state == SYSTEM_BOOTING) {
		zone->wait_table = (wait_queue_head_t *)
			alloc_bootmem_node(pgdat, alloc_size);
	} else {
		/*
		 * This case means that a zone whose size was 0 gets new memory
		 * via memory hot-add.
		 * But it may be the case that a new node was hot-added.  In
		 * this case vmalloc() will not be able to use this new node's
		 * memory - this wait_table must be initialized to use this new
		 * node itself as well.
		 * To use this new node's memory, further consideration will be
		 * necessary.
		 */
		zone->wait_table = (wait_queue_head_t *)vmalloc(alloc_size);
	}
	if (!zone->wait_table)
		return -ENOMEM;
	printk("%s zone->wait_table=0x%08x",__FUNCTION__,zone->wait_table);
	for(i = 0; i < zone->wait_table_hash_nr_entries; ++i)
		init_waitqueue_head(zone->wait_table + i);

	return 0;
}

static __meminit void zone_pcp_init(struct zone *zone)
{
	int cpu;
	unsigned long batch = zone_batchsize(zone);

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
#ifdef CONFIG_NUMA
		/* Early boot. Slab allocator not functional yet */
		zone_pcp(zone, cpu) = &boot_pageset[cpu];
		setup_pageset(&boot_pageset[cpu],0);
#else
		setup_pageset(zone_pcp(zone,cpu), batch);
#endif
	}
	if (zone->present_pages)
		printk(KERN_DEBUG "  %s zone: %lu pages, LIFO batch:%lu\n",
			zone->name, zone->present_pages, batch);
}

__meminit int init_currently_empty_zone(struct zone *zone,
					unsigned long zone_start_pfn,
					unsigned long size)
{
	struct pglist_data *pgdat = zone->zone_pgdat;
	int ret;
	ret = zone_wait_table_init(zone, size);
	if (ret)
		return ret;
	pgdat->nr_zones = zone_idx(zone) + 1;

	zone->zone_start_pfn = zone_start_pfn;
	printk("memmap_init zone_start_pfn=%d size=%d!\n",zone_start_pfn,size);
	memmap_init(size, pgdat->node_id, zone_idx(zone), zone_start_pfn);

	zone_init_free_lists(pgdat, zone, zone->spanned_pages);

	return 0;
}

#ifdef CONFIG_ARCH_POPULATES_NODE_MAP
/*
 * Basic iterator support. Return the first range of PFNs for a node
 * Note: nid == MAX_NUMNODES returns first region regardless of node
 */
static int __init first_active_region_index_in_nid(int nid)
{
	int i;

	for (i = 0; i < nr_nodemap_entries; i++)
		if (nid == MAX_NUMNODES || early_node_map[i].nid == nid)
			return i;

	return -1;
}

/*
 * Basic iterator support. Return the next active range of PFNs for a node
 * Note: nid == MAX_NUMNODES returns next region regardles of node
 */
static int __init next_active_region_index_in_nid(int index, int nid)
{
	for (index = index + 1; index < nr_nodemap_entries; index++)
		if (nid == MAX_NUMNODES || early_node_map[index].nid == nid)
			return index;

	return -1;
}

#ifndef CONFIG_HAVE_ARCH_EARLY_PFN_TO_NID
/*
 * Required by SPARSEMEM. Given a PFN, return what node the PFN is on.
 * Architectures may implement their own version but if add_active_range()
 * was used and there are no special requirements, this is a convenient
 * alternative
 */
int __init early_pfn_to_nid(unsigned long pfn)
{
	int i;

	for (i = 0; i < nr_nodemap_entries; i++) {
		unsigned long start_pfn = early_node_map[i].start_pfn;
		unsigned long end_pfn = early_node_map[i].end_pfn;

		if (start_pfn <= pfn && pfn < end_pfn)
			return early_node_map[i].nid;
	}

	return 0;
}
#endif /* CONFIG_HAVE_ARCH_EARLY_PFN_TO_NID */

/* Basic iterator support to walk early_node_map[] */
#define for_each_active_range_index_in_nid(i, nid) \
	for (i = first_active_region_index_in_nid(nid); i != -1; \
				i = next_active_region_index_in_nid(i, nid))

/**
 * free_bootmem_with_active_regions - Call free_bootmem_node for each active range
 * @nid: The node to free memory on. If MAX_NUMNODES, all nodes are freed.
 * @max_low_pfn: The highest PFN that will be passed to free_bootmem_node
 *
 * If an architecture guarantees that all ranges registered with
 * add_active_ranges() contain no holes and may be freed, this
 * this function may be used instead of calling free_bootmem() manually.
 */
void __init free_bootmem_with_active_regions(int nid,
						unsigned long max_low_pfn)
{
	int i;

	for_each_active_range_index_in_nid(i, nid) {
		unsigned long size_pages = 0;
		unsigned long end_pfn = early_node_map[i].end_pfn;

		if (early_node_map[i].start_pfn >= max_low_pfn)
			continue;

		if (end_pfn > max_low_pfn)
			end_pfn = max_low_pfn;

		size_pages = end_pfn - early_node_map[i].start_pfn;
		free_bootmem_node(NODE_DATA(early_node_map[i].nid),
				PFN_PHYS(early_node_map[i].start_pfn),
				size_pages << PAGE_SHIFT);
	}
}

/**
 * sparse_memory_present_with_active_regions - Call memory_present for each active range
 * @nid: The node to call memory_present for. If MAX_NUMNODES, all nodes will be used.
 *
 * If an architecture guarantees that all ranges registered with
 * add_active_ranges() contain no holes and may be freed, this
 * function may be used instead of calling memory_present() manually.
 */
void __init sparse_memory_present_with_active_regions(int nid)
{
	int i;

	for_each_active_range_index_in_nid(i, nid)
		memory_present(early_node_map[i].nid,
				early_node_map[i].start_pfn,
				early_node_map[i].end_pfn);
}

/**
 * push_node_boundaries - Push node boundaries to at least the requested boundary
 * @nid: The nid of the node to push the boundary for
 * @start_pfn: The start pfn of the node
 * @end_pfn: The end pfn of the node
 *
 * In reserve-based hot-add, mem_map is allocated that is unused until hotadd
 * time. Specifically, on x86_64, SRAT will report ranges that can potentially
 * be hotplugged even though no physical memory exists. This function allows
 * an arch to push out the node boundaries so mem_map is allocated that can
 * be used later.
 */
#ifdef CONFIG_MEMORY_HOTPLUG_RESERVE
void __init push_node_boundaries(unsigned int nid,
		unsigned long start_pfn, unsigned long end_pfn)
{
	printk(KERN_DEBUG "Entering push_node_boundaries(%u, %lu, %lu)\n",
			nid, start_pfn, end_pfn);

	/* Initialise the boundary for this node if necessary */
	if (node_boundary_end_pfn[nid] == 0)
		node_boundary_start_pfn[nid] = -1UL;

	/* Update the boundaries */
	if (node_boundary_start_pfn[nid] > start_pfn)
		node_boundary_start_pfn[nid] = start_pfn;
	if (node_boundary_end_pfn[nid] < end_pfn)
		node_boundary_end_pfn[nid] = end_pfn;
}

/* If necessary, push the node boundary out for reserve hotadd */
static void __init account_node_boundary(unsigned int nid,
		unsigned long *start_pfn, unsigned long *end_pfn)
{
	printk(KERN_DEBUG "Entering account_node_boundary(%u, %lu, %lu)\n",
			nid, *start_pfn, *end_pfn);

	/* Return if boundary information has not been provided */
	if (node_boundary_end_pfn[nid] == 0)
		return;

	/* Check the boundaries and update if necessary */
	if (node_boundary_start_pfn[nid] < *start_pfn)
		*start_pfn = node_boundary_start_pfn[nid];
	if (node_boundary_end_pfn[nid] > *end_pfn)
		*end_pfn = node_boundary_end_pfn[nid];
}
#else
void __init push_node_boundaries(unsigned int nid,
		unsigned long start_pfn, unsigned long end_pfn) {}

static void __init account_node_boundary(unsigned int nid,
		unsigned long *start_pfn, unsigned long *end_pfn) {}
#endif


/**
 * get_pfn_range_for_nid - Return the start and end page frames for a node
 * @nid: The nid to return the range for. If MAX_NUMNODES, the min and max PFN are returned.
 * @start_pfn: Passed by reference. On return, it will have the node start_pfn.
 * @end_pfn: Passed by reference. On return, it will have the node end_pfn.
 *
 * It returns the start and end page frame of a node based on information
 * provided by an arch calling add_active_range(). If called for a node
 * with no available memory, a warning is printed and the start and end
 * PFNs will be 0.
 */
void __init get_pfn_range_for_nid(unsigned int nid,
			unsigned long *start_pfn, unsigned long *end_pfn)
{
	int i;
	*start_pfn = -1UL;
	*end_pfn = 0;

	for_each_active_range_index_in_nid(i, nid) {
		*start_pfn = min(*start_pfn, early_node_map[i].start_pfn);
		*end_pfn = max(*end_pfn, early_node_map[i].end_pfn);
	}

	if (*start_pfn == -1UL) {
		printk(KERN_WARNING "Node %u active with no memory\n", nid);
		*start_pfn = 0;
	}

	/* Push the node boundaries out if requested */
	account_node_boundary(nid, start_pfn, end_pfn);
}

/*
 * Return the number of pages a zone spans in a node, including holes
 * present_pages = zone_spanned_pages_in_node() - zone_absent_pages_in_node()
 */
unsigned long __init zone_spanned_pages_in_node(int nid,
					unsigned long zone_type,
					unsigned long *ignored)
{
	unsigned long node_start_pfn, node_end_pfn;
	unsigned long zone_start_pfn, zone_end_pfn;

	/* Get the start and end of the node and zone */
	get_pfn_range_for_nid(nid, &node_start_pfn, &node_end_pfn);
	zone_start_pfn = arch_zone_lowest_possible_pfn[zone_type];
	zone_end_pfn = arch_zone_highest_possible_pfn[zone_type];

	/* Check that this node has pages within the zone's required range */
	if (zone_end_pfn < node_start_pfn || zone_start_pfn > node_end_pfn)
		return 0;

	/* Move the zone boundaries inside the node if necessary */
	zone_end_pfn = min(zone_end_pfn, node_end_pfn);
	zone_start_pfn = max(zone_start_pfn, node_start_pfn);

	/* Return the spanned pages */
	return zone_end_pfn - zone_start_pfn;
}

/*
 * Return the number of holes in a range on a node. If nid is MAX_NUMNODES,
 * then all holes in the requested range will be accounted for.
 */
unsigned long __init __absent_pages_in_range(int nid,
				unsigned long range_start_pfn,
				unsigned long range_end_pfn)
{
	int i = 0;
	unsigned long prev_end_pfn = 0, hole_pages = 0;
	unsigned long start_pfn;

	/* Find the end_pfn of the first active range of pfns in the node */
	i = first_active_region_index_in_nid(nid);
	if (i == -1)
		return 0;

	/* Account for ranges before physical memory on this node */
	if (early_node_map[i].start_pfn > range_start_pfn)
		hole_pages = early_node_map[i].start_pfn - range_start_pfn;

	prev_end_pfn = early_node_map[i].start_pfn;

	/* Find all holes for the zone within the node */
	for (; i != -1; i = next_active_region_index_in_nid(i, nid)) {

		/* No need to continue if prev_end_pfn is outside the zone */
		if (prev_end_pfn >= range_end_pfn)
			break;

		/* Make sure the end of the zone is not within the hole */
		start_pfn = min(early_node_map[i].start_pfn, range_end_pfn);
		prev_end_pfn = max(prev_end_pfn, range_start_pfn);

		/* Update the hole size cound and move on */
		if (start_pfn > range_start_pfn) {
			BUG_ON(prev_end_pfn > start_pfn);
			hole_pages += start_pfn - prev_end_pfn;
		}
		prev_end_pfn = early_node_map[i].end_pfn;
	}

	/* Account for ranges past physical memory on this node */
	if (range_end_pfn > prev_end_pfn)
		hole_pages += range_end_pfn -
				max(range_start_pfn, prev_end_pfn);

	return hole_pages;
}

/**
 * absent_pages_in_range - Return number of page frames in holes within a range
 * @start_pfn: The start PFN to start searching for holes
 * @end_pfn: The end PFN to stop searching for holes
 *
 * It returns the number of pages frames in memory holes within a range.
 */
unsigned long __init absent_pages_in_range(unsigned long start_pfn,
							unsigned long end_pfn)
{
	return __absent_pages_in_range(MAX_NUMNODES, start_pfn, end_pfn);
}

/* Return the number of page frames in holes in a zone on a node */
unsigned long __init zone_absent_pages_in_node(int nid,
					unsigned long zone_type,
					unsigned long *ignored)
{
	unsigned long node_start_pfn, node_end_pfn;
	unsigned long zone_start_pfn, zone_end_pfn;

	get_pfn_range_for_nid(nid, &node_start_pfn, &node_end_pfn);
	zone_start_pfn = max(arch_zone_lowest_possible_pfn[zone_type],
							node_start_pfn);
	zone_end_pfn = min(arch_zone_highest_possible_pfn[zone_type],
							node_end_pfn);

	return __absent_pages_in_range(nid, zone_start_pfn, zone_end_pfn);
}

#else
static inline unsigned long zone_spanned_pages_in_node(int nid,
					unsigned long zone_type,
					unsigned long *zones_size)
{
	return zones_size[zone_type];
}

static inline unsigned long zone_absent_pages_in_node(int nid,
						unsigned long zone_type,
						unsigned long *zholes_size)
{
	if (!zholes_size)
		return 0;

	return zholes_size[zone_type];
}

#endif

static void __init calculate_node_totalpages(struct pglist_data *pgdat,
		unsigned long *zones_size, unsigned long *zholes_size)
{
	unsigned long realtotalpages, totalpages = 0;
	enum zone_type i;

	for (i = 0; i < MAX_NR_ZONES; i++)
		totalpages += zone_spanned_pages_in_node(pgdat->node_id, i,
								zones_size);
	pgdat->node_spanned_pages = totalpages;

	realtotalpages = totalpages;
	for (i = 0; i < MAX_NR_ZONES; i++)
		realtotalpages -=
			zone_absent_pages_in_node(pgdat->node_id, i,
								zholes_size);
	pgdat->node_present_pages = realtotalpages;
	printk(KERN_DEBUG "On node %d totalpages: %lu\n", pgdat->node_id,
							realtotalpages);
}

/*
 * Set up the zone data structures:
 *   - mark all pages reserved
 *   - mark all memory queues empty
 *   - clear the memory bitmaps
 */
static void __meminit free_area_init_core(struct pglist_data *pgdat,
		unsigned long *zones_size, unsigned long *zholes_size)
{
	enum zone_type j;
	int nid = pgdat->node_id;
	unsigned long zone_start_pfn = pgdat->node_start_pfn;
	int ret;

	pgdat_resize_init(pgdat);
	pgdat->nr_zones = 0;
	init_waitqueue_head(&pgdat->kswapd_wait);
	pgdat->kswapd_max_order = 0;
	
	for (j = 0; j < MAX_NR_ZONES; j++) {
		struct zone *zone = pgdat->node_zones + j;
		unsigned long size, realsize, memmap_pages;

		size = zone_spanned_pages_in_node(nid, j, zones_size);
		realsize = size - zone_absent_pages_in_node(nid, j,
								zholes_size);

		/*
		 * Adjust realsize so that it accounts for how much memory
		 * is used by this zone for memmap. This affects the watermark
		 * and per-cpu initialisations
		 */
		memmap_pages = (size * sizeof(struct page)) >> PAGE_SHIFT;
		if (realsize >= memmap_pages) {
			realsize -= memmap_pages;
			printk(KERN_DEBUG
				"  %s zone: %lu pages used for memmap\n",
				zone_names[j], memmap_pages);
		} else
			printk(KERN_WARNING
				"  %s zone: %lu pages exceeds realsize %lu\n",
				zone_names[j], memmap_pages, realsize);

		/* Account for reserved DMA pages */
		if (j == ZONE_DMA && realsize > dma_reserve) {
			realsize -= dma_reserve;
			printk(KERN_DEBUG "  DMA zone: %lu pages reserved\n",
								dma_reserve);
		}

		if (!is_highmem_idx(j))
			nr_kernel_pages += realsize;
		nr_all_pages += realsize;

		zone->spanned_pages = size;
		zone->present_pages = realsize;
#ifdef CONFIG_NUMA
		zone->node = nid;
		zone->min_unmapped_pages = (realsize*sysctl_min_unmapped_ratio)
						/ 100;
		zone->min_slab_pages = (realsize * sysctl_min_slab_ratio) / 100;
#endif
		zone->name = zone_names[j];
		spin_lock_init(&zone->lock);
		spin_lock_init(&zone->lru_lock);
		zone_seqlock_init(zone);
		zone->zone_pgdat = pgdat;
		zone->free_pages = 0;

		zone->prev_priority = DEF_PRIORITY;

		zone_pcp_init(zone);
		INIT_LIST_HEAD(&zone->active_list);
		INIT_LIST_HEAD(&zone->inactive_list);
		zone->nr_scan_active = 0;
		zone->nr_scan_inactive = 0;
		zone->nr_active = 0;
		zone->nr_inactive = 0;
		zap_zone_vm_stats(zone);
		atomic_set(&zone->reclaim_in_progress, 0);
		if (!size)
			continue;

		zonetable_add(zone, nid, j, zone_start_pfn, size);
		ret = init_currently_empty_zone(zone, zone_start_pfn, size);
		BUG_ON(ret);
		zone_start_pfn += size;
	}
}

static void __init alloc_node_mem_map(struct pglist_data *pgdat)
{
	/* Skip empty nodes */
	if (!pgdat->node_spanned_pages)
		return;

#ifdef CONFIG_FLAT_NODE_MEM_MAP
	/* ia64 gets its own node_mem_map, before this, without bootmem */
	if (!pgdat->node_mem_map) {
		unsigned long size, start, end;
		struct page *map;

		/*
		 * The zone's endpoints aren't required to be MAX_ORDER
		 * aligned but the node_mem_map endpoints must be in order
		 * for the buddy allocator to function correctly.
		 */
		start = pgdat->node_start_pfn & ~(MAX_ORDER_NR_PAGES - 1);
		end = pgdat->node_start_pfn + pgdat->node_spanned_pages;
		end = ALIGN(end, MAX_ORDER_NR_PAGES);
		size =  (end - start) * sizeof(struct page);
		printk("%s start=%d end=%d size=%d\n",__FUNCTION__,start,end,size);
		map = alloc_remap(pgdat->node_id, size);
		if (!map)
			map = alloc_bootmem_node(pgdat, size);		
		pgdat->node_mem_map = map + (pgdat->node_start_pfn - start);
		printk("%s map=0x%08x node_mem_map=0x%08x\n",__FUNCTION__,map,pgdat->node_mem_map);
	}
#ifdef CONFIG_FLATMEM
	/*
	 * With no DISCONTIG, the global mem_map is just set as node 0's
	 */
	if (pgdat == NODE_DATA(0)) {
		mem_map = NODE_DATA(0)->node_mem_map;
#ifdef CONFIG_ARCH_POPULATES_NODE_MAP
		if (page_to_pfn(mem_map) != pgdat->node_start_pfn)
			mem_map -= pgdat->node_start_pfn;
#endif /* CONFIG_ARCH_POPULATES_NODE_MAP */
	}
#endif
#endif /* CONFIG_FLAT_NODE_MEM_MAP */
}

void __meminit free_area_init_node(int nid, struct pglist_data *pgdat,
		unsigned long *zones_size, unsigned long node_start_pfn,
		unsigned long *zholes_size)
{
	pgdat->node_id = nid;
	pgdat->node_start_pfn = node_start_pfn;
	calculate_node_totalpages(pgdat, zones_size, zholes_size);
	printk("%s pgdat:node_start_pfn=0x%08x node_spanned_pages=0x%08x\n node_mem_map=0x%08x\n",__FUNCTION__,pgdat->node_start_pfn,pgdat->node_spanned_pages,pgdat->node_mem_map);
	alloc_node_mem_map(pgdat);

	free_area_init_core(pgdat, zones_size, zholes_size);
}

#ifdef CONFIG_ARCH_POPULATES_NODE_MAP
/**
 * add_active_range - Register a range of PFNs backed by physical memory
 * @nid: The node ID the range resides on
 * @start_pfn: The start PFN of the available physical memory
 * @end_pfn: The end PFN of the available physical memory
 *
 * These ranges are stored in an early_node_map[] and later used by
 * free_area_init_nodes() to calculate zone sizes and holes. If the
 * range spans a memory hole, it is up to the architecture to ensure
 * the memory is not freed by the bootmem allocator. If possible
 * the range being registered will be merged with existing ranges.
 */
void __init add_active_range(unsigned int nid, unsigned long start_pfn,
						unsigned long end_pfn)
{
	int i;

	printk(KERN_DEBUG "Entering add_active_range(%d, %lu, %lu) "
			  "%d entries of %d used\n",
			  nid, start_pfn, end_pfn,
			  nr_nodemap_entries, MAX_ACTIVE_REGIONS);

	/* Merge with existing active regions if possible */
	for (i = 0; i < nr_nodemap_entries; i++) {
		if (early_node_map[i].nid != nid)
			continue;

		/* Skip if an existing region covers this new one */
		if (start_pfn >= early_node_map[i].start_pfn &&
				end_pfn <= early_node_map[i].end_pfn)
			return;

		/* Merge forward if suitable */
		if (start_pfn <= early_node_map[i].end_pfn &&
				end_pfn > early_node_map[i].end_pfn) {
			early_node_map[i].end_pfn = end_pfn;
			return;
		}

		/* Merge backward if suitable */
		if (start_pfn < early_node_map[i].end_pfn &&
				end_pfn >= early_node_map[i].start_pfn) {
			early_node_map[i].start_pfn = start_pfn;
			return;
		}
	}

	/* Check that early_node_map is large enough */
	if (i >= MAX_ACTIVE_REGIONS) {
		printk(KERN_CRIT "More than %d memory regions, truncating\n",
							MAX_ACTIVE_REGIONS);
		return;
	}

	early_node_map[i].nid = nid;
	early_node_map[i].start_pfn = start_pfn;
	early_node_map[i].end_pfn = end_pfn;
	nr_nodemap_entries = i + 1;
}

/**
 * shrink_active_range - Shrink an existing registered range of PFNs
 * @nid: The node id the range is on that should be shrunk
 * @old_end_pfn: The old end PFN of the range
 * @new_end_pfn: The new PFN of the range
 *
 * i386 with NUMA use alloc_remap() to store a node_mem_map on a local node.
 * The map is kept at the end physical page range that has already been
 * registered with add_active_range(). This function allows an arch to shrink
 * an existing registered range.
 */
void __init shrink_active_range(unsigned int nid, unsigned long old_end_pfn,
						unsigned long new_end_pfn)
{
	int i;

	/* Find the old active region end and shrink */
	for_each_active_range_index_in_nid(i, nid)
		if (early_node_map[i].end_pfn == old_end_pfn) {
			early_node_map[i].end_pfn = new_end_pfn;
			break;
		}
}

/**
 * remove_all_active_ranges - Remove all currently registered regions
 *
 * During discovery, it may be found that a table like SRAT is invalid
 * and an alternative discovery method must be used. This function removes
 * all currently registered regions.
 */
void __init remove_all_active_ranges(void)
{
	memset(early_node_map, 0, sizeof(early_node_map));
	nr_nodemap_entries = 0;
#ifdef CONFIG_MEMORY_HOTPLUG_RESERVE
	memset(node_boundary_start_pfn, 0, sizeof(node_boundary_start_pfn));
	memset(node_boundary_end_pfn, 0, sizeof(node_boundary_end_pfn));
#endif /* CONFIG_MEMORY_HOTPLUG_RESERVE */
}

/* Compare two active node_active_regions */
static int __init cmp_node_active_region(const void *a, const void *b)
{
	struct node_active_region *arange = (struct node_active_region *)a;
	struct node_active_region *brange = (struct node_active_region *)b;

	/* Done this way to avoid overflows */
	if (arange->start_pfn > brange->start_pfn)
		return 1;
	if (arange->start_pfn < brange->start_pfn)
		return -1;

	return 0;
}

/* sort the node_map by start_pfn */
static void __init sort_node_map(void)
{
	sort(early_node_map, (size_t)nr_nodemap_entries,
			sizeof(struct node_active_region),
			cmp_node_active_region, NULL);
}

/* Find the lowest pfn for a node. This depends on a sorted early_node_map */
unsigned long __init find_min_pfn_for_node(unsigned long nid)
{
	int i;

	/* Regions in the early_node_map can be in any order */
	sort_node_map();

	/* Assuming a sorted map, the first range found has the starting pfn */
	for_each_active_range_index_in_nid(i, nid)
		return early_node_map[i].start_pfn;

	printk(KERN_WARNING "Could not find start_pfn for node %lu\n", nid);
	return 0;
}

/**
 * find_min_pfn_with_active_regions - Find the minimum PFN registered
 *
 * It returns the minimum PFN based on information provided via
 * add_active_range().
 */
unsigned long __init find_min_pfn_with_active_regions(void)
{
	return find_min_pfn_for_node(MAX_NUMNODES);
}

/**
 * find_max_pfn_with_active_regions - Find the maximum PFN registered
 *
 * It returns the maximum PFN based on information provided via
 * add_active_range().
 */
unsigned long __init find_max_pfn_with_active_regions(void)
{
	int i;
	unsigned long max_pfn = 0;

	for (i = 0; i < nr_nodemap_entries; i++)
		max_pfn = max(max_pfn, early_node_map[i].end_pfn);

	return max_pfn;
}

/**
 * free_area_init_nodes - Initialise all pg_data_t and zone data
 * @max_zone_pfn: an array of max PFNs for each zone
 *
 * This will call free_area_init_node() for each active node in the system.
 * Using the page ranges provided by add_active_range(), the size of each
 * zone in each node and their holes is calculated. If the maximum PFN
 * between two adjacent zones match, it is assumed that the zone is empty.
 * For example, if arch_max_dma_pfn == arch_max_dma32_pfn, it is assumed
 * that arch_max_dma32_pfn has no pages. It is also assumed that a zone
 * starts where the previous one ended. For example, ZONE_DMA32 starts
 * at arch_max_dma_pfn.
 */
void __init free_area_init_nodes(unsigned long *max_zone_pfn)
{
	unsigned long nid;
	enum zone_type i;

	/* Record where the zone boundaries are */
	memset(arch_zone_lowest_possible_pfn, 0,
				sizeof(arch_zone_lowest_possible_pfn));
	memset(arch_zone_highest_possible_pfn, 0,
				sizeof(arch_zone_highest_possible_pfn));
	arch_zone_lowest_possible_pfn[0] = find_min_pfn_with_active_regions();
	arch_zone_highest_possible_pfn[0] = max_zone_pfn[0];
	for (i = 1; i < MAX_NR_ZONES; i++) {
		arch_zone_lowest_possible_pfn[i] =
			arch_zone_highest_possible_pfn[i-1];
		arch_zone_highest_possible_pfn[i] =
			max(max_zone_pfn[i], arch_zone_lowest_possible_pfn[i]);
	}

	/* Print out the zone ranges */
	printk("Zone PFN ranges:\n");
	for (i = 0; i < MAX_NR_ZONES; i++)
		printk("  %-8s %8lu -> %8lu\n",
				zone_names[i],
				arch_zone_lowest_possible_pfn[i],
				arch_zone_highest_possible_pfn[i]);

	/* Print out the early_node_map[] */
	printk("early_node_map[%d] active PFN ranges\n", nr_nodemap_entries);
	for (i = 0; i < nr_nodemap_entries; i++)
		printk("  %3d: %8lu -> %8lu\n", early_node_map[i].nid,
						early_node_map[i].start_pfn,
						early_node_map[i].end_pfn);

	/* Initialise every node */
	for_each_online_node(nid) {
		pg_data_t *pgdat = NODE_DATA(nid);
		free_area_init_node(nid, pgdat, NULL,
				find_min_pfn_for_node(nid), NULL);
	}
}
#endif /* CONFIG_ARCH_POPULATES_NODE_MAP */

/**
 * set_dma_reserve - set the specified number of pages reserved in the first zone
 * @new_dma_reserve: The number of pages to mark reserved
 *
 * The per-cpu batchsize and zone watermarks are determined by present_pages.
 * In the DMA zone, a significant percentage may be consumed by kernel image
 * and other unfreeable allocations which can skew the watermarks badly. This
 * function may optionally be used to account for unfreeable pages in the
 * first zone (e.g., ZONE_DMA). The effect will be lower watermarks and
 * smaller per-cpu batchsize.
 */
void __init set_dma_reserve(unsigned long new_dma_reserve)
{
	dma_reserve = new_dma_reserve;
}

#ifndef CONFIG_NEED_MULTIPLE_NODES
static bootmem_data_t contig_bootmem_data;
struct pglist_data contig_page_data = { .bdata = &contig_bootmem_data };

EXPORT_SYMBOL(contig_page_data);
#endif

void __init free_area_init(unsigned long *zones_size)
{
	free_area_init_node(0, NODE_DATA(0), zones_size,
			__pa(PAGE_OFFSET) >> PAGE_SHIFT, NULL);
}

#ifdef CONFIG_HOTPLUG_CPU
static int page_alloc_cpu_notify(struct notifier_block *self,
				 unsigned long action, void *hcpu)
{
	int cpu = (unsigned long)hcpu;

	if (action == CPU_DEAD) {
		local_irq_disable();
		__drain_pages(cpu);
		vm_events_fold_cpu(cpu);
		local_irq_enable();
		refresh_cpu_vm_stats(cpu);
	}
	return NOTIFY_OK;
}
#endif /* CONFIG_HOTPLUG_CPU */

void __init page_alloc_init(void)
{
	hotcpu_notifier(page_alloc_cpu_notify, 0);
}

/*
 * calculate_totalreserve_pages - called when sysctl_lower_zone_reserve_ratio
 *	or min_free_kbytes changes.
 */
static void calculate_totalreserve_pages(void)
{
	struct pglist_data *pgdat;
	unsigned long reserve_pages = 0;
	enum zone_type i, j;

	for_each_online_pgdat(pgdat) {
		for (i = 0; i < MAX_NR_ZONES; i++) {
			struct zone *zone = pgdat->node_zones + i;
			unsigned long max = 0;

			/* Find valid and maximum lowmem_reserve in the zone */
			for (j = i; j < MAX_NR_ZONES; j++) {
				if (zone->lowmem_reserve[j] > max)
					max = zone->lowmem_reserve[j];
			}

			/* we treat pages_high as reserved pages. */
			max += zone->pages_high;

			if (max > zone->present_pages)
				max = zone->present_pages;
			reserve_pages += max;
		}
	}
	totalreserve_pages = reserve_pages;
}

/*
 * setup_per_zone_lowmem_reserve - called whenever
 *	sysctl_lower_zone_reserve_ratio changes.  Ensures that each zone
 *	has a correct pages reserved value, so an adequate number of
 *	pages are left in the zone after a successful __alloc_pages().
 */
static void setup_per_zone_lowmem_reserve(void)
{
	struct pglist_data *pgdat;
	enum zone_type j, idx;

	for_each_online_pgdat(pgdat) {
		for (j = 0; j < MAX_NR_ZONES; j++) {
			struct zone *zone = pgdat->node_zones + j;
			unsigned long present_pages = zone->present_pages;

			zone->lowmem_reserve[j] = 0;

			idx = j;
			while (idx) {
				struct zone *lower_zone;

				idx--;

				if (sysctl_lowmem_reserve_ratio[idx] < 1)
					sysctl_lowmem_reserve_ratio[idx] = 1;

				lower_zone = pgdat->node_zones + idx;
				lower_zone->lowmem_reserve[j] = present_pages /
					sysctl_lowmem_reserve_ratio[idx];
				present_pages += lower_zone->present_pages;
			}
		}
	}

	/* update totalreserve_pages */
	calculate_totalreserve_pages();
}

/**
 * setup_per_zone_pages_min - called when min_free_kbytes changes.
 *
 * Ensures that the pages_{min,low,high} values for each zone are set correctly
 * with respect to min_free_kbytes.
 */
void setup_per_zone_pages_min(void)
{
	unsigned long pages_min = min_free_kbytes >> (PAGE_SHIFT - 10);
	unsigned long lowmem_pages = 0;
	struct zone *zone;
	unsigned long flags;

	/* Calculate total number of !ZONE_HIGHMEM pages */
	for_each_zone(zone) {
		if (!is_highmem(zone))
			lowmem_pages += zone->present_pages;
	}

	for_each_zone(zone) {
		u64 tmp;

		spin_lock_irqsave(&zone->lru_lock, flags);
		tmp = (u64)pages_min * zone->present_pages;
		do_div(tmp, lowmem_pages);
		if (is_highmem(zone)) {
			/*
			 * __GFP_HIGH and PF_MEMALLOC allocations usually don't
			 * need highmem pages, so cap pages_min to a small
			 * value here.
			 *
			 * The (pages_high-pages_low) and (pages_low-pages_min)
			 * deltas controls asynch page reclaim, and so should
			 * not be capped for highmem.
			 */
			int min_pages;

			min_pages = zone->present_pages / 1024;
			if (min_pages < SWAP_CLUSTER_MAX)
				min_pages = SWAP_CLUSTER_MAX;
			if (min_pages > 128)
				min_pages = 128;
			zone->pages_min = min_pages;
		} else {
			/*
			 * If it's a lowmem zone, reserve a number of pages
			 * proportionate to the zone's size.
			 */
			zone->pages_min = tmp;
		}

		zone->pages_low   = zone->pages_min + (tmp >> 2);
		zone->pages_high  = zone->pages_min + (tmp >> 1);
		spin_unlock_irqrestore(&zone->lru_lock, flags);
	}

	/* update totalreserve_pages */
	calculate_totalreserve_pages();
}

/*
 * Initialise min_free_kbytes.
 *
 * For small machines we want it small (128k min).  For large machines
 * we want it large (64MB max).  But it is not linear, because network
 * bandwidth does not increase linearly with machine size.  We use
 *
 * 	min_free_kbytes = 4 * sqrt(lowmem_kbytes), for better accuracy:
 *	min_free_kbytes = sqrt(lowmem_kbytes * 16)
 *
 * which yields
 *
 * 16MB:	512k
 * 32MB:	724k
 * 64MB:	1024k
 * 128MB:	1448k
 * 256MB:	2048k
 * 512MB:	2896k
 * 1024MB:	4096k
 * 2048MB:	5792k
 * 4096MB:	8192k
 * 8192MB:	11584k
 * 16384MB:	16384k
 */
static int __init init_per_zone_pages_min(void)
{
	unsigned long lowmem_kbytes;

	lowmem_kbytes = nr_free_buffer_pages() * (PAGE_SIZE >> 10);

	min_free_kbytes = int_sqrt(lowmem_kbytes * 16);
	if (min_free_kbytes < 128)
		min_free_kbytes = 128;
	if (min_free_kbytes > 65536)
		min_free_kbytes = 65536;
	setup_per_zone_pages_min();
	setup_per_zone_lowmem_reserve();
	return 0;
}
module_init(init_per_zone_pages_min)

/*
 * min_free_kbytes_sysctl_handler - just a wrapper around proc_dointvec() so 
 *	that we can call two helper functions whenever min_free_kbytes
 *	changes.
 */
int min_free_kbytes_sysctl_handler(ctl_table *table, int write, 
	struct file *file, void __user *buffer, size_t *length, loff_t *ppos)
{
	proc_dointvec(table, write, file, buffer, length, ppos);
	setup_per_zone_pages_min();
	return 0;
}

#ifdef CONFIG_NUMA
int sysctl_min_unmapped_ratio_sysctl_handler(ctl_table *table, int write,
	struct file *file, void __user *buffer, size_t *length, loff_t *ppos)
{
	struct zone *zone;
	int rc;

	rc = proc_dointvec_minmax(table, write, file, buffer, length, ppos);
	if (rc)
		return rc;

	for_each_zone(zone)
		zone->min_unmapped_pages = (zone->present_pages *
				sysctl_min_unmapped_ratio) / 100;
	return 0;
}

int sysctl_min_slab_ratio_sysctl_handler(ctl_table *table, int write,
	struct file *file, void __user *buffer, size_t *length, loff_t *ppos)
{
	struct zone *zone;
	int rc;

	rc = proc_dointvec_minmax(table, write, file, buffer, length, ppos);
	if (rc)
		return rc;

	for_each_zone(zone)
		zone->min_slab_pages = (zone->present_pages *
				sysctl_min_slab_ratio) / 100;
	return 0;
}
#endif

/*
 * lowmem_reserve_ratio_sysctl_handler - just a wrapper around
 *	proc_dointvec() so that we can call setup_per_zone_lowmem_reserve()
 *	whenever sysctl_lowmem_reserve_ratio changes.
 *
 * The reserve ratio obviously has absolutely no relation with the
 * pages_min watermarks. The lowmem reserve ratio can only make sense
 * if in function of the boot time zone sizes.
 */
int lowmem_reserve_ratio_sysctl_handler(ctl_table *table, int write,
	struct file *file, void __user *buffer, size_t *length, loff_t *ppos)
{
	proc_dointvec_minmax(table, write, file, buffer, length, ppos);
	setup_per_zone_lowmem_reserve();
	return 0;
}

/*
 * percpu_pagelist_fraction - changes the pcp->high for each zone on each
 * cpu.  It is the fraction of total pages in each zone that a hot per cpu pagelist
 * can have before it gets flushed back to buddy allocator.
 */

int percpu_pagelist_fraction_sysctl_handler(ctl_table *table, int write,
	struct file *file, void __user *buffer, size_t *length, loff_t *ppos)
{
	struct zone *zone;
	unsigned int cpu;
	int ret;

	ret = proc_dointvec_minmax(table, write, file, buffer, length, ppos);
	if (!write || (ret == -EINVAL))
		return ret;
	for_each_zone(zone) {
		for_each_online_cpu(cpu) {
			unsigned long  high;
			high = zone->present_pages / percpu_pagelist_fraction;
			setup_pagelist_highmark(zone_pcp(zone, cpu), high);
		}
	}
	return 0;
}

int hashdist = HASHDIST_DEFAULT;

#ifdef CONFIG_NUMA
static int __init set_hashdist(char *str)
{
	if (!str)
		return 0;
	hashdist = simple_strtoul(str, &str, 0);
	return 1;
}
__setup("hashdist=", set_hashdist);
#endif

/*
 * allocate a large system hash table from bootmem
 * - it is assumed that the hash table must contain an exact power-of-2
 *   quantity of entries
 * - limit is the number of hash buckets, not the total allocation size
 */
void *__init alloc_large_system_hash(const char *tablename,
				     unsigned long bucketsize,
				     unsigned long numentries,
				     int scale,
				     int flags,
				     unsigned int *_hash_shift,
				     unsigned int *_hash_mask,
				     unsigned long limit)
{
	unsigned long long max = limit;
	unsigned long log2qty, size;
	void *table = NULL;

	/* allow the kernel cmdline to have a say */
	if (!numentries) {
		/* round applicable memory size up to nearest megabyte */
		numentries = (flags & HASH_HIGHMEM) ? nr_all_pages : nr_kernel_pages;
		numentries += (1UL << (20 - PAGE_SHIFT)) - 1;
		numentries >>= 20 - PAGE_SHIFT;
		numentries <<= 20 - PAGE_SHIFT;

		/* limit to 1 bucket per 2^scale bytes of low memory */
		if (scale > PAGE_SHIFT)
			numentries >>= (scale - PAGE_SHIFT);
		else
			numentries <<= (PAGE_SHIFT - scale);
	}
	numentries = roundup_pow_of_two(numentries);

	/* limit allocation size to 1/16 total memory by default */
	if (max == 0) {
		max = ((unsigned long long)nr_all_pages << PAGE_SHIFT) >> 4;
		do_div(max, bucketsize);
	}

	if (numentries > max)
		numentries = max;

	/*
	 * we will allocate at least a page (even on low memory systems)
	 * so do a fixup here to ensure we utilise the space that will be
	 * allocated,  this also prevents us reporting -ve orders
	 */
	if (bucketsize * numentries < PAGE_SIZE)
		numentries = (PAGE_SIZE + bucketsize - 1) / bucketsize;

	log2qty = long_log2(numentries);

	do {
		size = bucketsize << log2qty;
		if (flags & HASH_EARLY)
			table = alloc_bootmem(size);
		else if (hashdist)
			table = __vmalloc(size, GFP_ATOMIC, PAGE_KERNEL);
		else {
			unsigned long order;
			for (order = 0; ((1UL << order) << PAGE_SHIFT) < size; order++)
				;
			table = (void*) __get_free_pages(GFP_ATOMIC, order);
		}
	} while (!table && size > PAGE_SIZE && --log2qty);

	if (!table)
		panic("Failed to allocate %s hash table\n", tablename);

	printk("%s hash table entries: %d (order: %d, %lu bytes)\n",
	       tablename,
	       (1U << log2qty),
	       long_log2(size) - PAGE_SHIFT,
	       size);

	if (_hash_shift)
		*_hash_shift = log2qty;
	if (_hash_mask)
		*_hash_mask = (1 << log2qty) - 1;

	return table;
}

#ifdef CONFIG_OUT_OF_LINE_PFN_TO_PAGE
struct page *pfn_to_page(unsigned long pfn)
{
	return __pfn_to_page(pfn);
}
unsigned long page_to_pfn(struct page *page)
{
	return __page_to_pfn(page);
}
EXPORT_SYMBOL(pfn_to_page);
EXPORT_SYMBOL(page_to_pfn);
#endif /* CONFIG_OUT_OF_LINE_PFN_TO_PAGE */

#if MAX_NUMNODES > 1
/*
 * Find the highest possible node id.
 */
int highest_possible_node_id(void)
{
	unsigned int node;
	unsigned int highest = 0;

	for_each_node_mask(node, node_possible_map)
		highest = node;
	return highest;
}
EXPORT_SYMBOL(highest_possible_node_id);
#endif
