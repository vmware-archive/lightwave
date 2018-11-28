noinst_LTLIBRARIES = libpostwatch.la

idl_srcdir=$(top_srcdir)/lwraft/idl

libpostwatch_la_SOURCES = \
    postwatch_server_c.c  \
    postwatch_server.cc \
    postwatch.pb.cc \
    postwatch.grpc.pb.cc \
    postwatch_queue.c

GRPC_BUILT_INCLUDES = \
    postwatch.grpc.pb.h \
    postwatch.pb.h

GRPC_BUILT_SRCS = \
    postwatch.grpc.pb.cc \
    postwatch.grpc.pb.h

PROTOBUF_BUILT_SRCS = \
    postwatch.pb.cc \
    postwatch.pb.h

CLEANFILES = \
    $(GRPC_BUILT_SRCS) \
    $(PROTOBUF_BUILT_SRCS) \
    $(GRPC_BUILT_INCLUDES)

BUILT_SOURCES = $(GRPC_BUILT_INCLUDES)

libpostwatch_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -Wall  -Werror \
    @LW_INCLUDES@

PROTOC = protoc
PROTOS_PATH = $(idl_srcdir)

$(GRPC_BUILT_SRCS): $(PROTOS_PATH)/postwatch.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

$(PROTOBUF_BUILT_SRCS): $(PROTOS_PATH)/postwatch.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

libpostwatch_la_LDFLAGS = \
    -static
