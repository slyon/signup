#include <cmath>
#include <eosiolib/print.hpp>

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosio.system/exchange_state.hpp>
#include <src/exchange_state.cpp>
#include <eosio.system/eosio.system.hpp>
//#include <eosio.system/native.hpp>

using namespace std;
using namespace eosio;

const symbol EOS_S = symbol("EOS", 4);
const symbol RAMCORE_S = symbol("RAMCORE", 4);
const symbol RAM_S = symbol("RAM", 0);

// eosiosystem::native::newaccount Doesn't seem to want to take authorities.
/*struct call {
  struct eosio {
    void newaccount(name creator, name newact,
                    authority owner, authority active);
  };
};*/

CONTRACT signup : public eosio::contract {

  private:
    const name REFERENCE = name("cryptobank24");
    /*
    struct signup_public_key {
        uint8_t        type;
        array<unsigned char,33> data;
    };
    struct permission_level_weight {
        permission_level permission;
        uint16_t weight;
    };
    struct key_weight {
        signup_public_key key;
        uint16_t weight;
    };
    struct wait_weight {
        uint32_t wait_sec;
        uint16_t weight;
    };
    struct authority {
        uint32_t threshold;
        vector<key_weight> keys;
        vector<permission_level_weight> accounts;
        vector<wait_weight> waits;
    };
    struct newaccount {
        name creator;
        name name;
        authority owner;
        authority active;
    };
    */
    // Temporary authority until native is fixed. Ref: https://github.com/EOSIO/eos/issues/4669
    /*struct wait_weight {
      uint32_t wait_sec;
      //weight_type weight;
      uint16_t weight;
    };
    struct authority {
      uint32_t threshold;
      vector<eosiosystem::key_weight> keys;
      vector<eosiosystem::permission_level_weight> accounts;
      vector<wait_weight> waits;
    };*/
    array<char, 33> owner_pubkey_char;
    array<char, 33> active_pubkey_char;
    enum plan_type: uint8_t {
      SMALL = 1,
      MEDIUM = 2,
      LARGE = 3
    };

  public:
    using contract::contract;

    ACTION transfer( name   from,
                     name   to,
                     asset  quantity,
                     string memo );

    signup( name receiver, name code, datastream<const char*> ds ):
      contract( receiver, code, ds ) {}
};