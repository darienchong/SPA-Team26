#include "Tokeniser.h"

#include <fstream>
#include <list>
#include <string>
#include <unordered_set>
#include <iostream>

namespace {
  const static Token WHITESPACE{ TokenType::DELIMITER, " " };
  /**
   * Checks if a given character is a delimiter or not.
   * 
   * @param c The character to check.
   * @returns `true` if the character is a delimiter, `false` otherwise.
   */
  bool isDelimiter(char c) {
    std::unordered_set<char> delims{ '{' , '}', '(', ')', ';', '_', '"', ',' };
    return delims.count(c) > 0;
  }

  /**
   * Constructs a Token from a delimiter.
   * 
   * @param stream The stream to read from.
   * @returns Token representing a delimiter. Throws std::invalid_argument
   *    if a non-delimiter character is encountered.
   */
  Token constructDelimiter(std::istream& stream) {
    TokenType type = TokenType::DELIMITER;
    std::string value;
    
    bool isNextCharDelimiter = isDelimiter(stream.peek());
    if (isNextCharDelimiter) {
      value.push_back(stream.get());
    } else {
      throw std::invalid_argument("[Tokeniser::constructDelimiter]: Expected one of {}();_\", but got " + stream.peek());
    }

    return { type, value };
  }

  /**
   * Constructs a Token from an identifier.
   * Identifiers cannot have a digit as the first character.
   * 
   * @param stream The stream to read from.
   * @returns Token representing an identifier.
   */
  Token constructIdentifier(std::istream& stream) {
    TokenType type = TokenType::IDENTIFIER;
    std::string value;

    bool isFirstChar = true;

    while (std::isalnum(stream.peek())) {
      bool isFirstCharDigit = isFirstChar && std::isdigit(stream.peek());

      // Identifiers cannot have digits as the first character.
      if (isFirstCharDigit) {
        throw std::invalid_argument("[Tokeniser::constructName]: Encountered a digit as the first character of a name.");
      } else {
        isFirstChar = false;
      }
      value.push_back(stream.get());
    }

    return { type, value };
  }

  /**
   * Constructs a Token from a number.
   * Numbers cannot have 0 as the first digit.
   * 
   * @param stream The stream to read from.
   * @returns Token representing a number.
   */
  Token constructNumber(std::istream& stream, bool isAllowLeadingZeroes) {
    TokenType type = TokenType::NUMBER;
    std::string value;

    bool isFirstChar = true;

    while (std::isdigit(stream.peek())) {
      // Since we peek ahead, if value equals "0" at any time it means that
      // we are at least on the second digit, hence an invalid construction.
      if (value == "0") {
        if (!isAllowLeadingZeroes) {
          throw std::invalid_argument("[Tokeniser::constructNumber]: Encountered 0 as the first digit of a number.");
        }
      }

      value.push_back(stream.get());
    }

    if (std::isalpha(stream.peek())) {
      throw std::invalid_argument("[Tokeniser::constructNumber]: Encountered an alphabetical letter while constructing a number.");
    }

    return { type, value };
  
  }

  /**
   * Checks if a character is a single-character operator.
   * Operators can be single character (e.g. +) or double character (e.g. <=).
   * 
   * @param c The character to check.
   * @returns `true` if the character is a single-character operator, `false` otherwise.
   */
  bool isSingleOperator(char c) {
    std::unordered_set<char> singleOperators{ '+', '-', '*', '/', '%' };

    return singleOperators.count(c) > 0;
  }

  /**
   * Check if a character is the first character of a one and/or two-character
   * operator. These operators are valid both as one-character operators on
   * their own, but are also the first character of a double-character operator
   * (e.g. < is valid, but so is <=).
   * Operators can be single character (e.g. +) or double character (e.g. <=).
   * 
   * @param c The character to check.
   * @returns `true` if the character is the first char of a double-character
   *    operator, `false` otherwise.
   */
  bool isCanHaveEquals(char c) {
    std::unordered_set<char> canHaveEquals{ '>', '<', '=', '!' };

    return canHaveEquals.count(c) > 0;
  }

  /**
   * Check if a character is the first character of two-character
   * operator that expects an = as the second character. 
   * These operators are valid only as double-character operator
   * (e.g. ! is not valid, but != is).
   * Operators can be single character (e.g. +) or double character (e.g. <=).
   *
   * @param c The character to check.
   * @returns `true` if the character is the first char of a double-character
   *    operator, `false` otherwise.
   */
  bool isExpectEquals(char c) {
    std::unordered_set<char> expectEquals;

    return expectEquals.count(c) > 0;
  }

  /**
   * Check if a character is the first character of two-character
   * operator that expects a & as the second character.
   * Operators can be single character (e.g. +) or double character (e.g. <=).
   *
   * @param c The character to check.
   * @returns `true` if the character is the first char of a double-character
   *    operator, `false` otherwise.
   */
  bool isExpectAmpersand(char c) {
    std::unordered_set<char> expectAmpersand{ '&' };

    return expectAmpersand.count(c) > 0;
  }

