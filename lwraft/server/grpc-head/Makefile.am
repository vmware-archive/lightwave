noinst_LTLIBRARIES = libdomath-grpc.la

idl_srcdir=$(top_srcdir)/lwraft/idl

libdomath_grpc_la_SOURCES = \
    addint64.c \
    domath_server_c.c  \
    domath_server.cc \
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

libdomath_grpc_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    @LW_INCLUDES@

PROTOC = protoc
PROTOS_PATH = $(idl_srcdir)

$(GRPC_BUILT_SRCS): $(PROTOS_PATH)/domath.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

$(PROTOBUF_BUILT_SRCS): $(PROTOS_PATH)/domath.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

libdomath_grpc_la_LDFLAGS = \
    -static
