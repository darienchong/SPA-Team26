#pragma once

/**
 * Defines the types of Tokens that we can have.
 * 
 * @author Darien Chong
 */
enum class TokenType {
  // Delimiters that separate other tokens e.g. {
  Delimiter,

  // Either keywords e.g. procedure or names e.g. foo
  Identifier,

  // Numbers e.g. 2345
  Number,

  // Operators on numbers e.g. +
  Operator
};