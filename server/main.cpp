#include "IoService.h"
#include <dto/Serializer.h>
#include <dto/fileserver.pb.h>
#include <experimental/filesystem>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>

struct WriteFileSession : Session
{
  explicit WriteFileSession(const Session::Engine& engine, const std::experimental::filesystem::path& serverFilesFolder) :
    Session(engine), m_basePath(serverFilesFolder), m_isInit(false), m_totalFileSize(0)
  {
  }

  bool isInit() const
  {
    return m_isInit;
  }

  void onDataReceived(Data& inputBuffer) override
  {
    if(!isInit())
    {
      parseMetaInfo(inputBuffer);
    }
    else
    {
      m_fileStream.write(inputBuffer.data(), inputBuffer.size());
      m_fileStream.flush();

      inputBuffer.clear();
    }
  }

  void parseMetaInfo(Data& inputBuffer)
  try
  {
    if(!inputBuffer.empty())
    {
      size_t bytesConsumed = 0;
      const std::shared_ptr<FileServer::Messages::CreateFile> message = Serialization::parseDelimited(inputBuffer, bytesConsumed);

      if(!message)
        return;

      if(!bytesConsumed)
      {
        inputBuffer.clear();
        return;
      }

      inputBuffer.erase(inputBuffer.begin(), inputBuffer.begin() + bytesConsumed);

      const std::experimental::filesystem::path receivedFilePath(message->path());
      const std::experimental::filesystem::path serverFilePath(m_basePath / receivedFilePath);

      std::experimental::filesystem::create_directories(serverFilePath.parent_path());

      std::cout << "start writing file " << serverFilePath << std::endl;

      m_fileStream.exceptions(std::ofstream::failbit | std::ofstream::badbit);
      m_fileStream.open(serverFilePath.string(), std::ofstream::out | std::ofstream::trunc);

      if(!m_fileStream.is_open())
        throw std::runtime_error("unable to open target file for writing");

      m_totalFileSize = message->size();
      m_isInit = true;
    }
  }
  catch (std::exception& ex)
  {
    std::cerr << "an exception occured in WriteFileSession: " << ex.what();
    close();
  }

  ~WriteFileSession()
  {
    m_fileStream.close();
  }

private:
  const std::experimental::filesystem::path m_basePath;
  bool m_isInit;
  std::ofstream m_fileStream;
  uint64_t m_totalFileSize;
};

const PointerToSession createSession(const Session::Engine& engine, const std::experimental::filesystem::path& serverFilesFolder)
{
  return std::make_shared<WriteFileSession>(engine, serverFilesFolder);
}

int main()
{
  const size_t preferredThreadsCount = std::thread::hardware_concurrency();
  IoService networkService(preferredThreadsCount);

  const asio::ip::tcp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 8080);
  const std::experimental::filesystem::path serverFilesFolder = "/tmp";

  networkService.listen(ep, std::bind(&createSession, std::placeholders::_1, serverFilesFolder));

  for (;;)
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
