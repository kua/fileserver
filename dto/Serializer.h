#ifndef _Serializer_H_1CAC97CB_E706_485D_8D20_6AA7A7C0EB05_INCLUDED_
#define _Serializer_H_1CAC97CB_E706_485D_8D20_6AA7A7C0EB05_INCLUDED_

#include "dto/fileserver.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include <memory>

namespace Serialization
{
  void serializeDelimited(const FileServer::Messages::CreateFile& msg, std::vector<char>& result)
  {
    const size_t messageSize = msg.ByteSize();
    const size_t headerSize = google::protobuf::io::CodedOutputStream::VarintSize32(messageSize);
    result.resize(headerSize + messageSize);

    google::protobuf::uint8* buffer = reinterpret_cast<google::protobuf::uint8*>(result.data());
    google::protobuf::io::CodedOutputStream::WriteVarint32ToArray(messageSize, buffer);
    msg.SerializeWithCachedSizesToArray(buffer + headerSize);
  }

  const std::shared_ptr<FileServer::Messages::CreateFile> parseDelimited(const std::vector<char>& data, size_t& bytesConsumed)
  {
    google::protobuf::io::CodedInputStream is(reinterpret_cast<const google::protobuf::uint8*>(data.data()), data.size());
    google::protobuf::uint32 messageSize = 0;
    const bool isMessageSizeRead = is.ReadVarint32(&messageSize);
    if(!isMessageSizeRead)
    {
      return std::shared_ptr<FileServer::Messages::CreateFile>();
    }
    const size_t totalFrameSize = google::protobuf::io::CodedOutputStream::VarintSize32(messageSize) + messageSize;
    if(data.size() < totalFrameSize)
    {
      return std::shared_ptr<FileServer::Messages::CreateFile>();
    }
    const std::shared_ptr<FileServer::Messages::CreateFile> result = std::make_shared<FileServer::Messages::CreateFile>();
    is.PushLimit(messageSize);
    const bool parseOk = result->ParseFromCodedStream(&is);

    if(!parseOk)
      throw std::runtime_error("fileserver message parse error");

    bytesConsumed = totalFrameSize;

    return result;
  }
}

#endif //_Serializer_H_1CAC97CB_E706_485D_8D20_6AA7A7C0EB05_INCLUDED_
