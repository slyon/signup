#include "signup.hpp"
#include "abieos_numeric.hpp"

asset buyrambytes(uint32_t bytes) {
  eosiosystem::rammarket market(name("eosio"), name("eosio").value);
  auto itr = market.find(RAMCORE_S.raw());
  eosio::check(itr != market.end(), "RAMCORE market not found");
  auto tmp = *itr;
  return tmp.convert(asset(bytes, RAM_S), EOS_S);
}