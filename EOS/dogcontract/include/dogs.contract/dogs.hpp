#pragma once

#include <vector>
#include <string>
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

#define ACTION [[eosio::action]] void

#ifndef ds_symbol
#define ds_symbol eosio::symbol
#endif

#ifndef ds_string
#define ds_string std::string
#endif

#define MAX_NAME_SIZE 12
#define MIN_NAME_SIZE 2
#define MAX_IMG_SIZE 64

#define DOG_SYMBOL_STR "DOGCOIN"
#define DOG_SYMBOL_DECIMAL 0

#define DOG_INS_QTY 10
#define DOG_RM_QTY 20

#include "table/dog_struct.hpp"

CONTRACT dogs : public contract {
  public:
    using contract::contract;
    dogs(name receiver, name code, datastream<const char*> ds)
    :contract(receiver, code, ds)
    ,currency_symbol(DOG_SYMBOL_STR, DOG_SYMBOL_DECIMAL) {}

    ACTION insdog(name owner
        , ds_string dog_name
        ,ds_string dog_img
        ,uint8_t dog_age
    );

    ACTION moddog(int dog_id
        ,ds_string dog_name
        ,ds_string dog_img
        ,uint8_t dog_age
    );

    ACTION rmdog (
        int dog_id
    );

    ACTION rmadog(
        name user
    );

    void check_balance(name user, int payQty);
    void sub_balance(name user, int payQty);

    [[eosio::on_notify("eosio.token::transfer")]]
    void deposit(
        name from
        ,name to
        ,asset quantity
        ,ds_string memo
    );

  private:

    //only accept payments that match the same currency
    const symbol currency_symbol;

    //table struct dog data
    TABLE dog{
      int id; //unique ID for index
      name owner; //eos account name, owner of dogs
      std::vector<dogrow> rows; //get and set all data from struct dog_stats.hpp
      std::string dog_name;
      time_point_sec created_at; //creation date

      uint64_t primary_key() const{return id;}//get primary key by ID variable
      uint64_t by_owner() const{return owner.value;}//get dogs by owner index
    };

    //table struct balance, used to manage account funds
    TABLE balance {
      asset funds; //asset is EOS type for tokens

      //primary key for table, never used but need it
      uint64_t primary_key() const{return funds.symbol.raw();}
    };//end table balance

    //define table type index
    typedef multi_index<"dogtable"_n, dog, indexed_by<"byowner"_n, const_mem_fun<dog, uint64_t, &dog::by_owner>>> dog_index;
    typedef multi_index<"balances"_n, balance> balance_index;


};//end CONTRACT
