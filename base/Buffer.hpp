#pragma once
#ifndef REACTOR_BUFFER_HPP
#define REACTOR_BUFFER_HPP

#include <string.h>
#include <string_view>
#include <vector>
namespace reactor
{
class Buffer
{
  public:
    Buffer() : windex_(0), rindex_(0), max_unread_(default_max_unread_) {}

    const char *readable_data() const { return buffer_.data() + rindex_; }
    size_t readable_bytes() const { return windex_ - rindex_; }

    //丢数据
    void retrive(std::size_t n);
    void retrive_all();

    //往buffer里读写
    std::string_view string(size_t n) const;
    std::string_view read_all_as_string() const { return std::string_view(string(readable_bytes())); }

    template <typename Tp> void append(Tp *p, size_t n)
    {
        make_space_for_write(n * sizeof(Tp));
        memcpy(buffer_.data() + windex_, p, n * sizeof(Tp));
        windex_ += n * sizeof(Tp);
    }
    void append(std::string_view s);
    void append(const Buffer &);

    // minimum default value 1024
    void set_max_unread_bytes(size_t max);

    void swap(Buffer &rhs);

  private:
    void make_space_for_write(size_t n);
    size_t writeable_bytes() const { return buffer_.size() - windex_; }

  private:
    constexpr static size_t default_max_unread_ = 65535;

    std::size_t windex_;
    std::size_t rindex_;
    std::size_t max_unread_;

    std::vector<char> buffer_;
};
} // namespace reactor

#endif