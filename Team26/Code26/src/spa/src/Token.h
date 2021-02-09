#pragma once

#include "TokenType.h"

#include <string>

/**
 * Standard struct representing a token during tokenising and parsing.
 * 
 * @author Darien Chong
 */
struct Token {
  // The type of the token.
  const TokenType type;

  // The value of the token.
  const std::string value;
  
  /**
   * Overloading the == operator for comparisons.
   * Returns `true` if the type and value of the tokens match, else `false`.
   * 
   * @param lhs One of the tokens to compare against.
   * @param rhs The other token to compare against.
   * @returns `true` if the two tokens have identical type and value,
   *    `false` otherwise.
   */
  friend bool operator==(const Token &lhs, const Token& rhs) {
    return lhs.type == rhs.type &&
      lhs.value == rhs.value;
  }

  /**
   * Overloading the != equality for ease of comparison.
   * 
   * @param lhs One of the tokens to compare against.
   * @param rhs The other token to compare against.
   * @returns `true` if the two tokens have either different type or value,
   *    `false` otherwise.
   */
  friend bool operator!=(const Token &lhs, const Token& rhs) {
    return !(lhs == rhs);
  }
};
