#ifndef _IoService_H_9C859311_12B0_4272_84A9_39DF45DB9094_INCLUDED_
#define _IoService_H_9C859311_12B0_4272_84A9_39DF45DB9094_INCLUDED_

#include "Session.h"
#include <asio.hpp>
#include <memory>
#include <functional>

class IoService
{
public:
  typedef std::function<PointerToSession(const Session::Engine& engine)> CreateSessionFunction;

  explicit IoService(const size_t workerThreadCount);
  ~IoService();
  void listen(const asio::ip::tcp::endpoint& listenEndpoint, const CreateSessionFunction& sessionCreator, bool reuseAddress = true);
  bool isWantedToDie() const;
private:
  IoService(const IoService&) = delete;
  void operator=(const IoService&) = delete;

  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

#endif //_IoService_H_9C859311_12B0_4272_84A9_39DF45DB9094_INCLUDED_
