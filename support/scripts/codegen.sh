#!/bin/bash -xe
set -ex

swagger-gen() {
    local gentype="$1" module="$2" spec="$3"
    local pkgdir=gen/$module
    local extraopts

    rm -fr $pkgdir/models
    if [ "$gentype" == "server" ]; then
        extraopts="--exclude-main"
        rm -fr $pkgdir/restapi/{operations,doc.go,embedded_spec.go,server.go}
    fi

    new_spec=$(readlink -f $spec)
    cd go/src/
    mkdir -p $pkgdir
    swagger generate $gentype -f $new_spec -t $pkgdir -A $module -P models.Principal $extraopts
}

TYPE="$1"
MODULE="$2"
SPEC="$3"

test -n "$TYPE"
test -n "$MODULE"
test -n "$SPEC"

swagger-gen "$TYPE" "$MODULE" "$SPEC"