  /**
   * Check if a character is the first character of two-character
   * operator that expects a | as the second character.
   * Operators can be single character (e.g. +) or double character (e.g. <=).
   *
   * @param c The character to check.
   * @returns `true` if the character is the first char of a double-character
   *    operator, `false` otherwise.
   */
  bool isExpectShefferStroke(char c) {
    std::unordered_set<char> expectShefferStroke{ '|' };

    return expectShefferStroke.count(c) > 0;
  }

  /**
   * Check if a given character is the first character of an operator.
   *
   * @param c The character to check.
   * @returns `true` if the character is an operator or the first
   *    character of a two-character operator, `false` otherwise.
   */
  bool isOperator(char c) {
    return isSingleOperator(c) ||
      isCanHaveEquals(c) ||
      isExpectEquals(c) ||
      isExpectAmpersand(c) ||
      isExpectShefferStroke(c);
  }

  /**
   * Constructs a Token from an operator.
   * 
   * @param stream The stream to read from.
   * @returns Token representing an operator.
   */
  Token constructOperator(std::istream& stream) {
    TokenType type = TokenType::OPERATOR;
    std::string value;

    // We need to treat the operators depending on the first char
    // we see, since some operators consist of two characters e.g. <=
    // We thus need to also perform checks to ensure that the two character
    // operators are valid e.g. we allow <= but not !<
 
    if (!isOperator(stream.peek())) {
      throw std::invalid_argument("[Tokeniser::constructOperator]: Expected one of +-*/%>=<!&| but got " + stream.peek());
    }

    if (isSingleOperator(stream.peek())) {
      value.push_back(stream.get());
      return { type, value };
    }
    
    // We distinguish the cases where the operator is valid with or without
    // an = after it, and the cases where the operator is invalid without 
    // an = after it. Right now, there aren't any operators that are invalid
    // without an = after it, but for extensibility we'll leave this logic in.
    if (isCanHaveEquals(stream.peek())) {
      value.push_back(stream.get());
      bool isNextCharEquals = stream.peek() == '=';

      if (isNextCharEquals) {
        value.push_back(stream.get());
      }

      return { type, value };
    }

    if (isExpectEquals(stream.peek())) {
      value.push_back(stream.get());
      bool isNextCharEquals = stream.peek() == '=';

      if (isNextCharEquals) {
        value.push_back(stream.get());
      } else {
        throw std::invalid_argument("[Tokeniser::constructOperator]: Expected = but got " + stream.peek());
      }

      return { type, value };
    }

    if (isExpectAmpersand(stream.peek())) {
      value.push_back(stream.get());
      bool isNextCharAmpersand = stream.peek() == '&';

      if (isNextCharAmpersand) {
        value.push_back(stream.get());
      } else {
        throw std::invalid_argument("[Tokeniser::constructOperator]: Expected & but got " + stream.peek());
      }

      return { type, value };
    }

    if (isExpectShefferStroke(stream.peek())) {
      value.push_back(stream.get());
      bool isNextCharShefferStroke = stream.peek() == '|';

      if (isNextCharShefferStroke) {
        value.push_back(stream.get());
      } else {
        throw std::invalid_argument("[Tokeniser::constructOperator]: Expected | but got " + stream.peek());
      }

      return { type, value };
    }

    throw std::logic_error("[Tokeniser::constructOperator]: Failed to construct operator, got " + stream.peek());
  }

  /**
   * Help to discard whitespace characters (meaningless during parsing).
   * Advances the stream until it encounters a non-whitespace character.
   * 
   * @param stream The stream to read from.
   */
  void consumeWhitespace(std::istream& stream) {
    while (std::isspace(stream.peek())) {
      stream.get();
    }
  }
}

std::list<Token> Tokeniser::tokenise(std::istream &stream) {
  std::list<Token> tokens;

  while (stream.peek() != EOF) {
    bool isAlphabet = std::isalpha(stream.peek());
    bool isDigit = std::isdigit(stream.peek());
    bool isWhitespace = std::isspace(stream.peek());

    if (isAlphabet) {
      tokens.push_back(constructIdentifier(stream));
    } else if (isDelimiter(stream.peek())) {
      tokens.push_back(constructDelimiter(stream));
    } else if (isDigit) {
      tokens.push_back(constructNumber(stream, isAllowLeadingZeroes));
    } else if (isOperator(stream.peek())) {
      tokens.push_back(constructOperator(stream));
    } else if (isWhitespace) {
      if (!isConsumeWhitespace) {
        stream.get();
        tokens.push_back(WHITESPACE);
      } else {
        consumeWhitespace(stream);
      }
    } else {
      throw std::logic_error("[Tokeniser::tokenise]: Failed to recognise character " + stream.peek());
    }
  }

  return tokens;
}

Tokeniser Tokeniser::consumingWhitespace() {
  this->isConsumeWhitespace = true;
  return *this;
}

Tokeniser Tokeniser::notConsumingWhitespace() {
  this->isConsumeWhitespace = false;
  return *this;
}

Tokeniser Tokeniser::allowingLeadingZeroes() {
  this->isAllowLeadingZeroes = true;
  return *this;
}

Tokeniser Tokeniser::notAllowingLeadingZeroes() {
  this->isAllowLeadingZeroes = false;
  return *this;
}


