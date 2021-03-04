#pragma once
#ifndef REACTOR_ERRNO_HPP
#define REACTOR_ERRNO_HPP

#include <errno.h>
#include <string.h>
#include <string>

namespace reactor
{
class ErrorCode
{
  public:
    ErrorCode() : err_(errno) {}
    operator bool() { return err_ == 0; }

    std::string str() const { return strerror(err_); }
    bool        EAgain() { return err_ == EAGAIN; }

  private:
    int err_;
};

} // namespace reactor
#endif