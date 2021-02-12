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
  bool isConsumeWhitespace;
public:
  /**
   * Standard c'tor.
   */
  Tokeniser() {
    isConsumeWhitespace = true;
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

  /**
   * Sets a flag to consume all encountered whitespace characters
   * while tokenizing. The default behaviour is to consume whitespace.
   * Returns a Tokeniser to allow for chaining e.g.
   * std::list<Token> tokens = tokeniser.consumingWhitespace().tokenise(my_stream);
   * 
   * @returns A Tokeniser that consumes encountered whitespace characters.
   */
  Tokeniser consumingWhitespace();

  /**
   * Sets a flag to tokenize all encountered whitespace characters
   * while tokenizing.
   * Returns a Tokeniser to allow for chaining e.g.
   * std::list<Token> tokens = tokeniser.notConsumingWhitespace().tokenise(my_stream);
   *
   * @returns A Tokeniser that does not consume encountered whitespace
   *     characters.
   */
  Tokeniser notConsumingWhitespace();
};