#pragma once
#include "filesystem.h"
#include "vga_color.h"

//Black        0;30     Dark Gray     1;30
//Red          0;31     Light Red     1;31
//Green        0;32     Light Green   1;32
//Brown/Orange 0;33     Yellow        1;33
//Blue         0;34     Light Blue    1;34
//Purple       0;35     Light Purple  1;35
//Cyan         0;36     Light Cyan    1;36
//Light Gray   0;37     White         1;37
const vga_color linux_colors[][2] = {
        {vga_color_black, vga_color_dark_grey},
        {vga_color_red, vga_color_light_red},
        {vga_color_green, vga_color_light_green},
        {vga_color_brown, vga_color_light_brown},
        {vga_color_blue, vga_color_light_blue},
        {vga_color_magenta, vga_color_light_magenta},
        {vga_color_cyan, vga_color_light_cyan},
        {vga_color_light_grey, vga_color_white},
};



class stdin_file : public vfs_entry {
public:
    stdin_file(const std::string& name, vfs_entry* parent);

    std::size_t write(const_ptr_t buffer, std::size_t size, bool noblock) override;
    std::size_t read(ptr_t buffer, std::size_t size, bool noblock) override;
    bool open() override;
    bool close() override;
    std::size_t seek(std::size_t offset, file_whence_t whence) override;
    bool is_file() override;
    bool available_read() override;
};

class stdout_file : public vfs_entry {
    static void sgr_reset();
    static void sgr(int n);
    static void sgr(int n, int m);
    void parse_cs(std::stringstream& ss);
    void parse_escape(std::stringstream& ss);
public:
    stdout_file(const std::string& name, vfs_entry* parent);

    std::size_t write(const_ptr_t buffer, std::size_t size, bool noblock) override;
    std::size_t read(ptr_t buffer, std::size_t size, bool noblock) override;
    bool open() override;
    bool close() override;
    std::size_t seek(std::size_t offset, file_whence_t whence) override;
    bool is_file() override;
};



class dev_mem : public vfs_entry {
public:
    dev_mem(const std::string& name, vfs_entry* parent);

    std::size_t write(const_ptr_t buffer, std::size_t size, bool noblock) override;
    std::size_t read(ptr_t buffer, std::size_t size, bool noblock) override;
    bool open() override;
    bool close() override;
    std::size_t seek(std::size_t offset, file_whence_t whence) override;
    bool is_file() override;
    bool available_read() override;
};


class dev_zero : public vfs_entry {
public:
    dev_zero(const std::string& name, vfs_entry* parent);

    std::size_t write(const_ptr_t buffer, std::size_t size, bool noblock) override;
    std::size_t read(ptr_t buffer, std::size_t size, bool noblock) override;
    bool open() override;
    bool close() override;
    std::size_t seek(std::size_t offset, file_whence_t whence) override;
    bool is_file() override;
    bool available_read() override;
};


class dev_stub : public vfs_entry {
public:
    dev_stub(const std::string& name, vfs_entry* parent);

    std::size_t write(const_ptr_t buffer, std::size_t size, bool noblock) override;
    std::size_t read(ptr_t buffer, std::size_t size, bool noblock) override;
    bool open() override;
    bool close() override;
    std::size_t seek(std::size_t offset, file_whence_t whence) override;
    bool is_file() override;
    bool available_read() override;
};

