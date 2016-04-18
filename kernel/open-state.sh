#!/usr/bin/env sh
# Load kernel Lua modules, create a Lua state and
# open standard libraries

if [ $# -eq 0 ]; then
	echo "Usage $0 <state name>"
	exit 1
fi

sudo modload lua

sudo sysctl -w kern.lua.verbose=1 1> /dev/null
sudo sysctl -w ddb.onpanic=1 1> /dev/null

luactl -cs require $1
