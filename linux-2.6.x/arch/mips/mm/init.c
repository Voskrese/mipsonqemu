/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994 - 2000 Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 * Kevin D. Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000 MIPS Technologies, Inc.  All rights reserved.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/pagemap.h>
#include <linux/ptrace.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/highmem.h>
#include <linux/swap.h>
#include <linux/proc_fs.h>
#include <linux/pfn.h>

#include <asm/bootinfo.h>
#include <asm/cachectl.h>
#include <asm/cpu.h>
#include <asm/dma.h>
#include <asm/kmap_types.h>
#include <asm/mmu_context.h>
#include <asm/sections.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/tlb.h>
#include <asm/fixmap.h>

/* Atomicity and interruptability */
#ifdef CONFIG_MIPS_MT_SMTC

#include <asm/mipsmtregs.h>

#define ENTER_CRITICAL(flags) \
	{ \
	unsigned int mvpflags; \
	local_irq_save(flags);\
	mvpflags = dvpe()
#define EXIT_CRITICAL(flags) \
	evpe(mvpflags); \
	local_irq_restore(flags); \
	}
#else

#define ENTER_CRITICAL(flags) local_irq_save(flags)
#define EXIT_CRITICAL(flags) local_irq_restore(flags)

#endif /* CONFIG_MIPS_MT_SMTC */

DEFINE_PER_CPU(struct mmu_gather, mmu_gathers);

unsigned long highstart_pfn, highend_pfn;

/*
 * We have up to 8 empty zeroed pages so we can map one of the right colour
 * when needed.  This is necessary only on R4000 / R4400 SC and MC versions
 * where we have to avoid VCED / VECI exceptions for good performance at
 * any price.  Since page is never written to after the initialization we
 * don't have to care about aliases on other CPUs.
 */
unsigned long empty_zero_page, zero_page_mask;

/*
 * Not static inline because used by IP27 special magic initialization code
 */
unsigned long setup_zero_pages(void)
{
	unsigned int order;
	unsigned long size;
	struct page *page;

	if (cpu_has_vce)
		order = 3;
	else
		order = 0;

	empty_zero_page = __get_free_pages(GFP_KERNEL | __GFP_ZERO, order);
	if (!empty_zero_page)
		panic("Oh boy, that early out of memory?");

	page = virt_to_page(empty_zero_page);
	split_page(page, order);
	while (page < virt_to_page(empty_zero_page + (PAGE_SIZE << order))) {
		SetPageReserved(page);
		page++;
	}

	size = PAGE_SIZE << order;
	zero_page_mask = (size - 1) & PAGE_MASK;

	return 1UL << order;
}

/*
 * These are almost like kmap_atomic / kunmap_atmic except they take an
 * additional address argument as the hint.
 */

#define kmap_get_fixmap_pte(vaddr)					\
	pte_offset_kernel(pmd_offset(pud_offset(pgd_offset_k(vaddr), (vaddr)), (vaddr)), (vaddr))

#ifdef CONFIG_MIPS_MT_SMTC
static pte_t *kmap_coherent_pte;
static void __init kmap_coherent_init(void)
{
	unsigned long vaddr;

	/* cache the first coherent kmap pte */
	vaddr = __fix_to_virt(FIX_CMAP_BEGIN);
	kmap_coherent_pte = kmap_get_fixmap_pte(vaddr);
}
#else
static inline void kmap_coherent_init(void) {}
#endif

