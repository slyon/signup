#include <boost/test/unit_test.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/chain/wast_to_wasm.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fc/log/logger.hpp>
#include <eosio/chain/exceptions.hpp>
#include <Runtime/Runtime.h>

#include "tester.hpp"
struct _abi_hash {
   name owner;
   fc::sha256 hash;
};
FC_REFLECT( _abi_hash, (owner)(hash) );

struct connector {
   asset balance;
   double weight = .5;
};
FC_REFLECT( connector, (balance)(weight) );

using namespace signup;

BOOST_AUTO_TEST_SUITE(signup_tests)

bool within_one(int64_t a, int64_t b) { return std::abs(a - b) <= 1; }

BOOST_FIXTURE_TEST_CASE( buy_and_claim, signup_tester ) try {
   cross_15_percent_threshold();
   // prepare liquid balance
   transfer( "eosio", "alice1111111", core_sym::from_string("100.0000"), "eosio" );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("100.0000"), get_balance( "alice1111111" ) );
   transfer( "eosio", "cointreasury", core_sym::from_string("100.0000"), "eosio" ); //liquid
   transfer( "eosio", "cointreasury", core_sym::from_string("10.0000"), "eosio" ); //RAM
   BOOST_REQUIRE_EQUAL( core_sym::from_string("110.0000"), get_balance( "cointreasury" ) );

   // prepare RAM and stake
   BOOST_REQUIRE_EQUAL( success(), buyram( "cointreasury", "cointreasury", core_sym::from_string("10.0000") ) );
   auto stake_cointreasury = stake( "eosio", "cointreasury", core_sym::from_string("10.0000"), core_sym::from_string("10.0000") );
   BOOST_REQUIRE_EQUAL( success(), stake_cointreasury );

   // create "Basic" (01) account
   name new_acc = name("jimmyparker1");
   std::string memo = "01:"+new_acc.to_string()+":EOS7QaaUfuxjGzW4mYs6LoQBuEhVEh2sJXLXzoQHhx6HsKDuHNJsv:EOS77mv92nFMXGqSTU6WDFrNAJzfNuBcr7wanr5ewag4jUs4npfbK";
   auto tx = transfer_tok( N(alice1111111), N(cointreasury), core_sym::from_string("1.9999"), memo);
   BOOST_REQUIRE_EQUAL(wasm_assert_msg("Price too low"), tx);
   tx = transfer_tok( N(alice1111111), N(cointreasury), core_sym::from_string("2.0001"), "INVALID MEMO");
   BOOST_REQUIRE_EQUAL(wasm_assert_msg("Malformed Memo (invalid length)"), tx);
   //TODO: check more error cases
   tx = transfer_tok( N(eosio), new_acc, core_sym::from_string("2.0000"), "");
   BOOST_REQUIRE_EQUAL(wasm_assert_msg("to account does not exist"), tx);
   tx = transfer_tok( N(alice1111111), N(cointreasury), core_sym::from_string("2.0000"), memo);
   BOOST_REQUIRE_EQUAL(success(), tx);

   // verify new_acc was successfuly created with correct stake
   tx = transfer_tok( N(eosio), new_acc, core_sym::from_string("1.0000"), "");
   BOOST_REQUIRE_EQUAL(success(), tx);
   auto new_res = get_total_stake( new_acc );
   BOOST_REQUIRE_EQUAL(new_res["owner"], new_acc.to_string());
   BOOST_REQUIRE_EQUAL(new_res["net_weight"], core_sym::from_string("0.1000").to_string());
   BOOST_REQUIRE_EQUAL(new_res["cpu_weight"], core_sym::from_string("0.2000").to_string());
   BOOST_REQUIRE_EQUAL(new_res["ram_bytes"], "3051"); //TODO: verify which (exact) amount is needed

   // verify new_acc does not own any stake (only delegated resources)
   tx = unstake(new_acc.to_string(), core_sym::from_string("0.0001"), core_sym::from_string("0.0000"));
   BOOST_REQUIRE_EQUAL(wasm_assert_msg("insufficient staked net bandwidth"), tx);
   tx = unstake(new_acc.to_string(), core_sym::from_string("0.0000"), core_sym::from_string("0.0001"));
   BOOST_REQUIRE_EQUAL(wasm_assert_msg("insufficient staked cpu bandwidth"), tx);
   // verify delegated stake
   auto del_stake = get_dbw_obj( N(cointreasury), new_acc );
   BOOST_REQUIRE_EQUAL(del_stake["from"], "cointreasury");
   BOOST_REQUIRE_EQUAL(del_stake["to"], new_acc.to_string());
   BOOST_REQUIRE_EQUAL(del_stake["net_weight"], core_sym::from_string("0.1000").to_string());
   BOOST_REQUIRE_EQUAL(del_stake["cpu_weight"], core_sym::from_string("0.2000").to_string());

   // claim stake
   BOOST_REQUIRE_EQUAL( wasm_assert_msg("No stake found for this account."), signup_claim( N(alice1111111) ) );
   BOOST_REQUIRE_EQUAL( success(), signup_claim( new_acc ) );

   // delegate some system resources, so we can unstake the owned resources
   auto stake_new = stake( "eosio", new_acc.to_string(), core_sym::from_string("10.0000"), core_sym::from_string("10.0000") );
   BOOST_REQUIRE_EQUAL( success(), stake_new );
   // un-stake the (now owned) resources
   BOOST_REQUIRE_EQUAL( success(), unstake( new_acc, core_sym::from_string("0.1000"), core_sym::from_string("0.2000") ) );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( claim_and_refund, signup_tester ) try {
   cross_15_percent_threshold();
   // prepare liquid balance
   transfer( "eosio", "alice1111111", core_sym::from_string("100.0000"), "eosio" );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("100.0000"), get_balance( "alice1111111" ) );
   transfer( "eosio", "cointreasury", core_sym::from_string("6.0000"), "eosio" ); //liquid
   transfer( "eosio", "cointreasury", core_sym::from_string("10.0000"), "eosio" ); //RAM
   BOOST_REQUIRE_EQUAL( core_sym::from_string("16.0000"), get_balance( "cointreasury" ) );

   // prepare RAM and stake
   BOOST_REQUIRE_EQUAL( success(), buyram( "cointreasury", "cointreasury", core_sym::from_string("10.0000") ) );
   auto stake_cointreasury = stake( "eosio", "cointreasury", core_sym::from_string("10.0000"), core_sym::from_string("10.0000") );
   BOOST_REQUIRE_EQUAL( success(), stake_cointreasury );

   // prepare stake of user accounts
   auto stake_user = stake( "cointreasury", "alice1111111", core_sym::from_string("1.0000"), core_sym::from_string("1.0000"));
   BOOST_REQUIRE_EQUAL( success(), stake_user );
   stake_user = stake( "cointreasury", "bob111111111", core_sym::from_string("1.0000"), core_sym::from_string("1.0000"));
   BOOST_REQUIRE_EQUAL( success(), stake_user );

   // try non-available refund
   BOOST_REQUIRE_EQUAL( wasm_assert_msg("No refund request found."), signup_refund( N(alice1111111) ) );

   // claim bob's stake
   BOOST_REQUIRE_EQUAL( core_sym::from_string("2.0000"), get_balance( "cointreasury" ) );
   BOOST_REQUIRE_EQUAL( success(), signup_claim( N(bob111111111) ) );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("0.0000"), get_balance( "cointreasury" ) );

   // claim alice's stake (needs refund of bob's stake)
   BOOST_REQUIRE_EQUAL( wasm_assert_msg("overdrawn balance"), signup_claim( N(alice1111111) ) );
   BOOST_REQUIRE_EQUAL( wasm_assert_msg("Refund is not available yet."), signup_refund( N(alice1111111) ) );
   produce_block( fc::hours(3*24) ); // after refund available
   BOOST_REQUIRE_EQUAL( success(), signup_refund( N(alice1111111) ) );
   BOOST_REQUIRE_EQUAL( success(), signup_claim( N(alice1111111) ) );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("0.0000"), get_balance( "cointreasury" ) );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( proxy_staking_rewards, signup_tester ) try {
   cross_15_percent_threshold();
   // prepare liquid balance
   transfer( "eosio", "alice1111111", core_sym::from_string("100.0000"), "eosio" );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("100.0000"), get_balance( "alice1111111" ) );
   transfer( "eosio", "cointreasury", core_sym::from_string("100.0000"), "eosio" ); //liquid
   transfer( "eosio", "cointreasury", core_sym::from_string("10.0000"), "eosio" ); //RAM
   BOOST_REQUIRE_EQUAL( core_sym::from_string("110.0000"), get_balance( "cointreasury" ) );

   // prepare RAM and stake
   BOOST_REQUIRE_EQUAL( success(), buyram( "cointreasury", "cointreasury", core_sym::from_string("10.0000") ) );
   auto stake_cointreasury = stake( "eosio", "cointreasury", core_sym::from_string("10.0000"), core_sym::from_string("10.0000") );
   BOOST_REQUIRE_EQUAL( success(), stake_cointreasury );

   // prepare reward proxy accounts
   create_accounts_with_resources( {  N(genereospool), N(proxy4nation), N(vproxyrewrd1), N(vproxyrewrd2), N(vproxyrewrd3) } );
   transfer( "eosio", "genereospool", core_sym::from_string("10.0000"), "eosio" ); //liquid
   transfer( "eosio", "proxy4nation", core_sym::from_string("10.0000"), "eosio" ); //liquid
   transfer( "eosio", "vproxyrewrd1", core_sym::from_string("10.0000"), "eosio" ); //liquid
   transfer( "eosio", "vproxyrewrd2", core_sym::from_string("10.0000"), "eosio" ); //liquid
   transfer( "eosio", "vproxyrewrd3", core_sym::from_string("10.0000"), "eosio" ); //liquid

   // reject invalid (non-whitelisted) transfer
   auto tx = transfer_tok( N(alice1111111), N(cointreasury), core_sym::from_string("0.1000"), "proxy rewards");
   BOOST_REQUIRE_EQUAL( wasm_assert_msg("Malformed Memo (invalid length)"), tx );
   // accept valid transaction of whitelisted proxy accounts
   tx = transfer_tok( N(genereospool), N(cointreasury), core_sym::from_string("0.1000"), "Proxy rewards from colinrewards day 18302 [powered by https://genpool.io]");
   BOOST_REQUIRE_EQUAL( success(), tx );
   tx = transfer_tok( N(proxy4nation), N(cointreasury), core_sym::from_string("0.1000"), "StrengthEARN the network `proxy4nation` via https://proxy.eosnation.io");
   BOOST_REQUIRE_EQUAL( success(), tx );
   // accept valid CPU stake of whitelisted proxy accounts
   tx = stake( "vproxyrewrd1", "cointreasury", core_sym::from_string("0.0000"), core_sym::from_string("0.1000") );
   BOOST_REQUIRE_EQUAL( success(), tx );
   tx = stake( "vproxyrewrd2", "cointreasury", core_sym::from_string("0.0000"), core_sym::from_string("0.1000") );
   BOOST_REQUIRE_EQUAL( success(), tx );
   tx = stake( "vproxyrewrd3", "cointreasury", core_sym::from_string("0.0000"), core_sym::from_string("0.1000") );
   BOOST_REQUIRE_EQUAL( success(), tx );
} FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
