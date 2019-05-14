#include "extra.cpp"

// Notify a partner about this newly created account
void signup::notify(name new_account) {
  require_auth(get_self());
  require_recipient(PARTNER);
}

void signup::on_transfer( name from, name to, asset quantity, string memo ) {
  // do nothing on outgoing transfers
  if (from == _self || to != _self) {
    return;
  }

  // do nothing on transfers from our reference account, or system accounts:
  // eoscanada.com/en/what-are-eosio-system-accounts-and-what-do-they-each-do
  if ( from == REFERENCE || from == name("eosio") || from == name("eosio.bpay") ||
       from == name("eosio.msig") || from == name("eosio.names") ||
       from == name("eosio.prods") || from == name("eosio.ram") ||
       from == name("eosio.ramfee") || from == name("eosio.saving") ||
       from == name("eosio.stake") || from == name("eosio.token") ||
       from == name("eosio.unregd") || from == name("eosio.vpay") ||
       from == name("eosio.wrap") || from == name("eosio.rex") ) {
    return;
  }

  eosio::check(quantity.symbol == TLOS_S, "Must be payed in TLOS");
  eosio::check(quantity.is_valid(), "Invalid token transfer");
  eosio::check(quantity.amount > 0, "Quantity must be positive");

  /**
   * Parse Memo
   * Memo must have format "plan_id:account_name:owner_key:active_key"
   */
  eosio::check(memo.length() == 123 || memo.length() == 69, "Malformed Memo (invalid length)");
  eosio::check(memo[2] == ':', "Malformed Memo ([2] == :)");
  eosio::check(memo[15] == ':', "Malformed Memo ([15] == :)");

  const string owner_key_str = memo.substr(16, 53);
  string active_key_str;
  if(memo[69] == ':') {
    // active key provided
    active_key_str = memo.substr(70, 53);
  } else {
    // active key is the same as owner
    active_key_str =  owner_key_str;
  }

  const abieos::public_key owner_pubkey =
      abieos::string_to_public_key(owner_key_str);
  const abieos::public_key active_pubkey =
      abieos::string_to_public_key(active_key_str);

  copy(owner_pubkey.data.begin(), owner_pubkey.data.end(),
       owner_pubkey_char.begin());
  copy(active_pubkey.data.begin(), active_pubkey.data.end(),
       active_pubkey_char.begin());

  //TODO: verify "EOS" prefix of keys
  //TODO: verify checksum (ripemd160())

  const string plan_str = memo.substr(0, 2);
  const int plan_id = stoi(plan_str);

  const string new_name_str = memo.substr(3, 12);
  const name new_name = name(new_name_str.c_str());

  const auto owner_auth = eosiosystem::authority{1, {{{(uint8_t)abieos::key_type::k1, owner_pubkey_char}, 1}}, {}, {}};
  const auto active_auth = eosiosystem::authority{1, {{{(uint8_t)abieos::key_type::k1, active_pubkey_char}, 1}}, {}, {}};

  asset cpu;
  asset net;
  asset ram;
  asset fee;
  const asset ram_replace_amount = buyrambytes(256);
  if(plan_id == 1) {
    eosio::check(quantity.amount >= 100000, "Pice too low");
    cpu = asset(9000, TLOS_S);
    net = asset(1000, TLOS_S);
    ram = buyrambytes(3 * 1024);
    fee = asset(10000, TLOS_S);
  } else if(plan_id == 2) {
    eosio::check(quantity.amount >= 500000, "Pice too low");
    cpu = asset(25000, TLOS_S);
    net = asset(5000, TLOS_S);
    ram = buyrambytes(4 * 1024);
    fee = asset(40000, TLOS_S);
  } else if(plan_id == 3) {
    eosio::check(quantity.amount >= 1000000, "Pice too low");
    cpu = asset(50000, TLOS_S);
    net = asset(10000, TLOS_S);
    ram = buyrambytes(8 * 1024);
    fee = asset(70000, TLOS_S);
  } else {
    eosio::check(false, "Invalid plan ID");
  }

  eosio::check(cpu + net + ram + fee + ram_replace_amount <= quantity, "Not enough money");

  const auto remaining_balance = quantity - cpu - net - ram - fee - ram_replace_amount;

  // create account
  action(
    permission_level{ get_self(), "active"_n },
    "eosio"_n,
    "newaccount"_n,
    std::make_tuple(_self, new_name, owner_auth, active_auth)//{_self, new_name, owner_auth, active_auth}//user//std::make_tuple( user )
  ).send();

  // buy ram
  action(
    permission_level{ get_self(), "active"_n },
    "eosio"_n,
    "buyram"_n,
    std::make_tuple(_self, new_name, ram)
  ).send();

  // replace lost ram
  action(
    permission_level{ get_self(), "active"_n },
    "eosio"_n,
    "buyram"_n,
    std::make_tuple(_self, _self, ram_replace_amount)
  ).send();

  // delegate and transfer cpu and net
  action(
    permission_level{ get_self(), "active"_n },
    "eosio"_n,
    "delegatebw"_n,
    std::make_tuple(_self, new_name, net, cpu, 1)
  ).send();

  // fee
  action(
    permission_level{ get_self(), "active"_n },
    "eosio.token"_n,
    "transfer"_n,
    std::make_tuple(_self, REFERENCE, fee, std::string("Account creation fee"))
  ).send();

  if (remaining_balance.amount > 0) {
    // transfer remaining balance to new account
    action(
      permission_level{ get_self(), "active"_n },
      "eosio.token"_n,
      "transfer"_n,
      std::make_tuple(_self, new_name, remaining_balance, std::string("Initial balance"))
    ).send();
  }

  // Notify partner
  action(
    permission_level{ get_self(), "active"_n },
    get_self(),
    "notify"_n,
    std::make_tuple(new_name)
  ).send();
}
