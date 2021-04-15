# Query Generator

## To run
Use python to run the query generator.

`python3 query_generator.py <optional arguments>`

where `<optional arguments>` are optional arguments listed below used to adjust the queries generated.

## Suggested usage
Presets are available for ease of use, system testing and to generate queries which more likely are valid.

Below is the list of presets with the following params:
 - `num_queries` = 20
 - `len_queries` = 2
 - `num_syn` = 5
 - `iden_len` = 2
 - `and_limit` = 2

| Command | Description |
| --- | --- |
| `--preset 1 0` | BOOLEAN select, test iter2 queries |
| `--preset 2 0` | tuple select, test iter2 queries |
| `--preset 3 0` | single select, test iter2 queries |
| `--preset 1 1` | BOOLEAN select, Follows |
| `--preset 1 2` | BOOLEAN select, FollowsT |
| `--preset 1 3` | BOOLEAN select, Parent |
| `--preset 1 4` | BOOLEAN select, ParentT |
| `--preset 1 5` | BOOLEAN select, Calls |
| `--preset 1 6` | BOOLEAN select, CallsT |
| `--preset 1 7` | BOOLEAN select, UsesP/S |
| `--preset 1 8` | BOOLEAN select, ModifiesP/S |
| `--preset 1 9` | BOOLEAN select, Next |
| `--preset 1 10` | BOOLEAN select, NextT |
| `--preset 1 11` | BOOLEAN select, Affects |
| `--preset 1 12` | BOOLEAN select, AffectsT |
| `--preset 1 13` | BOOLEAN select, pattern-assign |
| `--preset 1 14` | BOOLEAN select, pattern-if |
| `--preset 1 15` | BOOLEAN select, pattern-while |
| `--preset 1 16` | BOOLEAN select, with |
| `--preset 2 1` | tuple select, Follows |
| `--preset 2 2` | tuple select, FollowsT |
| `--preset 2 3` | tuple select, Parent |
| `--preset 2 4` | tuple select, ParentT |
| `--preset 2 5` | tuple select, Calls |
| `--preset 2 6` | tuple select, CallsT |
| `--preset 2 7` | tuple select, UsesP/S |
| `--preset 2 8` | tuple select, ModifiesP/S |
| `--preset 2 9` | tuple select, Next |
| `--preset 2 10` | tuple select, NextT |
| `--preset 2 11` | tuple select, Affects |
| `--preset 2 12` | tuple select, AffectsT |
| `--preset 2 13` | tuple select, pattern-assign |
| `--preset 2 14` | tuple select, pattern-if |
| `--preset 2 15` | tuple select, pattern-while |
| `--preset 2 16` | tuple select, with |
| `--preset 3 1` | single select, Follows |
| `--preset 3 2` | single select, FollowsT |
| `--preset 3 3` | single select, Parent |
| `--preset 3 4` | single select, ParentT |
| `--preset 3 5` | single select, Calls |
| `--preset 3 6` | single select, CallsT |
| `--preset 3 7` | single select, UsesP/S |
| `--preset 3 8` | single select, ModifiesP/S |
| `--preset 3 9` | single select, Next |
| `--preset 3 10` | single select, NextT |
| `--preset 3 11` | single select, Affects |
| `--preset 3 12` | single select, AffectsT |
| `--preset 3 13` | single select, pattern-assign |
| `--preset 3 14` | single select, pattern-if |
| `--preset 3 15` | single select, pattern-while |
| `--preset 3 16` | single select, with |