static inline void *kmap_coherent(struct page *page, unsigned long addr)
{
	enum fixed_addresses idx;
	unsigned long vaddr, flags, entrylo;
	unsigned long old_ctx;
	pte_t pte;
	int tlbidx;

	inc_preempt_count();
	idx = (addr >> PAGE_SHIFT) & (FIX_N_COLOURS - 1);
#ifdef CONFIG_MIPS_MT_SMTC
	idx += FIX_N_COLOURS * smp_processor_id();
#endif
	vaddr = __fix_to_virt(FIX_CMAP_END - idx);
	pte = mk_pte(page, PAGE_KERNEL);
#if defined(CONFIG_64BIT_PHYS_ADDR) && defined(CONFIG_CPU_MIPS32_R1)
	entrylo = pte.pte_high;
#else
	entrylo = pte_val(pte) >> 6;
#endif

	ENTER_CRITICAL(flags);
	old_ctx = read_c0_entryhi();
	write_c0_entryhi(vaddr & (PAGE_MASK << 1));
	write_c0_entrylo0(entrylo);
	write_c0_entrylo1(entrylo);
#ifdef CONFIG_MIPS_MT_SMTC
	set_pte(kmap_coherent_pte - (FIX_CMAP_END - idx), pte);
	/* preload TLB instead of local_flush_tlb_one() */
	mtc0_tlbw_hazard();
	tlb_probe();
	tlb_probe_hazard();
	tlbidx = read_c0_index();
	mtc0_tlbw_hazard();
	if (tlbidx < 0)
		tlb_write_random();
	else
		tlb_write_indexed();
#else
	tlbidx = read_c0_wired();
	write_c0_wired(tlbidx + 1);
	write_c0_index(tlbidx);
	mtc0_tlbw_hazard();
	tlb_write_indexed();
#endif
	tlbw_use_hazard();
	write_c0_entryhi(old_ctx);
	EXIT_CRITICAL(flags);

	return (void*) vaddr;
}

#define UNIQUE_ENTRYHI(idx) (CKSEG0 + ((idx) << (PAGE_SHIFT + 1)))

static inline void kunmap_coherent(struct page *page)
{
#ifndef CONFIG_MIPS_MT_SMTC
	unsigned int wired;
	unsigned long flags, old_ctx;

	ENTER_CRITICAL(flags);
	old_ctx = read_c0_entryhi();
	wired = read_c0_wired() - 1;
	write_c0_wired(wired);
	write_c0_index(wired);
	write_c0_entryhi(UNIQUE_ENTRYHI(wired));
	write_c0_entrylo0(0);
	write_c0_entrylo1(0);
	mtc0_tlbw_hazard();
	tlb_write_indexed();
	tlbw_use_hazard();
	write_c0_entryhi(old_ctx);
	EXIT_CRITICAL(flags);
#endif
	dec_preempt_count();
	preempt_check_resched();
}

void copy_to_user_page(struct vm_area_struct *vma,
	struct page *page, unsigned long vaddr, void *dst, const void *src,
	unsigned long len)
{
	if (cpu_has_dc_aliases) {
		void *vto = kmap_coherent(page, vaddr) + (vaddr & ~PAGE_MASK);
		memcpy(vto, src, len);
		kunmap_coherent(page);
	} else
		memcpy(dst, src, len);
	if ((vma->vm_flags & VM_EXEC) && !cpu_has_ic_fills_f_dc)
		flush_cache_page(vma, vaddr, page_to_pfn(page));
}

EXPORT_SYMBOL(copy_to_user_page);

void copy_from_user_page(struct vm_area_struct *vma,
	struct page *page, unsigned long vaddr, void *dst, const void *src,
	unsigned long len)
{
	if (cpu_has_dc_aliases) {
		void *vfrom =
			kmap_coherent(page, vaddr) + (vaddr & ~PAGE_MASK);
		memcpy(dst, vfrom, len);
		kunmap_coherent(page);
	} else
		memcpy(dst, src, len);
}

EXPORT_SYMBOL(copy_from_user_page);


#ifdef CONFIG_HIGHMEM
pte_t *kmap_pte;
pgprot_t kmap_prot;

static void __init kmap_init(void)
{
	unsigned long kmap_vstart;

	/* cache the first kmap pte */
	kmap_vstart = __fix_to_virt(FIX_KMAP_BEGIN);
	kmap_pte = kmap_get_fixmap_pte(kmap_vstart);

	kmap_prot = PAGE_KERNEL;
}
#endif /* CONFIG_HIGHMEM */

