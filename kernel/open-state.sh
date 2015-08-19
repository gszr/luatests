#!/usr/bin/env sh
# Load kernel Lua modules, create a Lua state and
# open standard libraries

if [ $# -eq 0 ]; then
	echo "Usage $0 lua_state_name"
	exit 1
fi

sudo modload lua
sudo modload luaos
sudo modload luaio
sudo modload luamath
sudo modload luabase

sudo sysctl -w kern.lua.verbose=1
sudo sysctl -w ddb.onpanic=1

sudo luactl -c openlibs $1
