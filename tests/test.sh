#!/bin/gosh
echo "Test 1..."
sleep 1
uname
tty
ps
echo "Test 2!..."
sleep 1
export
export TEST_RUN=ya
echo "3..?"
sleep 1
gosh test2.sh
echo bye!