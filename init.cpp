#include <cstdint>
#include "include/ptr.h"
#include "include/page_entry.h"
#include "include/gdt.h"
#include "include/boot_info.h"
#include "include/sysdefs.h"
#include "include/assembly.h"

extern ptr_t upper_border;
extern ptr_t lower_border;
extern uint8_t _kernel_end;




[[gnu::aligned(page_size)]]
struct {
    uint8_t stack[kib(16)];
    uint8_t stack_top;
} stack;

[[gnu::aligned(page_size)]]
page_entry boot_page_directory[page_table_size];

[[gnu::aligned(page_size)]]
page_entry page_table_catalog[page_table_size]; // TODO: rename to page_catalog

[[gnu::aligned(page_size)]]
uint8_t vga_console[pages(1)];

[[gnu::section(".multiboot.text")]]
void relocate_kernel();

void unmap_lower_pages();

void kernel_main(boot_info_t* boot_info); // TODO: from header?

[[noreturn]]
void relocated_code();

[[noreturn, gnu::section(".multiboot.text")]]
void init_code();


extern "C"
[[noreturn, gnu::naked, gnu::section(".multiboot.text")]]
void _start() {
    assembly::const_set_esp(bootstrap_address(&stack.stack_top));

    // Jump to non-naked function
    assembly::const_jump(init_code);
}


[[noreturn, gnu::section(".multiboot.text")]]
void init_code() {
    // save boot info from bootloader
    auto boot_info = assembly::get_ebx<boot_info_t*>();

    relocate_kernel();

    assembly::set_cr3(bootstrap_address(boot_page_directory));

    assembly::set_cr0(assembly::get_cr0() | (assembly::cr0::write_protect | assembly::cr0::paging));

    assembly::const_set_esp(&stack.stack_top);

    assembly::memory_set_ebx(relocated_address(boot_info)); // TODO: allow to optimize?

    assembly::register_jump(relocated_code);
}

[[noreturn]]
void relocated_code() {
    auto boot_info = assembly::get_ebx<boot_info_t*>();

    unmap_lower_pages();

    // TLB flush
    // TODO: as function like "tlb_flush"
    assembly::set_cr3(assembly::get_cr3());

    gdt_init();

    kernel_main(boot_info);

    // Infinite loop
    // TODO: call panic?
    assembly::clear_interrupts();
    while (true)
        assembly::halt();
}

[[gnu::section(".multiboot.text")]]
void relocate_kernel() {
    auto _boot_page_directory = bootstrap_address(boot_page_directory);
    auto _page_table_catalog = bootstrap_address(page_table_catalog);
    auto& _upper_border = *bootstrap_address(&upper_border);
    auto& _lower_border = *bootstrap_address(&lower_border);

    auto cat_dir_index = page_directory_index(page_catalog_base);
    auto& directory = _boot_page_directory[cat_dir_index];
    directory.set_address(_page_table_catalog);
    directory.writable = true;
    directory.present = true;

    for (ptr_t i = 0UL; i < bootstrap_address(_lower_border); i += pages(page_table_size)) {
        page_entry* new_page_table = bootstrap_address(_lower_border);
        _lower_border += page_size;

        auto dir_index_low = page_directory_index(i);
        auto dir_index_high = page_directory_index(relocated_address(i));

        auto& directory_low = _boot_page_directory[dir_index_low];
        directory_low.set_address(new_page_table);
        directory_low.present = true;
        directory_low.writable = true;

        auto& directory_high = _boot_page_directory[dir_index_high];
        directory_high.set_address(new_page_table);
        directory_high.present = true;
        directory_high.writable = true;

        auto& catalog_entry = _page_table_catalog[dir_index_high];
        catalog_entry.set_address(new_page_table);
        catalog_entry.writable = true;
        catalog_entry.present = true;

        for (ptr_t j = 0UL; j < pages(page_table_size) && i + j < bootstrap_address(_lower_border); j += page_size) {
            auto page_address = i + j;
            auto tbl_index = page_table_index(page_address);

            auto& page_table = new_page_table[tbl_index];
            auto target_address = page_address == bootstrap_address(vga_console)
                    ? vga_console_physical_address : page_address; // TODO: map vga console later
            page_table.set_address(target_address);
            page_table.writable = true;
            page_table.present = true;
        }
    }
}

void unmap_lower_pages() {
    for (ptr_t i = 0UL; i < bootstrap_address(lower_border); i += pages(page_table_size)) {
        auto dir_index = page_directory_index(i);
        boot_page_directory[dir_index].present = false;
    }
}
