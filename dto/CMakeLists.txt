set(SRCS Serializer.h)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS "fileserver.proto")

add_library(dto fileserver.proto ${SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