void __init fixrange_init(unsigned long start, unsigned long end,
	pgd_t *pgd_base)
{
#if defined(CONFIG_HIGHMEM) || defined(CONFIG_MIPS_MT_SMTC)
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	int i, j, k;
	unsigned long vaddr;

	vaddr = start;
	i = __pgd_offset(vaddr);
	j = __pud_offset(vaddr);
	k = __pmd_offset(vaddr);
	pgd = pgd_base + i;

	for ( ; (i < PTRS_PER_PGD) && (vaddr != end); pgd++, i++) {
		pud = (pud_t *)pgd;
		for ( ; (j < PTRS_PER_PUD) && (vaddr != end); pud++, j++) {
			pmd = (pmd_t *)pud;
			for (; (k < PTRS_PER_PMD) && (vaddr != end); pmd++, k++) {
				if (pmd_none(*pmd)) {
					pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);
					set_pmd(pmd, __pmd((unsigned long)pte));
					if (pte != pte_offset_kernel(pmd, 0))
						BUG();
				}
				vaddr += PMD_SIZE;
			}
			k = 0;
		}
		j = 0;
	}
#endif
}

#ifndef CONFIG_NEED_MULTIPLE_NODES
extern void pagetable_init(void);

static int __init page_is_ram(unsigned long pagenr)
{
	int i;

	for (i = 0; i < boot_mem_map.nr_map; i++) {
		unsigned long addr, end;

		if (boot_mem_map.map[i].type != BOOT_MEM_RAM)
			/* not usable memory */
			continue;

		addr = PFN_UP(boot_mem_map.map[i].addr);
		end = PFN_DOWN(boot_mem_map.map[i].addr +
			       boot_mem_map.map[i].size);

		if (pagenr >= addr && pagenr < end)
			return 1;
	}

	return 0;
}

void __init paging_init(void)
{
	unsigned long zones_size[MAX_NR_ZONES] = { 0, };
	unsigned long max_dma, high, low;
#ifndef CONFIG_FLATMEM
	unsigned long zholes_size[MAX_NR_ZONES] = { 0, };
	unsigned long i, j, pfn;
#endif

	pagetable_init();

#ifdef CONFIG_HIGHMEM
	kmap_init();
#endif
	kmap_coherent_init();

	max_dma = virt_to_phys((char *)MAX_DMA_ADDRESS) >> PAGE_SHIFT;
	low = max_low_pfn;
	high = highend_pfn;
	printk("%s max_low_pfn=%d highend_pfn=%d\n",__FUNCTION__,max_low_pfn,highend_pfn);
#ifdef CONFIG_ISA
	if (low < max_dma)
		zones_size[ZONE_DMA] = low;
	else {
		zones_size[ZONE_DMA] = max_dma;
		zones_size[ZONE_NORMAL] = low - max_dma;
	}
#else
	zones_size[ZONE_DMA] = low;
#endif
#ifdef CONFIG_HIGHMEM
	if (cpu_has_dc_aliases) {
		printk(KERN_WARNING "This processor doesn't support highmem.");
		if (high - low)
			printk(" %ldk highmem ignored", high - low);
		printk("\n");
	} else
		zones_size[ZONE_HIGHMEM] = high - low;
#endif

#ifdef CONFIG_FLATMEM
	free_area_init(zones_size);
#else
	pfn = 0;
	for (i = 0; i < MAX_NR_ZONES; i++)
		for (j = 0; j < zones_size[i]; j++, pfn++)
			if (!page_is_ram(pfn))
				zholes_size[i]++;
	free_area_init_node(0, NODE_DATA(0), zones_size, 0, zholes_size);
#endif
}

static struct kcore_list kcore_mem, kcore_vmalloc;
#ifdef CONFIG_64BIT
static struct kcore_list kcore_kseg0;
#endif

