#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

CONTRACT dogcontract : public contract {
  public:
    using contract::contract;
    dogcontract(name receiver, name code, datastream<const char*> ds):contract(receiver, code, ds)
     , currency_symbol("DOGCOIN", 0){}
    /*
    ** Function insert the name and age of a dog with the dog_id.
    ** Owner can not be changed in this function.
    */
    ACTION insert(name owner, std::string dog_name, int age){
      //Require auth of the owner
      require_auth(owner);
      //check balance of sender/owner_index
      check_balance(owner);
      //reduce balance
      reduce_balance(owner);

      //Get the index of our table
      dog_index dogs(get_self(), get_self().value); //this is an scope
      //Execute the insert function. Specifying the dog_name and age,
      //payer of storage and a lambda function.
      dogs.emplace(owner, [&](auto& row){
        row.id = dogs.available_primary_key();
        row.dog_name = dog_name;
        row.age = age;
        row.owner = owner;
      });
    }//end ACTION insert

      /*
      ** Function delete dog by dog_id.
      ** Owner can not be changed in this function.
      */
      ACTION erase(int dog_id){
        //Get the index of our table
        dog_index dogs(get_self(), get_self().value); //this is an scope
        //Fetch the current data of our dog
        auto dog = dogs.get(dog_id, "Unable to fetch dog.");
        //Require auth of the owner
        require_auth(dog.owner);
        //Get the iterator to be able to find the row in the table
        auto iterator = dogs.find(dog_id);
        //Execute the erase function. Specifying the iterator,
        dogs.erase(iterator);
      }//end ACTION erase

      /*
      ** Function modifies the name and age of a dog with the dog_id.
      ** Owner can not be changed in this function.
      */
      ACTION modify(int dog_id, std::string dog_name, int age){
        //Get the index of our table
        dog_index dogs(get_self(), get_self().value);
        //Fetch the current data of our dog
        auto dog = dogs.get(dog_id, "Unable ot fetch dog.");
        //Require auth of the owner
        require_auth(dog.owner);
        //Get the iterator to be able to find and modify the row in the table
        auto iterator = dogs.find(dog_id);
        //Execute the modify function. Specifying the iterator,
        //payer of storage and a lambda function.
        dogs.modify(iterator, dog.owner, [&](auto& row){
        row.dog_name = dog_name;
        row.age = age;
        });
      }//end ACTION modify

      /*
      ** Function remove all dogs by Owner.
      ** Owner can not be changed in this function.
      */
      ACTION removeall(name user){
        //Get the index of our table
        dog_index dogs(get_self(), get_self().value);
        //Fetch the current data of dogs by owner
        auto owner_index = dogs.get_index<"byowner"_n>();
        //Get the iterator to be able to find the row in the table
        auto iterator = owner_index.find(user.value);
        while(iterator != owner_index.end()){
          //delete row
          owner_index.erase(iterator);
          iterator = owner_index.find(user.value);
        }
      }//end ACTION removeall
      /*
      ** function to trigger by listening other contract
      */
      [[eosio::on_notify("eosio.token::transfer")]]
      void pay(name from, name to, asset quantity, std::string memo){
        //filter to listen other than SELF
        if(from == get_self() || to != get_self()){return;}
        //check quantity is more than 0
        check(quantity.amount > 0, "Not enough coins.");
        //check if currency symbol is correct
        check(quantity.symbol == currency_symbol, "Not the right coin.");
        //individual scope, get only balance by active account
        balance_index balance(get_self(), from.value);
        auto iterator = balance.find(currency_symbol.raw());
        //reach end balance table, modify balance instead insert a new row
        if(iterator != balance.end()){
          balance.modify(iterator, get_self(), [&](auto &row){
            row.funds += quantity;
          });
        }
        //no deposits before, 1st deposit insert a row
        else{
          balance.emplace(get_self(), [&](auto &row){
            row.funds = quantity;
          });
        }
      }//end void pay

    private:
      const symbol currency_symbol; //to check if payment match our currency symbol

      //table struct dog data
      TABLE dog{
        int id; //unique ID for index
        std::string dog_name;
        int age;
        name owner; //eos account name, owner of dogs
        //get primary key by ID variable
        uint64_t primary_key() const{return id;}
        //get dogs by owner index
        uint64_t by_owner() const{return owner.value;}
      };//end table dog

      //table struct balance, used to manage account funds
      TABLE balance {
        asset funds; //asset is EOS type for tokens
        //primary key for table, never used but need it
        uint64_t primary_key() const{return funds.symbol.raw();}
      };//end table balance

      //define table type index
      typedef multi_index<"dogs"_n, dog, indexed_by<"byowner"_n, const_mem_fun<dog, uint64_t, &dog::by_owner>>> dog_index;
      //index for balance table
      typedef multi_index<"balances"_n, balance> balance_index;

      //check balance for owner
      void check_balance(name user){
        //scope to get balance value
        balance_index balances(get_self(), user.value);
        //get balance from account, if empty throw error msg
        auto row = balances.get(currency_symbol.raw(), "DOGCOIN balance is empty.");
        //check
        check(row.funds.amount >= 10, "Not enough DOGCOINS Deposited.");
      };//end check balance

      void reduce_balance(name user){
        //scope to get balance value
        balance_index balances(get_self(), user.value);
        //find currency symbol balance
        auto iterator = balances.find(currency_symbol.raw());
        //if there is a balance in account
        if(iterator != balances.end()){
          //charge/reduce balance for action performed
          balances.modify(iterator, get_self(), [&](auto &row){
            row.funds.set_amount(row.funds.amount-10);
          });
        }//end if
      };//end reduce_balance
};
