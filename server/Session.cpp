#include "Session.h"
#include <stdexcept>

namespace
{
  const size_t INPUT_BUFFER_EMPTY_SPACE_REQUIREMENT = 65536;
}

Session::Session(const Engine& engine)
  : m_engine(engine)
{
}

void Session::close()
{
  if(m_engine.socket->is_open())
    m_engine.socket->close();
}

void Session::start()
{
  startReading();
}

void Session::onDataReceived(Data& inputBuffer)
{
  inputBuffer.clear();
}

void Session::onSocketError(const asio::error_code& error)
{
  close();
}

void Session::startReading()
{
  const size_t bytesAlreadyStored = m_inputBuffer.size();
  m_inputBuffer.resize(bytesAlreadyStored + INPUT_BUFFER_EMPTY_SPACE_REQUIREMENT);

  asio::async_read(
    *m_engine.socket,
    asio::buffer(m_inputBuffer.data() + bytesAlreadyStored, INPUT_BUFFER_EMPTY_SPACE_REQUIREMENT),
    asio::transfer_at_least(1),
    m_engine.synchronizer->wrap(std::bind(&Session::handleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2))
  );
}

void Session::handleRead(const asio::error_code& error, const size_t bytesTransferred)
{
  const size_t bytesStored = m_inputBuffer.size() + bytesTransferred - INPUT_BUFFER_EMPTY_SPACE_REQUIREMENT;
  m_inputBuffer.resize(bytesStored);

  if (error)
  {
    handleSocketError(error);
    return;
  }

  try
  {
    onDataReceived(m_inputBuffer);
  }
  catch(...)
  {
    assert(!"An exception thrown within the onDataReceived function.");
    close();
  }

  startReading();
}

void Session::handleSocketError(const asio::error_code& error)
try
{
  onSocketError(error);
}
catch(...)
{
  assert(!"An exception thrown within the read error handler.");
  close();
}