void __init mem_init(void)
{
	unsigned long codesize, reservedpages, datasize, initsize;
	unsigned long tmp, ram;

#ifdef CONFIG_HIGHMEM
#ifdef CONFIG_DISCONTIGMEM
#error "CONFIG_HIGHMEM and CONFIG_DISCONTIGMEM dont work together yet"
#endif
	max_mapnr = highend_pfn;
#else
	max_mapnr = max_low_pfn;
#endif
	high_memory = (void *) __va(max_low_pfn << PAGE_SHIFT);

	totalram_pages += free_all_bootmem();
	printk("%s totalram_pages=%d\n",__FUNCTION__,totalram_pages);
	totalram_pages -= setup_zero_pages();	/* Setup zeroed pages.  */
	printk("%s totalram_pages=%d\n",__FUNCTION__,totalram_pages);
	reservedpages = ram = 0;
	for (tmp = 0; tmp < max_low_pfn; tmp++)
		if (page_is_ram(tmp)) {
			ram++;
			if (PageReserved(pfn_to_page(tmp)))
				reservedpages++;
		}
	num_physpages = ram;
	printk("%s num_physpages=%d reservedpages=%d\n",__FUNCTION__,num_physpages,reservedpages);

#ifdef CONFIG_HIGHMEM
	for (tmp = highstart_pfn; tmp < highend_pfn; tmp++) {
		struct page *page = mem_map + tmp;

		if (!page_is_ram(tmp)) {
			SetPageReserved(page);
			continue;
		}
		ClearPageReserved(page);
#ifdef CONFIG_LIMITED_DMA
		set_page_address(page, lowmem_page_address(page));
#endif
		init_page_count(page);
		__free_page(page);
		totalhigh_pages++;
	}
	totalram_pages += totalhigh_pages;
	num_physpages += totalhigh_pages;
#endif

	codesize =  (unsigned long) &_etext - (unsigned long) &_text;
	datasize =  (unsigned long) &_edata - (unsigned long) &_etext;
	initsize =  (unsigned long) &__init_end - (unsigned long) &__init_begin;

#ifdef CONFIG_64BIT
	if ((unsigned long) &_text > (unsigned long) CKSEG0)
		/* The -4 is a hack so that user tools don't have to handle
		   the overflow.  */
		kclist_add(&kcore_kseg0, (void *) CKSEG0, 0x80000000 - 4);
#endif
	kclist_add(&kcore_mem, __va(0), max_low_pfn << PAGE_SHIFT);
	kclist_add(&kcore_vmalloc, (void *)VMALLOC_START,
		   VMALLOC_END-VMALLOC_START);

	printk(KERN_INFO "Memory: %luk/%luk available (%ldk kernel code, "
	       "%ldk reserved, %ldk data, %ldk init, %ldk highmem)\n",
	       (unsigned long) nr_free_pages() << (PAGE_SHIFT-10),
	       ram << (PAGE_SHIFT-10),
	       codesize >> 10,
	       reservedpages << (PAGE_SHIFT-10),
	       datasize >> 10,
	       initsize >> 10,
	       (unsigned long) (totalhigh_pages << (PAGE_SHIFT-10)));
}
#endif /* !CONFIG_NEED_MULTIPLE_NODES */

void free_init_pages(char *what, unsigned long begin, unsigned long end)
{
	unsigned long addr;

	for (addr = begin; addr < end; addr += PAGE_SIZE) {
		ClearPageReserved(virt_to_page(addr));
		init_page_count(virt_to_page(addr));
		memset((void *)addr, 0xcc, PAGE_SIZE);
		free_page(addr);
		totalram_pages++;
	}
	printk(KERN_INFO "Freeing %s: %ldk freed\n", what, (end - begin) >> 10);
}

#ifdef CONFIG_BLK_DEV_INITRD
void free_initrd_mem(unsigned long start, unsigned long end)
{
#ifdef CONFIG_64BIT
	/* Switch from KSEG0 to XKPHYS addresses */
	start = (unsigned long)phys_to_virt(CPHYSADDR(start));
	end = (unsigned long)phys_to_virt(CPHYSADDR(end));
#endif
	free_init_pages("initrd memory", start, end);
}
#endif

extern unsigned long prom_free_prom_memory(void);

void free_initmem(void)
{
	unsigned long start, end, freed;

	freed = prom_free_prom_memory();
	if (freed)
		printk(KERN_INFO "Freeing firmware memory: %ldk freed\n",freed);

	start = (unsigned long)(&__init_begin);
	end = (unsigned long)(&__init_end);
#ifdef CONFIG_64BIT
	start = PAGE_OFFSET | CPHYSADDR(start);
	end = PAGE_OFFSET | CPHYSADDR(end);
#endif
	free_init_pages("unused kernel memory", start, end);
}
