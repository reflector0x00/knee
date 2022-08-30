#include <cstdint>
#include <vector>
#include <page_entry.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <bitset>
#include <array>
#include <map>
#include <set>
#include <allocator.h>
#include <ptr.h>
#include <linked_page_allocator.h>
#include <swapping_allocator.h>
#include <page_descriptor.h>
#include <tty_system.h>
//#include "throws.h"
#include <process.h>
#include "include/boot_info.h"
#include "include/sysdefs.h"
#include "include/assembly.h"

// TODO: from header?
extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];
extern page_entry boot_page_directory[1024];
extern page_entry page_table_catalog[1024];


typedef std::multimap<std::size_t, ptr_t, std::less<>, linked_paged_allocator<std::pair<const std::size_t, ptr_t>>> vs_mapping_t;
typedef std::map<ptr_t, page_descriptor, std::less<>, linked_paged_allocator<std::pair<const ptr_t, page_descriptor>>> alloc_mapping_t;
typedef std::vector<bool, swapping_allocator<bool>> frame_bitset_t;
typedef std::set<ptr_t, std::less<>, linked_paged_allocator<ptr_t>> pages_set_t;


// As we use ptr_t, "constinit" is required
constinit ptr_t upper_border = &_kernel_end;
constinit ptr_t lower_border = &_kernel_end;

// Temporary pre-main-allocator list
pages_set_t free_pages;

frame_bitset_t frames;
vs_mapping_t virtual_space;
alloc_mapping_t allocations; // Only kernel space

bool allocator_initialized = false;

void panic_on_use_before_allocator_init() {
    if (!allocator_initialized)
        panic("Use before allocator initialization");
}

void panic_on_use_after_allocator_init() {
    if (allocator_initialized)
        panic("Use after allocator initialization");
}

// TODO: review
void print_memory_mamppings() {
    bool cont = false;
    std::size_t last = -1;
    printk("Memory mappings:\n");
    for (std::size_t i = 0; i < 1024; ++i) {
        if (boot_page_directory[i].present) {
            bool any_page = false;
            page_entry* page_table = page_catalog_base + (i << 12);
            if (i << 22 == 0xFC000000)
                page_table = page_table_catalog;
            for (std::size_t j = 0; j < 1024; ++j) {
                if (page_table[j].present) {
                    any_page = true;
                    if (!cont) {
                        cont = true;
                        last = (i << 22) + (j << 12);
                        printk("%08X", (i << 22) + (j << 12));     
                    }
                }
                else if (cont) {
                    if (last != (i << 22) + ((j - 1) << 12))
                        printk("..%08X", (i << 22) + ((j - 1) << 12));     
                    printk("\n");
                    cont = false;
                }
            }
            if (!any_page) 
                printk("[%08X]", i << 22);
        }
    }
}


void print_virtual_space() {
    printk("Available virtual space:\n");
    for (auto& x : virtual_space)
        printk("%08X..%08X (%d pages)\n", x.second, x.second + (x.first << 12) - 1, x.first);
}

void print_allocations() {
    printk("Allocations:\n");
    auto cont = false;
    ptr_t first = nullptr;
    ptr_t last = nullptr;
    for (auto& x : allocations) {
        if (!cont) {
            printk("%08X", x.first);
            first = x.first;
            last = x.first;
            cont = true;
        }
        else {
            if (last + page_size != x.first) {
                if (first != last)
                    printk("..%08X", last);
                printk("\n");
                cont = false;
            }
            else
                last = x.first;
        }
    }

    if (cont) {
        if (first != last)
            printk("..%08X", last);
        printk("\n");
    }
}


ptr_t allocate_frame() {
    static std::size_t last_frame = 0;
    panic_on_use_before_allocator_init();

    for (std::size_t i = 0; i < frames.size(); ++i) {
        auto index = (last_frame + i) % frames.size();
        if (!frames[i]) {
            frames[i] = true;
            last_frame = index;
            return page_address(i);
        }
    }
    panic("Can't allocate new frame");
}

void deallocate_frame(ptr_t frame) {
    panic_on_use_before_allocator_init();

    auto index = page_index(frame);
    if (!frames[index])
        panic("Unallocated frame");
    
    frames[index] = false;
}

