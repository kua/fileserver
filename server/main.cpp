#include "IoService.h"
#include <dto/Serializer.h>
#include <dto/fileserver.pb.h>
#include <experimental/filesystem>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>

struct WriteFileSession : Session
{
  explicit WriteFileSession(const Session::Engine& engine, const std::experimental::filesystem::path& serverFilesFolder) :
    Session(engine), m_basePath(serverFilesFolder), m_isInit(false), m_totalFileSize(0), m_bytesReceived(0)
  {
  }

  bool isInit() const
  {
    return m_isInit;
  }

  bool isOver() const
  {
    return m_totalFileSize <= m_bytesReceived;
  }

  void onDataReceived(Data& inputBuffer) override
  {
    if(!isInit())
    {
      parseMetaInfo(inputBuffer);
    }

    if(!inputBuffer.empty())
    {
      writeDataChunk(inputBuffer);

      inputBuffer.clear();

      if(isOver())
        close();
    }
  }

  void writeDataChunk(Data& inputBuffer)
  try
  {
    m_fileStream.write(inputBuffer.data(), inputBuffer.size());
    m_fileStream.flush();
    m_bytesReceived += inputBuffer.size();
  }
  catch (std::exception& ex)
  {
    std::cerr << "an exception occured in WriteFileSession while writing data to file: " << ex.what();
    close();
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
        throw std::runtime_error("bytesConsumed should be greater than 0 after successful metadata message parsing");
      }

      inputBuffer.erase(inputBuffer.begin(), inputBuffer.begin() + bytesConsumed);

      const std::experimental::filesystem::path receivedFilePath(message->path());
      m_filePath = m_basePath / receivedFilePath;

      std::experimental::filesystem::create_directories(m_filePath.parent_path());

      std::cout << "start writing file " << m_filePath << std::endl;

      m_fileStream.exceptions(std::ofstream::failbit | std::ofstream::badbit);
      m_fileStream.open(m_filePath.string(), std::ofstream::out | std::ofstream::trunc);

      if(!m_fileStream.is_open())
        throw std::runtime_error("unable to open target file for writing");

      m_totalFileSize = message->size();
      m_isInit = true;
    }
  }
  catch (std::exception& ex)
  {
    std::cerr << "an exception occured in WriteFileSession while file creating: " << ex.what();
    close();
  }

  ~WriteFileSession()
  {
    m_fileStream.close();

    if(!isOver() && !m_filePath.empty())
    {
      std::experimental::filesystem::remove_all(m_filePath);
    }
  }

private:
  const std::experimental::filesystem::path m_basePath;
  std::experimental::filesystem::path m_filePath;
  bool m_isInit;
  std::ofstream m_fileStream;
  uint64_t m_totalFileSize;
  uint64_t m_bytesReceived;
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

  while(!networkService.isWantedToDie())
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
