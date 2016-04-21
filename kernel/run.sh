#!/usr/bin/env sh
#
# Run a test suite script in a kernel Lua state, 
# making sure the state is clean; if it already exists,
# it's recreated

if [ $# != 2 ]; then
	echo "Usage $0 <state name> <script name>"
	exit 1
fi

sudo modload lua 2> /dev/null

sudo sysctl -w kern.lua.verbose=1  1> /dev/null
sudo sysctl -w ddb.onpanic=1       1> /dev/null

luactl destroy $1 2> /dev/null
luactl -cs require $1 2> /dev/null
luactl load $1 ./kernel/preload.lua
luactl load $1 $2
