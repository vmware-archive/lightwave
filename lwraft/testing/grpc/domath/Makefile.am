noinst_PROGRAMS = domath_client

idl_srcdir=$(top_srcdir)/lwraft/idl

domath_client_SOURCES = \
    domath_client.cc \
    domath.pb.cc \
    domath.grpc.pb.cc

GRPC_BUILT_INCLUDES = \
    domath.grpc.pb.h \
    domath.pb.h

GRPC_BUILT_SRCS = \
    domath.grpc.pb.cc \
    domath.grpc.pb.h

PROTOBUF_BUILT_SRCS = \
    domath.pb.cc \
    domath.pb.h

CLEANFILES = \
    $(GRPC_BUILT_SRCS) \
    $(PROTOBUF_BUILT_SRCS) \
    $(GRPC_BUILT_INCLUDES)

BUILT_SOURCES = $(GRPC_BUILT_INCLUDES)

GRPC_CPP_PLUGIN_PATH = `which grpc_cpp_plugin`

domath_client_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    @LW_INCLUDES@

domath_client_LDFLAGS = \
    `pkg-config --libs protobuf grpc++ grpc`

PROTOC = protoc
PROTOS_PATH = $(idl_srcdir)

$(GRPC_BUILT_SRCS): $(PROTOS_PATH)/domath.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

$(PROTOBUF_BUILT_SRCS): $(PROTOS_PATH)/domath.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<
