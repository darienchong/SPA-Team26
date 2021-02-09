#pragma once

#include "Token.h"

#include <string>
#include <list>
#include <fstream>

/**
 * Concrete API for the Tokeniser.
 * Handles conversion of a file (given as an input file stream)
 * into a list of Tokens for parsing.
 * 
 * @author Darien Chong
 */
class Tokeniser {
private:

public:
  /**
   * Standard c'tor.
   */
  Tokeniser() {
  }

  /**
   * Standard d'tor.
   */
  ~Tokeniser() {
  }

  /**
   * Takes an input file stream and tokenises it,
   * turning it into a list of Tokens for parsing.
   * 
   * Pre-conditions: The program given (represented by the input file stream)
   * must be a syntactically valid program.
   * 
   * Post-conditions: The list of Tokens will correspond to the valid program
   * given.
   * 
   * @param stream Pointer to the input file stream to use.
   * @returns List of Tokens for parsing.
   */
  std::list<Token> tokenise(std::istream& stream);
};