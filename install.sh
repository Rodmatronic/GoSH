#!/bin/sh

# Warning, this cannot be run by GoSH directly

bash ./compile.sh
echo Killing any conflicting processes...
killall gosh

unixver='$(uname -s)'

if [[ $unixver != 'Darwin' ]]; then
    sudo rm /bin/gosh
    echo Installing to /bin
    sudo cp ./gosh /bin/gosh
    exit
fi

if [[ $unixver == 'Darwin' ]]; then
    echo Installing to /Applications
    sudo cp ./gosh /Applications
    exit
fi