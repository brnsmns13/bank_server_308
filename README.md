CprE 308 Project 2
==================
Requests
--------------------
1. Transaction
  * immediate resonse: ID <request_id> 
  * input: TRANS <acct1> <amount1> <acct2> <amount2> ... 
  * up to 10 transactions per input 
  * file response: 
    + if success: <request_id> OK
    + if failure: <request_id> ISF <acct_id>

2. Balance Check
  * immediate resonse: ID <request_id>
  * input: CHECK <account_id>
  * file response: <request_id> BAL <balance>

3. Exit
  * immediate response: EXIT <request_id>
  * file response: EXIT <request_id>

   request_id
   - incremented by 1 for every request made
   - need a mutex