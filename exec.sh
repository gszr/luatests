#!/usr/bin/env sh
# Load all test scripts using luactl

if [ $# -eq 0 ]; then
	echo "Usage $0 lua_state_name"
	exit 1
fi

echo;
sudo modload lua 
sudo modload luaos 
sudo modload luamath
sudo sysctl -w kern.lua.verbose=1
sudo sysctl -w ddb.onpanic=1
sudo luactl -c openlibs $1

echo;
luactl load $1 ./preload.lua;

for i in `find . -name "*.lua" -maxdepth 1`; do
	case $i in
		*all.lua|*big.lua|*files.lua|*math.lua|*main.lua|*preload.lua) ;;
		*) luactl load $1 $i;
	esac
done
