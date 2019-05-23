#include <cmath>
#include <eosiolib/print.hpp>

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosio.system/exchange_state.hpp>
#include <src/exchange_state.cpp>
#include <eosio.system/eosio.system.hpp>

using namespace std;
using namespace eosio;

const symbol EOS_S = symbol("EOS", 4);
const symbol TLOS_S = symbol("TLOS", 4);
const symbol RAMCORE_S = symbol("RAMCORE", 4);
const symbol RAM_S = symbol("RAM", 0);

CONTRACT signup : public eosio::contract {

  private:
    const name REFERENCE = name("cryptobank24");
    const name PARTNER = name("getmoretoken");
    array<char, 33> owner_pubkey_char;
    array<char, 33> active_pubkey_char;

  public:
    using contract::contract;

    ACTION notify(name new_account);

    [[eosio::on_notify("eosio.token::transfer")]]
    void on_transfer( name from, name to, asset quantity, string memo );

    signup( name receiver, name code, datastream<const char*> ds ):
      contract( receiver, code, ds ) {}
};
