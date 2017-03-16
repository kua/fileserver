#include "IoService.h"
#include <assert.h>
#include <thread>
#include <iostream>
#include <stdexcept>

typedef asio::ip::tcp::resolver asioResolver;

namespace
{
  void startAccept(asio::ip::tcp::acceptor& acceptor, const IoService::CreateSessionFunction& sessionCreator);

  void handleAccept(const asio::error_code& error, asio::ip::tcp::acceptor& acceptor,
                    const IoService::CreateSessionFunction& sessionCreator, const Session::Engine& sessionEngine)
  try
  {
    if (!error)
    {        
      PointerToSession newSession = sessionCreator(sessionEngine);
      newSession->start();
    }

    startAccept(acceptor, sessionCreator);
  }
  catch (const std::exception& err)
  {
    assert(((std::cerr << err.what() << std::endl), !"Exception thrown in the accept handler!"));
  }
  catch (...)
  {
    assert(!"Unknown exception thrown in the accept handler!");
  }

  void startAccept(asio::ip::tcp::acceptor& acceptor, const IoService::CreateSessionFunction& sessionCreator)
  {
    const Session::Engine sessionEngine(acceptor.get_io_service());

    typedef std::function<void(const asio::error_code&)> AsioAcceptHandler;
    const AsioAcceptHandler acceptHandler = std::bind(handleAccept, std::placeholders::_1, std::ref(acceptor), sessionCreator, sessionEngine);

    acceptor.async_accept(*sessionEngine.socket, sessionEngine.synchronizer->wrap(acceptHandler));
  }
}

struct IoService::Impl
{
  Impl(const size_t workerThreadCount)
  try
    : m_dummyWork(new asio::io_service::work(m_ioService))
  {
    if (workerThreadCount == 0)
      throw std::invalid_argument("IoService: аргумент workerThreadCount не может быть равен нулю");

    for(size_t i = 0; i < workerThreadCount; ++i)
      m_workerThreads.emplace_back(std::bind(static_cast<std::size_t (asio::io_service::*)()>(&asio::io_service::run), &m_ioService));
  }
  catch(const std::exception&)
  {
    std::ostringstream what;
    what  << "Ошибка создания рабочих потоков, workerThreadCount=" << workerThreadCount;
    throw std::invalid_argument(what.str());
  }

  asio::io_service m_ioService;
  std::unique_ptr<asio::io_service::work> m_dummyWork;
  std::vector<std::thread> m_workerThreads;
  std::vector<std::shared_ptr<asio::ip::tcp::acceptor>> m_acceptors;
};

IoService::IoService(const size_t workerThreadCount)
  : m_impl(new Impl(workerThreadCount))
{
}

IoService::~IoService()
{
  m_impl->m_dummyWork.reset();
  m_impl->m_ioService.stop();

  for (auto& thread : m_impl->m_workerThreads)
    if (thread.joinable())
      thread.join();
}

void IoService::listen(const asio::ip::tcp::endpoint& listenEndpoint, const CreateSessionFunction& sessionCreator, bool reuseAddress)
{
  const std::shared_ptr<asio::ip::tcp::acceptor> acceptor = std::make_shared<asio::ip::tcp::acceptor>(m_impl->m_ioService, listenEndpoint, reuseAddress);
  m_impl->m_acceptors.push_back(acceptor);

  startAccept(*acceptor, sessionCreator);
}
