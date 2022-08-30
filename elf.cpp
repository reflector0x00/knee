#include <vector>
#include <elf.h>
#include <tty_system.h>
#include <process.h>
#include <gdt.h>
#include <ports.h>
#include <stack_operator.h>
#include <sysdefs.h>

void execute_elf(const std::string& path, const std::vector<std::string>& args, const std::vector<std::string>& envs, bool create_new_process) {
    // We use memory operations during this function,
    // so interrupts may cause bugs
    if (!create_new_process)
        assembly::clear_interrupts();

    // Simple check that we can use allocations of current process for new one creation
    if (!current_process->allocations.empty() && create_new_process)
        panic("Not empty");

    // Open target elf
    std::ifstream elf(path, std::ios_base::binary);
    if (!elf.is_open()) {
        panic("Can't open file");
    }

    // Read ELF header
    elf32_header header;
    elf.read(reinterpret_cast<char*>(&header), sizeof(elf32_header));

    // TODO: check header magic, type, machine, etc.

    // Read program headers
    auto program_headers = new elf32_program_header[header.e_phnum];
    elf.seekg(header.e_phoff);
    elf.read(reinterpret_cast<char*>(program_headers), sizeof(elf32_program_header) * header.e_phnum);

    // Copy allocations so we can iterate through them while deallocating
    auto allocations = current_process->allocations;
    for (auto iter : allocations)
        deallocate_pages(iter.first, 1);

    if (!current_process->allocations.empty())
        panic("Allocations aren't empty after deallocations");

    // Load program headers to memory
    ptr_t last_address = nullptr;
    for (std::size_t i = 0; i < header.e_phnum; ++i) {
        auto& program_header = program_headers[i];
        // Skip non-loadable segments
        if (program_header.p_type != PT_LOAD)
            continue;

        // Page address should be aligned
        ptr_t page_address = program_header.p_vaddr & ~(page_size - 1);

        // TODO: function for ceil alignment
        auto _page_count = page_count(program_header.p_memsz);
        if (program_header.p_memsz % page_size)
            ++_page_count;

        // Allocate necessary pages for current segment
        ptr_t pages = allocate_pages(page_address, _page_count, true);

        // And copy memory from ELF
        elf.seekg(program_header.p_offset);
        elf.read(ptr_t(program_header.p_vaddr), program_header.p_filesz);
        // Clear trailing memory (for ex: ".bss" section requires so)
        memset(ptr_t(program_header.p_vaddr) + program_header.p_filesz, 0, program_header.p_memsz - program_header.p_filesz);

        // Save last address for heap allocation
        last_address = program_header.p_vaddr + program_header.p_memsz;
    }

    // Init heap address
    auto heap_page = last_address & ~(page_size - 1);
    heap_page += page_size;

    // Allocate stack
    ptr_t stack_end = kernel_base - page_size;
    auto stack_page = stack_end & ~(page_size - 1);
    stack_page -= stack_page_count * page_size;
    allocate_pages(stack_page, stack_page_count, true);

    // Prepare arguments and environment
    std::list<ptr_t> args_ptr;
    std::list<ptr_t> envs_ptr;
    stack_operator stack(stack_end);

    if (args.empty())
        args_ptr.emplace_back(stack.push(path));
    else
        for (auto &arg: args)
            args_ptr.emplace_back(stack.push(arg));

    if (!envs.empty())
        for (auto &env: envs)
            envs_ptr.emplace_back(stack.push(env));

    // Prepare initial stack state for child process
    // envp
    stack.push(0);
    envs_ptr.reverse();
    for (auto& ptr : envs_ptr)
        stack.push(ptr);
    // argv
    stack.push(0);
    args_ptr.reverse();
    for (auto& ptr : args_ptr)
        stack.push(ptr);
    // argc
    auto startup_args = stack.push(args_ptr.size());

    userspace_disable();

    // TODO: RAII
    auto& process = create_new_process ? create_process(header.e_entry, startup_args, false) : *current_process;
    if (create_new_process) {
        process.allocations = std::move(current_process->allocations);
        // move-ctor leaves container in unspecified state, so we clear it
        current_process->allocations.clear();
    }
    else {
        process.instruction_pointer = header.e_entry;
        process.stack_pointer = startup_args;

        process.cs = user_cs;
        process.es = user_ds;
        process.ds = user_ds;
        process.fs = user_ds;
        process.gs = user_ds;
        process.ss = user_ds;
        // TODO: privileges check
    }
    process.heap_start = heap_page;
    process.heap_end = heap_page;

    // TODO: reopen and track opened files
    process.descriptor_to_open = current_process->descriptor_to_open;

    // If we replaced old process,
    // we should enable userspace and exit to userspace
    // TODO: move to syscall
    if (!create_new_process) {
        userspace_enable();
        exit_to_current_process();
    }
}

void execute_elf(const std::string& path, const std::vector<std::string>& args, bool create_new) {
    std::vector<std::string> envs;
    execute_elf(path, args, envs, create_new);
}

void execute_elf(const std::string& path, bool create_new) {
    std::vector<std::string> args;
    execute_elf(path, args, create_new);
}
