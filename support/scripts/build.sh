#!/bin/bash

PROJECT_ROOT=$(pwd)

cd $PROJECT_ROOT/build && \
    make -s && \
    make -s check
