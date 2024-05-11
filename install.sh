#!/bin/sh

bash ./compile.sh
killall gosh
sudo cp ./gosh /bin/gosh