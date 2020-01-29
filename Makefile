all:
	eosio-cpp -Oz -o signup.wasm -I. -I../eosio.contracts/contracts/eosio.system/include -I../eosio.contracts/contracts/eosio.system signup.cpp --abigen
