#pragma once

#include <cstdio>
#include <format>
#include <stdexcept>

struct Output {
    explicit Output(const char* filename) : m_file(std::fopen(filename, "wb")) {
        if (!m_file) {
            std::string msg("cannot open output ");
            msg.append(filename);
            throw std::runtime_error(msg);
        }
    }

    ~Output() { std::fclose(m_file); }

    void write(const void* data, std::size_t bytes) {
        std::fwrite(data, 1, bytes, m_file);
    }

    void seek(unsigned offset) { std::fseek(m_file, offset, SEEK_SET); }

    Output& operator<<(char c) {
        write(&c, 1);
        return *this;
    }

    Output& operator<<(std::string_view str) {
        write(str.data(), str.size());
        return *this;
    }

private:
    std::FILE* m_file;
};

inline void print(std::string_view str) {
    std::fwrite(str.data(), 1, str.size(), stdout);
}

template<class... T>
inline void print(std::string_view str, const T&... arg) {
    const auto s = std::vformat(str, std::make_format_args(arg...));
    std::fwrite(s.data(), 1, s.size(), stdout);
}
