#pragma once

#include <cstdio>
#include <memory>
#include <stdexcept>

struct Input {
    explicit Input(const char* filename) : m_file(std::fopen(filename, "rb")) {
        if (!m_file) {
            std::string msg("cannot open input ");
            msg.append(filename);
            throw std::runtime_error(msg);
        }
        std::fseek(m_file, 0, SEEK_END);
        const auto size = std::ftell(m_file);
        std::fseek(m_file, 0, SEEK_SET);
        m_data = ::operator new(size);
        m_size = std::fread(m_data, 1, size, m_file);
    }

    ~Input() {
        std::fclose(m_file);
        ::operator delete(m_data);
    }

    std::size_t size() const { return m_size; }

    const void* data() const { return m_data; }

private:
    std::FILE* m_file;
    void* m_data;
    std::size_t m_size;
};