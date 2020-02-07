#include <vector>
#include <string>
#include <eosio/eosio.hpp>

using namespace std;
using namespace eosio;

#include "table/user/dog.hpp"

CONTRACT dogs : public contract {
  public:
    using contract::contract;
    dogs(name receiver, name code, datastream<const char*> ds)
    :contract(receiver, code, ds){}

  ACTION insdog(name from, std::string dog_name, std::string dog_img, uint8_t dog_age) {
    require_auth(from);


    // Init the _message table
    dog_index dogtable(get_self(), get_self().value); //this is an scope);

      dogtable.emplace(from, [&](auto& target) {
        target.id = dogtable.available_primary_key();
        target.owner = from;
        target.rows.push_back({
          dog_name,
          dog_img,
          dog_age
        });
      });

  }

  ACTION rmdog() {
    require_auth(get_self());

    dog_index dogtable(get_self(), get_self().value);

    // Delete all records in _messages table
    auto dog_itr = dogtable.begin();
    while (dog_itr != dogtable.end()) {
      dog_itr = dogtable.erase(dog_itr);
    }
  }

  ACTION getdog(name from){
  // Find the record from _messages table
    //auto dog_itr = dogtable.find(from.value);
  }

  //table struct dog data
  TABLE dog{
    int id; //unique ID for index
    name owner; //eos account name, owner of dogs
    std::vector<dogrow> rows; //get and set all data from struct dog_stats.hpp

    //get primary key by ID variable
    uint64_t primary_key() const{return id;}

    //get dogs by owner index
    uint64_t by_owner() const{return owner.value;}
  };//end table dog

  //define table type index
  typedef multi_index<"dogtable"_n, dog, indexed_by<"byowner"_n, const_mem_fun<dog, uint64_t, &dog::by_owner>>> dog_index;

};
