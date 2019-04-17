all:
	eosio-cpp -Oz -o signup.wasm -I. -I../eosio.contracts-old/eosio.system/include -I../eosio.contracts-old/eosio.system signup.cpp --abigen