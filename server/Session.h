#ifndef _Session_H_1D10ED4E_5A09_4373_AB33_32F415016CC8_INCLUDED_
#define _Session_H_1D10ED4E_5A09_4373_AB33_32F415016CC8_INCLUDED_

#include <asio.hpp>
#include <memory>

typedef std::vector<char> Data;

class Session
  : public std::enable_shared_from_this<Session>
{
public:
  class Engine
  {
  public:
    explicit Engine(asio::io_service& asioIoService)
      : socket(new asio::ip::tcp::socket(asioIoService))
      , synchronizer(new asio::strand(asioIoService))
    {
    }

    const std::shared_ptr<asio::ip::tcp::socket> socket;
    const std::shared_ptr<asio::strand> synchronizer;
  };

  void start();
  virtual ~Session() {}

protected:
  explicit Session(const Engine& essence);
  void close();

private:
  virtual void onDataReceived(Data& inputBuffer);
  virtual void onSocketError(const asio::error_code& error);

private:
  void startReading();

  void handleRead(const asio::error_code& error, const size_t bytesTransferred);
  void handleSocketError(const asio::error_code& error);

  Session(const Session&) = delete;
  void operator=(const Session&) = delete;

  Engine m_engine;
  Data m_inputBuffer;
};

typedef std::shared_ptr<Session> PointerToSession;

#endif //_Session_H_1D10ED4E_5A09_4373_AB33_32F415016CC8_INCLUDED_
