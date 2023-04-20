#!/bin/bash

# Read first argument
if [ -z "$1" ]; then
    echo "No argument supplied"
    exit 1
fi
CLASSPATH=.:commons-io-1.2.jar:commons-cli-1.1.jar:rabbitmq-client.jar
if [ "$1" = "productor" ]; then

    for var in "$@"; do
        if [[ "$var" != "productor" ]]; then
            new_args+=($var)
        fi
    done
    java -cp $CLASSPATH Send ${new_args[@]}
fi

if [ "$1" = "consumidor" ]; then
    java -cp $CLASSPATH Recv
fi