ptr_t allocate_virtual_space(std::size_t count) {
    panic_on_use_before_allocator_init();
    if (count == 0)
        panic("Undefined behaviour");

    auto page = virtual_space.upper_bound(count - 1); // upper bound, so minus one
    if (page == virtual_space.end())
        panic("Can't alllocate enough space");

    auto size = page->first;
    auto address = page->second;
    virtual_space.erase(page);
    if (size > count)
        virtual_space.emplace(size - count, address + page_size * count);

    return address;
}

std::pair<ptr_t, page_descriptor> saved; // TODO: different format
bool save_flag = false; // TODO: rename

void add_allocation(ptr_t from, ptr_t to, bool userspace) {
    if (save_flag) {
        save_flag = false; // not to cause recursion
        add_allocation(saved.first, saved.second.frame, false);
    }

    if (!allocations.get_allocator().get_lock()) {
        if (save_flag)
            panic("Not flushed");

        auto new_desc = page_descriptor{to};
        if (userspace)
            current_process->allocations.emplace(from, new_desc);
        else
            allocations.emplace(from, new_desc);
    }
    else {
        if (save_flag)
            panic("Failed to save");
        saved = std::make_pair(from, page_descriptor{to});
        save_flag = true;
    }
}

void memory_map(ptr_t page, ptr_t frame, bool userspace, bool mark) {
    panic_on_use_before_allocator_init();

    std::size_t dir_index = page_directory_index(page);

    auto offset = page_address(dir_index);
    page_entry* current_page_table = page_catalog_base + offset;

    page_entry& directory = boot_page_directory[dir_index];
    if (!directory.present) {
        if (!mark)
            panic("Directory not found");

        ptr_t new_page_table_addreess = allocate_frame();

        page_entry& page_table = page_table_catalog[dir_index];
        page_table.set_address(new_page_table_addreess);
        page_table.writable = true;
        page_table.present = true;

        directory.set_address(new_page_table_addreess);
        directory.writable = true;
        directory.present = true;
        directory.user = userspace;

        add_allocation(current_page_table, new_page_table_addreess, false);
    }
    else if (userspace && directory.user != userspace)
        panic("Try of change page directory permissions");

    std::size_t tbl_index = page_table_index(page);
    page_entry& table = current_page_table[tbl_index];
    if (table.present)
        panic("Page already mapped");

    table.set_address(frame);
    table.writable = true;
    table.present = true;
    table.user = userspace;

    if (mark)
        add_allocation(page, frame, userspace);
}

void memory_map(ptr_t page, ptr_t frame, bool userspace) {
    memory_map(page, frame, userspace, true);
}

void memory_unmap(ptr_t page, bool unmark) {
    panic_on_use_before_allocator_init();
    if (unmark)
        panic("TODO: unmark");

    std::size_t pd_index = page_directory_index(page);

    auto offset = page_address(pd_index);
    page_entry* current_page_table = page_catalog_base + offset;

    page_entry& directory = boot_page_directory[pd_index];
    if (!directory.present)
        panic("Not present directory");

    std::size_t pt_index = page_table_index(page);
    page_entry& table = current_page_table[pt_index];

    if (!table.present)
        panic("Not present page");

    table.present = false;
}


// TODO: rename or remove
ptr_t allocate_single_page() {
    panic_on_use_after_allocator_init();

    if (!free_pages.empty()) {
        auto iter = free_pages.begin();
        auto page = *iter;
        free_pages.erase(iter);
        return page;
    }

    auto page = lower_border;
    lower_border += page_size;

    // TODO: merge with memory_map
    // Do memory map
    auto dir_index = page_directory_index(page);
    auto &directory = boot_page_directory[dir_index];
    if (!directory.present) {
        ptr_t new_page_table_address = bootstrap_address(page);

        page_entry &page_table = page_table_catalog[dir_index];
        page_table.set_address(new_page_table_address);
        page_table.writable = true;
        page_table.present = true;

        page = lower_border;
        lower_border += page_size;

        directory.set_address(new_page_table_address);
        directory.writable = true;
        directory.present = true;
    }

    auto offset = page_address(dir_index);
    page_entry *new_page_table = page_catalog_base + offset;
    auto tbl_index = page_table_index(page);
    page_entry &table = new_page_table[tbl_index];
    if (!table.present) {
        table.set_address(bootstrap_address(page));
        table.writable = true;
        table.present = true;
    }

    return page;
}