## Optional arguments
| Command | Description |
| --- | --- |
| `--num_files NUM_FILES` | Number of files to generate. Must be >= 1. Default: 1 |
| `--num_queries NUM_QUERIES` | Number of queries to generate per file. Must be >= 0. Default: 50 |
| `--len_queries LEN_QUERIES` | Max length of each query generated. Must be >= 0. Default: 10 |
| `--num_syn NUM_SYN` | Number of synonyms to generate per entity. Must be >= 1. Default: 10 |
| `--num_proc NUM_PROC` | Number of procedures to generate. Uses `num_syn` to determine generated amount when `num_proc` < 0. Default: -1 |
| `--num_progl NUM_PROGL` | Number of program lines to generate. Uses `num_syn` to determine generated amount when `num_progl` < 0. Default: -1 |
| `--num_stmt NUM_STMT` | Number of statements to generate. Uses `num_syn` to determine generated amount when `num_stmt` < 0. Default: -1 |
| `--num_read NUM_READ` | Number of read statements to generate. Uses `num_syn` to  determine generated amount when `num_read` < 0. Default: -1 |
| `--num_print NUM_PRINT` | Number of print statements to generate. Uses `num_syn` to determine generated amount when `num_print` < 0. Default: -1 |
| `--num_call NUM_CALL` | Number of call statements to generate. Uses `num_syn` to determine generated amount when `num_call` < 0. Default: -1 |
| `--num_while NUM_WHILE` | Number of while statements to generate. Uses `num_syn` to determine generated amount when `num_while` < 0. Default: -1 |
| `--num_if NUM_IF` | Number of if statements to generate. Uses `num_syn` to determine generated amount when `num_if` < 0. Default: -1 |
| `--num_assign NUM_ASSIGN` | Number of assign statements to generate. Uses `num_syn` to determine generated amount when `num_assign` < 0. Default: -1 |
| `--num_var NUM_VAR` | Number of variables to generate. Uses `num_syn` to determine generated amount when `num_var` < 0. Default: -1 |
| `--num_const NUM_CONST` | Number of constants to generate. Uses `num_syn` to determine generated amount when `num_const` < 0. Default: -1 |
| `--weight_result WEIGHT_RESULT [WEIGHT_RESULT ...]` | Weight of result clause. Corresponds to frequency of occurrence of tuples and booleans respectively in result-cl. Each value must be of int type >= 0. Enter 2 values exactly. Default: [4, 1]. Example usage: `--weight_result 1 0` to always get tuples. |
| `--weight_tuple WEIGHT_TUPLE [WEIGHT_TUPLE ...]` | Weight of tuples. Corresponds to frequency of occurrence of elem and '<' elem '>' respectively in tuples. Each value must be of int type >= 0. Enter 2 values exactly. Default: [1, 3]. Example usage: `--weight_tuple 0 1` to always get '<' elem '>'. |
| `--weight_elem WEIGHT_ELEM [WEIGHT_ELEM ...]` | Weight of elem. Corresponds to frequency of occurrence of syn and attrRef respectively in elem. Each value must be of int type >= 0. Enter 2 values exactly. Default: [1, 1]. Example usage: `--weight_elem 1 0` to always get syn. |
| `--weight_select WEIGHT_SELECT [WEIGHT_SELECT ...]` | Weight of select clause. Corresponds to frequency of occurrence of suchthat-cl, with-cl and pattern-cl respectively in select-cl. Each value must be of int type >= 0. Enter 3 values exactly. Default: [1, 1, 1] Example usage: `--weight_select 0 0 1` to always get pattern-cl. |
| `--weight_with WEIGHT_WITH [WEIGHT_WITH ...]` | Weight of with clause. Corresponds to frequency of occurrence of IDEN, attrRef and syn respectively in with-cl. Each value must be of int type >= 0. Enter 3 values exactly. Default: [ 1, 1, 1] Example usage: `--weight_with 0 0 1` to always get syn with-cl. |
| `--weight_attr WEIGHT_ATTR [WEIGHT_ATTR ...]` | Weight of attrName in attrRef. Corresponds to frequency of occurrence of procName, varName, value and stmt# of attrRef respectively in with-cl. Each value must be of int type >= 0. Enter 4 values exactly. Default: [1, 1, 1, 1]. Example usage: `--weight_attr 0 0 0 1` to always get stmt#. |
| `--weight_suchthat WEIGHT_SUCHTHAT [WEIGHT_SUCHTHAT ...]` | Weight of suchthat clause. Corresponds to frequency of occurrence of ModifiesS/P, UsesS/P, Calls(T), Parent(T), Follows(T), Next(T) and Affects(T) respectively in suchthat-cl. Each value must be of int type >= 0. Enter 7 values exactly. Default: [1, 1, 1, 1, 1, 1, 1]. Example usage: `--weight_suchthat 0 0 0 0 0 0 1` to always get Affects(T) suchthat-cl. |
| `--weight_transitive WEIGHT_TRANSITIVE [WEIGHT_TRANSITIVE ...]` | Weight of transitive suchthat-cl. Corresponds to the frequency of occurrence of non-transitive and transitive clauses respectively in suchthat-cl. Each value must be of int type >= 0. Enter 2 values exactly. Default: [1, 1]. Example usage: `--weight_transitive 0 1` to always get transitive suchthat-cl. |
| `--weight_pattern WEIGHT_PATTERN [WEIGHT_PATTERN ...]` | Weight of pattern clause. Corresponds to frequency of occurrence of syn-assign, syn-if and syn-while respectively in pattern-cl. Each value must be of int type >= 0. Enter 3 values exactly. Default: [1, 1, 1]. Example usage: `--weight_pattern 0 1 0` to always get syn-if pattern-cl. |
| `--var VAR` | [File] containing variable names. Assumes inputs are correct. Default: None |
| `--proc PROC` | [File] containing procedure names. Assumes inputs are correct. Default: None |
| `--const CONST` | [File] containing constant values. Assumes inputs are correct. Default: None |
| `--delim DELIM` | Delimiter to separate the inputs in files provided. Default: ' ' |
| `--start START` | File number to start generation from. Default: 1 |
| `--out OUT` | [Path] to the output folder to contain the queries. Will create the directory if it does not exist. Default: queries\ |
| `--iden_len IDEN_LEN` | Synonym identity name length. Must be >= 1. Default: 10 |
| `--seed SEED` | RNG seed. Default: 42 |
| `--and_limit AND_LIMIT` | Number of 'and' allowed per clause. Default: 5 |
| `--select_limit SELECT_LIMIT` | Number of synonyms in select tuple allowed. Must be >= 1. Default: 5 |
| `--timing TIMING` | Timing for each query to run. Default: 5000 |
| `--verbose` | Makes the generator verbose for debugging purposes. Default: False |
| `--allow_error` | Allows for clauses with errors to be generated. Default: False |
| `--comment COMMENT [COMMENT ...]` | Comment to be generated for test cases together with query id. Default: NIL |
| `--preset PRESET [PRESET ...]` | Use a preset to generate queries. First input: 0 default (normal execution), 1 BOOLEAN select, 2 tuple select, 3 single select. Second input: 0 default (test iter2 queries), 1-12 suchthat-cl (1 Follows, 2 FollowsT, 3 Parent, 4 ParentT, 5 Calls, 6 CallsT, 7 UsesP/S, 8 ModifiesP/S, 9 Next, 10 NextT, 11 Affects, 12 AffectsT), 13-15 pattern-cl (13 syn-assign, 14 syn-if, 15 syn-while), 16 with-cl (all variants). Default: [0, 0]. Example usage: `--preset 3 11` to generate single select queries with Affects. |
