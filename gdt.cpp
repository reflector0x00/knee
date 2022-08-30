#include <gdt.h>
#include <assembly.h>


static struct [[gnu::packed]] {
    task_state_segment tss_entry{};
    uint8_t iomap[8192]{}; // TODO: magic number
} tss;





void set_tss_kernel_stack(uint32_t stack) { // TODO: ptr_t
    tss.tss_entry.esp0 = stack;
}

static segment_descriptor gdt_table[gdt_count];

void gdt_init() {
    gdt_table[index_zero] = segment_descriptor();
    gdt_table[index_kernel_code] = segment_descriptor(flat_base, flat_limit, access_kernel_code, flags_general);
    gdt_table[index_kernel_data] = segment_descriptor(flat_base, flat_limit, access_kernel_data, flags_general);
    gdt_table[index_user_code] = segment_descriptor(flat_base, flat_limit, access_user_code, flags_general);
    gdt_table[index_user_data] = segment_descriptor(flat_base, flat_limit, access_user_data, flags_general);
    gdt_table[index_task] = segment_descriptor(&tss.tss_entry, sizeof(tss), access_task, flags_task);

    tss.tss_entry.ss0 = kernel_ds;  // Set the kernel stack segment.
    tss.tss_entry.iomap_base = sizeof(task_state_segment);

    assembly::set_gdtr(sizeof(gdt_table) - 1, &gdt_table);

    assembly::set_cs(kernel_cs);
    assembly::register_set_ds(kernel_ds);
    assembly::register_set_es(kernel_ds);
    assembly::register_set_fs(kernel_ds);
    assembly::register_set_gs(kernel_ds);
    assembly::register_set_ss(kernel_ds);
    assembly::load_task_register(task_seg);
}