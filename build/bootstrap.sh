autoreconf -vif .. \
&& \
../configure \
    CFLAGS="-Wall -Werror -Wno-unused-but-set-variable \
            -Wno-pointer-sign -Wno-implicit-function-declaration \
            -Wno-address -Wno-enum-compare \
            -Wformat -Wformat-security -Werror=format-security \
            -D_FORTIFY_SOURCE=2 -O2 \
            -fno-strict-aliasing -fstack-protector-all" \
    LDFLAGS="-ldl -pie -fPIE -Wl,-z,now -Wl,-z,relro" \
    --prefix=/opt/vmware \
    --enable-debug=yes \
    --libdir=/opt/vmware/lib64 \
    --localstatedir=/var/lib/vmware \
    --with-config=./config \
