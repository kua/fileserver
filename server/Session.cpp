#include "Session.h"
#include <stdexcept>

namespace
{
  const size_t INPUT_BUFFER_EMPTY_SPACE_REQUIREMENT = 65536;
}

Session::Session(const Engine& engine)
  : m_engine(engine)
  , m_isAsyncReadInProgress(false)
{
}

void Session::close()
{
  if(m_engine.socket->is_open())
    m_engine.socket->close();
}

void Session::closeAsync()
{
  invokeAsync(std::bind(&Session::close, this));
}

void Session::invokeAsync(const AsyncOperation& operation)
try
{
  m_engine.synchronizer->post(std::bind(&Session::handleAsyncOperation, shared_from_this(), operation));
}
catch(const std::bad_weak_ptr&)
{
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

void Session::startReading(const bool callHandlerImmediately)
{
  if (m_isAsyncReadInProgress)
    return;

  const size_t bytesAlreadyStored = m_inputBuffer.size();
  m_inputBuffer.resize(bytesAlreadyStored + INPUT_BUFFER_EMPTY_SPACE_REQUIREMENT);

  asio::async_read(
    *m_engine.socket,
    asio::buffer(m_inputBuffer.data() + bytesAlreadyStored, INPUT_BUFFER_EMPTY_SPACE_REQUIREMENT),
    asio::transfer_at_least(callHandlerImmediately ? 0 : 1),
    m_engine.synchronizer->wrap(std::bind(&Session::handleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2))
  );

  m_isAsyncReadInProgress = true;
}

void Session::handleRead(const asio::error_code& error, const size_t bytesTransferred)
{
  m_isAsyncReadInProgress = false;

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

void Session::handleAsyncOperation(const AsyncOperation& asyncOperation)
try
{
  asyncOperation();
}
catch(...)
{
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
