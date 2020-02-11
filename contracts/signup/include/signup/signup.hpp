#include <cmath>
#include <eosio/print.hpp>

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio.system/exchange_state.hpp>
#include <src/exchange_state.cpp>
#include <eosio.system/eosio.system.hpp>
#include <eosio/system.hpp>

using namespace std;
using namespace eosio;

const symbol EOS_S = symbol("EOS", 4);
const symbol RAMCORE_S = symbol("RAMCORE", 4);
const symbol RAM_S = symbol("RAM", 0);
uint32_t refund_delay_sec = 3*24*3600;

struct [[eosio::table, eosio::contract("eosio.system")]] delegated_bandwidth {
  name          from;
  name          to;
  asset         net_weight;
  asset         cpu_weight;

  bool is_empty()const { return net_weight.amount == 0 && cpu_weight.amount == 0; }
  uint64_t  primary_key()const { return to.value; }

  // explicit serialization macro is not necessary, used here only to improve compilation time
  EOSLIB_SERIALIZE( delegated_bandwidth, (from)(to)(net_weight)(cpu_weight) )

};

struct [[eosio::table, eosio::contract("eosio.system")]] refund_request {
  name            owner;
  time_point_sec  request_time;
  eosio::asset    net_amount;
  eosio::asset    cpu_amount;

  bool is_empty()const { return net_amount.amount == 0 && cpu_amount.amount == 0; }
  uint64_t  primary_key()const { return owner.value; }

  // explicit serialization macro is not necessary, used here only to improve compilation time
  EOSLIB_SERIALIZE( refund_request, (owner)(request_time)(net_amount)(cpu_amount) )
};

typedef eosio::multi_index< "delband"_n, delegated_bandwidth > del_bandwidth_table;
typedef eosio::multi_index< "refunds"_n, refund_request >      refunds_table;

CONTRACT signup : public eosio::contract {

  private:
    const name REFERENCE = name("cryptobank24");
    const name PARTNER = name("cryptobank24");
    array<char, 33> owner_pubkey_char;
    array<char, 33> active_pubkey_char;
    void self_refund();

  public:
    using contract::contract;

    ACTION notify(name new_account);
    ACTION claim(name account_name);
    ACTION refund();

    [[eosio::on_notify("eosio.token::transfer")]]
    void on_transfer( name from, name to, asset quantity, string memo );

    signup( name receiver, name code, datastream<const char*> ds ):
      contract( receiver, code, ds ) {}
};
