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
  bool isAllowLeadingZeroes;
  bool isConsumeWhitespace;
public:
  /**
   * Standard c'tor.
   */
  Tokeniser() {
    isConsumeWhitespace = true;
    isAllowLeadingZeroes = false;
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
   * Returns a Tokeniser that consumes all encountered whitespace characters
   * while tokenizing. The default behaviour is to consume whitespace.
   * 
   * @returns A Tokeniser that consumes encountered whitespace characters.
   */
  Tokeniser consumingWhitespace();

  /**
   * Returns a Tokeniser that tokenizes all encountered whitespace characters
   * while tokenizing.
   *
   * @returns A Tokeniser that does not consume encountered whitespace
   *     characters.
   */
  Tokeniser notConsumingWhitespace();

  /**
   * Returns a Tokeniser that allows for leading zero non-single digit numbers
   * to be tokenised.
   * 
   * @returns A Tokeniser that allows tokenising of leading zero
   *    non-single digit numbers.
   */
  Tokeniser allowingLeadingZeroes();

  /**
   * Returns a Tokeniser that disallows leading zero non-single digit numbers
   * to be tokenised.
   * 
   * @returns A Tokeniser that disallows tokenising of leading zero
   *     non-single digit numbers.
   */
  Tokeniser notAllowingLeadingZeroes();
};
