#include "IoService.h"
#include <thread>
#include <chrono>
#include <iostream>

struct MySession : Session
{
  explicit MySession(const Session::Engine& engine) :
    Session(engine), bytesReceived(0)
  {}
  virtual void onDataReceived(Data& inputBuffer)
  {
    bytesReceived += inputBuffer.size();
    inputBuffer.clear();

    std::cout << "bytesReceived = " << bytesReceived << std::endl;
  }
  private:
    uint64_t bytesReceived;
};

const PointerToSession createSession(const Session::Engine& engine)
{
  return std::make_shared<MySession>(engine);
}

int main()
{
  const size_t preferredThreadsCount = std::thread::hardware_concurrency();
  IoService networkService(preferredThreadsCount);

  const asio::ip::tcp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 8080);
  networkService.listen(ep, &createSession);

  for (;;)
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
