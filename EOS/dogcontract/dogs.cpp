#include <dogs.contract/dogs.hpp>

/*
** Function insert data into table by ID.
** Owner can not be changed in this function.
*/
ACTION dogs::insdog(name owner, ds_string dog_name, ds_string dog_img, uint8_t dog_age) {
  //Require auth of the owner
  require_auth(owner);
  //check balance of sender/owner_index
  check_balance(owner, DOG_INS_QTY);
  //reduce balance
  sub_balance(owner, DOG_INS_QTY);
  //check variables
  check(dog_name.length() > MIN_NAME_SIZE, "Dog name must have at least 3 characters long.");
  check(dog_name.length() > MAX_NAME_SIZE, "Dog name should be less than 12 characters long.");
  check(dog_img.length() <= MAX_IMG_SIZE, "Dog img link should be less than 64 characters long.");
  //Get the index of our table
  dog_index dogtable(get_self(), get_self().value); //this is an scope);
  //Execute the insert function. Specifying the dog_name,age and img link,
  //payer of storage and a lambda function
  dogtable.emplace(owner, [&](auto& target) {
    target.id = dogtable.available_primary_key();
    target.owner = owner;
    //create a dogrow and set at the end(push_back) values
    target.rows.push_back({
      dog_img,
      dog_age
      });
    target.created_at = current_time_point();
    });

};//end ACTION insdog

/*
** Function modifies the name and age of a dog with the dog_id.
** Owner can not be changed in this function.
*/
ACTION dogs::moddog(int dog_id, ds_string dog_name, ds_string dog_img, uint8_t dog_age){
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
      dog_img,
      dog_age
    });
  });
};//end ACTION modify

/*
** Function delete dog by dog_id.
** Owner can not be changed in this function.
*/
ACTION dogs::rmdog (int dog_id) {
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
};//end ACTION erase

/*
** Function remove all dogs by Owner.
** Owner can not be changed in this function.
*/
ACTION dogs::rmadog(name user) {
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
};//end ACTION rmadog

//check balance for owner
void dogs::check_balance(name user, int payQty){
  //scope to get balance value
  balance_index balances(get_self(), user.value);
  //get balance from account, if empty throw error msg
  auto row = balances.get(currency_symbol.raw(), "DOGCOIN balance is empty.");
  //check
  check(row.funds.amount >= payQty, "Not enough DOGCOINS Deposited.");
};//end check balance

//charge the amount to of token to pay in order to execute an action
void dogs::sub_balance(name user, int payQty){
  //scope to get balance value
  balance_index balances(get_self(), user.value);
  //find currency symbol balance
  auto iterator = balances.find(currency_symbol.raw());
  //if there is a balance in account
  if(iterator != balances.end()){
    //charge/reduce balance for action performed
    balances.modify(iterator, get_self(), [&](auto &row){
      row.funds.set_amount(row.funds.amount - payQty);
    });
  }//end if
};//end reduce_balance

/*
** function to trigger by listening other contract
*/
[[eosio::on_notify("eosio.token::transfer")]]
void dogs::deposit(name from, name to, asset quantity, ds_string memo){
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
