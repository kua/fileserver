#include "IoService.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>

struct WriteFileSession : Session
{
  explicit WriteFileSession(const Session::Engine& engine) :
    Session(engine), bytesReceived(0)
  {
    m_fileStream.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    m_fileStream.open("./client", std::ofstream::out | std::ofstream::trunc);

    if(!m_fileStream.is_open())
    {
      throw std::runtime_error("Unable to open file");
    }
  }

  virtual void onDataReceived(Data& inputBuffer)
  {
    bytesReceived += inputBuffer.size();
    m_fileStream.write(inputBuffer.data(), inputBuffer.size());
    m_fileStream.flush();

    inputBuffer.clear();
  }

  ~WriteFileSession()
  {
    m_fileStream.close();
  }

private:
  std::ofstream m_fileStream;
  uint64_t bytesReceived;
};

const PointerToSession createSession(const Session::Engine& engine)
{
  return std::make_shared<WriteFileSession>(engine);
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
