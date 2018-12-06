noinst_PROGRAMS = postwatch_client

idl_srcdir=$(top_srcdir)/lwraft/idl

postwatch_client_SOURCES = \
    postwatch_client.cc \
    postwatch.pb.cc \
    postwatch.grpc.pb.cc

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

GRPC_CPP_PLUGIN_PATH = `which grpc_cpp_plugin`

postwatch_client_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    @LW_INCLUDES@

postwatch_client_LDFLAGS = \
    `pkg-config --libs protobuf grpc++ grpc`

PROTOC = protoc
PROTOS_PATH = $(idl_srcdir)

$(GRPC_BUILT_SRCS): $(PROTOS_PATH)/postwatch.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

$(PROTOBUF_BUILT_SRCS): $(PROTOS_PATH)/postwatch.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<
