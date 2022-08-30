#pragma once

#include <cstdint>
#include <string>
#include <fstream>
#include <unordered_set>
#include <vector>
#include "panic.h"
#include "allocator.h"


constexpr std::size_t EI_NIDENT = 16;

typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;

struct [[gnu::packed]] elf32_header {
    unsigned char   e_ident[EI_NIDENT];
    Elf32_Half      e_type;
    Elf32_Half      e_machine;
    Elf32_Word      e_version;
    Elf32_Addr      e_entry;
    Elf32_Off       e_phoff;
    Elf32_Off       e_shoff;
    Elf32_Word      e_flags;
    Elf32_Half      e_ehsize;
    Elf32_Half      e_phentsize;
    Elf32_Half      e_phnum;
    Elf32_Half      e_shentsize;
    Elf32_Half      e_shnum;
    Elf32_Half      e_shstrndx;
};


struct [[gnu::packed]] elf32_program_header {
    Elf32_Word      p_type;
    Elf32_Off       p_offset;
    Elf32_Addr      p_vaddr;
    Elf32_Addr      p_paddr;
    Elf32_Word      p_filesz;
    Elf32_Word      p_memsz;
    Elf32_Word      p_flags;
    Elf32_Word      p_align;
};


constexpr std::size_t PT_LOAD = 1;

constexpr std::size_t stack_page_count = 16;

void execute_elf(const std::string& path, const std::vector<std::string>& args, const std::vector<std::string>& envs, bool create_new_process = true);
void execute_elf(const std::string& path, const std::vector<std::string>& args, bool create_new = true);
void execute_elf(const std::string& path, bool create_new = true);