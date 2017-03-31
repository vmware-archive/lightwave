lib_LTLIBRARIES = libssovmdirclient.la

libssovmdirclient_la_SOURCES = \
    group_array_data.c \
    group_data.c \
    group_details_data.c \
    group_resource.c \
    password_details_data.c \
    password_reset_request_data.c \
    principal_data.c \
    search_result_data.c \
    solution_user_array_data.c \
    solution_user_data.c \
    solution_user_resource.c \
    string_array_data.c \
    user_array_data.c \
    user_data.c \
    user_details_data.c \
    user_resource.c

libssovmdirclient_la_CPPFLAGS = \
    -I$(top_srcdir)/ssoclients/common/include/ \
    -I$(top_srcdir)/ssoclients/common/include/public/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/public/ \
    -I$(top_srcdir)/ssoclients/restclient/vmdirclient/include/public/

libssovmdirclient_la_LIBADD = \
    @top_builddir@/ssoclients/restclient/coreclient/src/libssocoreclient.la
