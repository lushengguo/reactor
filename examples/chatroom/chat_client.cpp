#include "base/log.hpp"
#include "protocal.hpp"
#include <iostream>
namespace reactor
{
class ChatClient
{
  public:
    void tips() { std::cout << protocal_.tips() << std::flush; }
    void get_input() { std::getline(std::cin, input_); }

  private:
    Protocal    protocal_;
    std::string input_;
};
} // namespace reactor

using namespace reactor;

int main()
{
    disable_log_print();
    ChatClient client;
    client.show_tips();
}