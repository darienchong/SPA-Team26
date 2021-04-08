#pragma once

#include <string>

/**
 * Defines the types of Tokens that we can have.
 *
 * @author Darien Chong
 */
enum class TokenType {
  // Delimiters that separate other tokens e.g. {
  DELIMITER,

  // Either keywords e.g. procedure or names e.g. foo
  IDENTIFIER,

  // Numbers e.g. 2345
  NUMBER,

  // Operators on numbers e.g. +
  OPERATOR,

  // White space (single) character e.g. <SPACE>
  WHITESPACE
};

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
  friend bool operator==(const Token& lhs, const Token& rhs) {
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
  friend bool operator!=(const Token& lhs, const Token& rhs) {
    return !(lhs == rhs);
  }
};

namespace std {
  /**
   * Hash function for the Token class.
   */
  template<> struct hash<Token> {
    std::size_t operator()(Token const& token) const noexcept {
      std::size_t h1 = std::hash<TokenType>{}(token.type);
      std::size_t h2 = std::hash<std::string>{}(token.value);
      return h1 ^ (h2 << 1);
    }
  };
}