// TODO: inline in deallocate_pages
void deallocate_single_page(ptr_t page) {
    panic_on_use_before_allocator_init();

    // TODO: move to memory_unmap
    auto entry = allocations.find(page);
    ptr_t frame;
    if (entry == allocations.end()) {
        entry = current_process->allocations.find(page);
        if (entry == current_process->allocations.end()) {
            panic("Unallocated page");
        }
        else {
            frame = entry->second.frame;
            current_process->allocations.erase(page);
        }
    }
    else {
        frame = entry->second.frame;
        allocations.erase(entry);
    }
    memory_unmap(page, false);
    deallocate_frame(frame);
    assembly::tlb_flush();
}

ptr_t allocate_pages(std::size_t n, bool userspace) {
    if (allocator_initialized) {
        ptr_t origin = allocate_virtual_space(n);
        for (std::size_t i = 0; i < n; ++i) {            
            ptr_t frame = allocate_frame();      
            memory_map(origin + i * page_size, frame, userspace);
        }        
        return origin;
    }
    else {
        ptr_t origin = allocate_single_page();
        for (std::size_t i = 1; i < n; ++i) {
            ptr_t next_page = allocate_single_page();
            if (origin + i * page_size != next_page) {
                
                for (std::size_t j = 0; j < i; ++j) 
                    free_pages.insert(origin + j * page_size);
                
                origin = next_page;
                i = 0; // increment
            }
        }
        return origin;
    }
}

ptr_t allocate_pages(ptr_t address, std::size_t n, bool userspace) {
    panic_on_use_before_allocator_init();

    // TODO: check in virtual space?
    for (std::size_t i = 0; i < n; ++i)
        memory_map(address + i * page_size, allocate_frame(), userspace);

    return address;
}

void deallocate_pages(ptr_t origin, std::size_t n) {
    if (allocator_initialized) {
        for (std::size_t i = 0; i < n; ++i)
            deallocate_single_page(origin + i * page_size);
    }
    else {
        for (std::size_t i = 0; i < n; ++i)
            free_pages.insert(origin + i * page_size);
    }
}

void deallocate_virtual_space(ptr_t page) {
    for (auto iter = virtual_space.begin(); iter != virtual_space.end(); ++iter) {
        auto size = iter->first;
        auto begin = iter->second;
        auto end = begin + pages(size) - 1;

        if (begin <= page && page <= end) {
            virtual_space.erase(iter);
            if (begin == page)
                virtual_space.emplace(size - 1, begin + page_size);
            else if (page + page_size == end)
                virtual_space.emplace(size - 1, begin);
            else {
                auto page_end = page + page_size;
                auto left_size = page_count(page - begin);
                auto right_size = page_count(end + 1 - page_end);

                virtual_space.emplace(left_size, begin);
                virtual_space.emplace(right_size, page_end);
            }
            return;
        }
    }
    panic("Unknown page");
}

void mark_page_as_allocated(ptr_t page) {
    auto frame = bootstrap_address(page);

    frames[page_index(frame)] = true;
    add_allocation(page, frame, false);
    deallocate_virtual_space(page);
}

void userspace_disable() {
    for (auto& iter : current_process->allocations)
        memory_unmap(iter.first, false);
    assembly::tlb_flush();
}

void userspace_enable() {
    for (auto& iter : current_process->allocations)
        memory_map(iter.first, iter.second.frame, true, false);
    assembly::tlb_flush();
}

ptr_t kernel_page_to_frame(ptr_t page) {
    auto iter = allocations.find(page);
    if (iter == allocations.end())
        panic("Unknown page");
    return iter->second.frame;
}

// TODO: rewrite as class
void allocator_init(boot_info_t* boot_info) {
    std::size_t available_pages = (boot_info->mem_lower + boot_info->mem_upper + 1) / (page_size / kib(1)) ;

    new(&frames) frame_bitset_t(available_pages, false);
    new(&virtual_space) vs_mapping_t();
    new(&allocations) alloc_mapping_t();
    new(&free_pages) pages_set_t();


    // Kernel address space
    auto beyond_page_catalog = page_catalog_base + page_catalog_size;
    virtual_space.emplace(page_count(size_between(kernel_base, page_catalog_base - 1)), kernel_base);
    virtual_space.emplace(page_count(size_between(beyond_page_catalog, address_space_end)), beyond_page_catalog);

    for (ptr_t page = kernel_base; page < &_kernel_end; page += page_size)
        mark_page_as_allocated(page);

    for (ptr_t page = upper_border; page < lower_border; page += page_size)
        if (free_pages.find(page) == free_pages.end())
            mark_page_as_allocated(page);

    allocator_initialized = true;

    print_memory_mamppings();
    print_virtual_space();
}