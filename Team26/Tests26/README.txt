This directory contains all the system test cases.

IMPORTANT NOTE: 
All test cases EXCEPT `general` have at least THREE queries for each type of query condition 
(e.g., three for Follows, three for Modifies, three for pattern, etc.) and TEN queries for various 
combinations of query conditions (e.g., two for Follows + pattern, three for Modifies + pattern, etc.). 

This is because the test case `general` is designed to be simple and check for ALL types of valid general 
queries, queries with syntax errors and queries with semantic errors.

The following shows a brief description of the purpose of each test case:

`general`
  - SIMPLE Program
    - program with irregular white spaces
  - Queries
    - Valid general queries
    - Syntax Errors
    - Semantic Errors

`no-nesting`
  - SIMPLE Program 
    - program with no nesting
    - use of keywords as variables
    - All types of naming variations
    - Complicated assignment expression
  - Queries
    - All possible kinds of valid and invalid queries (except for Parent/Parent* relRef)
  
`multiple-deep-nesting`
  - SIMPLE Program 
    - program with multiple container statments
    - deep nesting
  - Queries
    - All possible kinds of valid and invalid queries (especially for Parent/Parent* relRef)

`program1`, `program2`, `program3`
  - SIMPLE Programs
    - 3 Different arbitrary programs that one might write using SIMPLE
  - Queries
    - Simple and complex queries to test the correctness of the Query Processing Subsystem
