#!/bin/bash

if [ "$#" = 1 ]; then

    pwgen > applications/tmp_uuid.h
    echo "const char* uuid = \"`cat applications/tmp_uuid.h`\";" > applications/uuid.h
    echo "\"`cat applications/tmp_uuid.h`\"; \"$1\"" >> applications/uuid.log

    make clean
    make flash APP=rocket_tag

fi
