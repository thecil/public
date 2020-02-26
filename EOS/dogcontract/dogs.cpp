#include <vector>
#include <string>
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>


uint8_t dog_hp = 100;

using namespace std;
using namespace eosio;

#include "table/user/dog.hpp"


CONTRACT dogs : public contract {
  public:
    using contract::contract;
    dogs(name receiver, name code, datastream<const char*> ds)
    :contract(receiver, code, ds){}

  /*
  ** Function insert data into table by ID.
  ** Owner can not be changed in this function.
  */
  ACTION insdog(name owner, std::string dog_name, std::string dog_img, uint8_t dog_age) {
    //get time microseconds
    auto dog_time = time_point(current_time_point()); 
    //Require auth of the owner
    require_auth(owner);
    //Get the index of our table
    dog_index dogtable(get_self(), get_self().value); //this is an scope);
    //Execute the insert function. Specifying the dog_name,age and img link,
    //payer of storage and a lambda function
    dogtable.emplace(owner, [&](auto& target) {
      target.id = dogtable.available_primary_key();
      target.owner = owner;
      //create a dogrow and set at the end(push_back) values
      target.rows.push_back({
        dog_name,
        dog_img,
        dog_age,
        dog_hp
      });
      target.created_at = dog_time;
    });

  }//end ACTION insdog

  /*
  ** Function modifies the name and age of a dog with the dog_id.
  ** Owner can not be changed in this function.
  */
  ACTION moddog(int dog_id, std::string dog_name, std::string dog_img, uint8_t dog_age){
    //Get the index of our table
    dog_index dogtable(get_self(), get_self().value);
    //Fetch the current data of our dog
    auto dog = dogtable.get(dog_id, "Unable ot fetch dog.");
    //Require auth of the owner
    require_auth(dog.owner);
    //Get the iterator to be able to find and modify the row in the table
    auto iterator = dogtable.find(dog_id);
    //Execute the modify function. Specifying the iterator,
    //payer of storage and a lambda function.
    dogtable.modify(iterator, dog.owner, [&](auto& target){
      //clear vector before new data
      target.rows.clear();
      //insert new data
      target.rows.push_back({
        dog_name,
        dog_img,
        dog_age,
        dog_hp
      });
      
    });
  }//end ACTION modify
  
  /*
  ** Function delete dog by dog_id.
  ** Owner can not be changed in this function.
  */
  ACTION rmdog (int dog_id) {
    //Get the index of our table
    dog_index dogtable(get_self(), get_self().value); //this is an scope
    //Fetch the current data of our dog
    auto dog = dogtable.get(dog_id, "Unable to fetch dog.");
    //Require auth of the owner
    require_auth(dog.owner);
    //Get the iterator to be able to find the row in the table
    auto iterator = dogtable.find(dog_id);
    //Execute the erase function. Specifying the iterator,
    dogtable.erase(iterator);
  }//end ACTION erase

  /*
  ** Function remove all dogs by Owner.
  ** Owner can not be changed in this function.
  */
  ACTION rmadog(name user) {
    //Get the index of our table
    dog_index dogtable(get_self(), get_self().value);
    //Fetch the current data of dogs by owner
    auto owner_index = dogtable.get_index<"byowner"_n>();
    //Get the iterator to be able to find the row in the table
    auto iterator = owner_index.find(user.value);
    while(iterator != owner_index.end()){
      //delete row
      owner_index.erase(iterator);
      iterator = owner_index.find(user.value);
    }
  }//end ACTION rmadog

/*

 // Find the record from  table
  ACTION getdog(name from){
    
    
    //Get the index of our table
    dog_index dogtable(get_self(), get_self().value); //this is an scope
    //Fetch the current data of our dog
    auto dog = dogtable.get(dog_id, "Unable to fetch dog.");
    //Require auth of the owner
    require_auth(dog.owner);


  }

    
  //table accounts data
  TABLE account{
    int id; //unique ID for index
    name owner; //eos account name owner of this account
    string dog_name;
    //get primary key by ID variable
    uint64_t primary_key() const{return id;}
  };//end account table
  */
  
  //table struct dog data
  TABLE dog{
    int id; //unique ID for index
    name owner; //eos account name, owner of dogs
    std::vector<dogrow> rows; //get and set all data from struct dog_stats.hpp
    //get primary key by ID variable
    uint64_t primary_key() const{return id;}
    //get dogs by owner index
    uint64_t by_owner() const{return owner.value;}
    time_point_sec created_at;
    time_point_sec block_at;
  };//end table dog

  //define table type index
  typedef multi_index<"dogtable"_n, dog, indexed_by<"byowner"_n, const_mem_fun<dog, uint64_t, &dog::by_owner>>> dog_index;

};
