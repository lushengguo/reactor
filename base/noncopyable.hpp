#pragma once
namespace reactor
{
class noncopyable
{
protected:
  noncopyable() {}
  ~noncopyable() {}

private:
  // forbid copy or assign
  noncopyable(const noncopyable &rhs) {}
  noncopyable &operator=(const noncopyable &rhs) { return *this; }
};
} // namespace reactor