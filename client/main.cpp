#include <dto/Serializer.h>
#include <experimental/filesystem>
#include <asio.hpp>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <fstream>
#include <iostream>

namespace
{
  struct ProgramOptions
  {
    const std::string endpoint;
    const std::string filePath;
  };

  ProgramOptions getProgramOptions(int argc, char** argv)
  {
    std::string endpoint, path;
    int opt;
    while ((opt = getopt(argc, argv, "e:f:h")) != EOF)
      switch(opt)
      {
        case 'e':
          endpoint = optarg;
          break;
        case 'f':
          path = optarg;
          break;
        case 'h':
        default:
          std::cout << "usage is -e <server endpoint>, -f <file path>" << std::endl;
          throw std::invalid_argument("");
      }

    if(endpoint.empty() || path.empty())
      throw std::invalid_argument("incorrect launch parameters specified, usage example ./client -e 127.0.0.1:8080 -f ./client");

    return ProgramOptions{endpoint, path};
  }

  const asio::ip::tcp::endpoint createTcpEndpoint(const std::string& source, asio::io_service& io_service)
  {
    size_t colonPos = source.find(':');
    if(colonPos != std::string::npos)
    {
      try
      {
        const std::string address = std::string(source.begin(), source.begin() + colonPos);
        const std::string port = std::string(source.begin() + colonPos + 1, source.end());
        const asio::ip::tcp::resolver::query query(address, port);
        asio::ip::tcp::resolver resolver(io_service);
        const asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);

        return iter->endpoint();
      }
      catch(...)
      {
      }
    }
    throw std::invalid_argument("incorrect endpoint string representation");
  }

  void uploadFile(const std::string& sourceFilename, asio::ip::tcp::socket& socket)
  {
    std::ifstream sourceFile(sourceFilename.c_str(), std::ifstream::binary);
    if (!sourceFile)
    {
      throw std::runtime_error("Can't open the source file: " + sourceFilename);
    }

    const std::experimental::filesystem::path path(sourceFilename);
    FileServer::Messages::CreateFile messaage;
    messaage.set_path(std::experimental::filesystem::canonical(path).string());
    messaage.set_size(std::experimental::filesystem::file_size(path));

    std::vector<char> header;
    Serialization::serializeDelimited(messaage, header);
    socket.write_some(asio::buffer(header.data(), header.size()));

    sourceFile.seekg(0);
    std::vector<char> dataChunk(1500,0);

    while(!sourceFile.eof())
    {
      sourceFile.read(dataChunk.data(), dataChunk.size());
      const std::streamsize bytesReallyRead = sourceFile.gcount();
      socket.write_some(asio::buffer(dataChunk.data(), bytesReallyRead));
    }
  }
} // namespace

int main(int argc, char** argv)
try
{
  const ProgramOptions options = getProgramOptions(argc, argv);

  asio::io_service io_service;
  asio::ip::tcp::endpoint endpoint = createTcpEndpoint(options.endpoint, io_service);

  asio::ip::tcp::socket socket(io_service);
  socket.connect(endpoint);
  uploadFile(options.filePath, socket);

  return EXIT_SUCCESS;
}
catch(std::exception& ex)
{
  std::cout << ex.what() << std::endl;

  return EXIT_FAILURE;
}
catch(...)
{
  return EXIT_SUCCESS;
}
