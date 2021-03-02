#pragma once

#include <string_view>
#include <vector>
namespace reactor
{
class Buffer
{
  public:
    Buffer() : windex_(0), rindex_(0) {}

    size_t readable_bytes() const { return windex_ - rindex_; }

    //丢数据
    void retrive(std::size_t n);
    void retrive_all();

    //往buffer里读写
    std::string_view read(size_t n) const;
    template <typename Tp>
    void append(Tp *p, size_t n);
    void append(std::string_view s);

    void swap(Buffer &rhs);

  private:
    void   make_space_for_write(size_t n);
    size_t writeable_bytes() const { return buffer_.size() - windex_; }

  private:
    std::size_t windex_;
    std::size_t rindex_;

    std::vector<char> buffer_;
};
} // namespace reactor