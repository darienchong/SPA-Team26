#pragma once

#include <vector>
#include <string>

namespace Pql {
  enum class EntityType {
    // design-entities
    STMT,
    READ,
    PRINT,
    WHILE,
    IF,
    ASSIGN,
    VARIABLE,
    CONSTANT,
    PROCEDURE,

    // other-entities
    STMT_NUMBER,
    WILDCARD,
    VARIABLE_NAME,
    SUB_EXPRESSION,
    EXPRESSION,

    // default
    UNDEFINED
  };

  enum class ClauseType {
    FOLLOWS,
    FOLLOWS_T,
    PARENT,
    PARENT_T,
    USES_S,
    MODIFIES_S,
    PATTERN_ASSIGN,

    // default
    UNDEFINED
  };

  /**
   * Represents an entity of a PQL query. An entity can be a synonym or any type of parameter in the PQL query.
  */
  class Entity {
  private:
    EntityType type;
    std::string value;

  public:
    /**
     * Default constructor. type is set to EntityType::UNDEFINED.
     */
    Entity();

    /**
     * Constructor.
     *
     * @param type Entity type.
     * @param value Entity value.
     */
    Entity(const EntityType& type, const std::string& value);

    /**
     * Get the entity type.
     *
     * @return Entity type.
     */
    EntityType getType() const;

    /**
     * Get the entity value
     *
     * @return Entity value.
     */
    std::string getValue() const;

    /**
     * Checks if the entity is a statement number.
     *
     * @return True if the entity is a statement number. Otherwise, false.
     */
    bool isStmtNumber() const;

    /**
     * Checks if the entity is a variable name.
     *
     * @return True if the entity is a variable name. Otherwise, false.
     */
    bool isVariableName() const;

    /**
     * Checks if the entity is a wildcard.
     *
     * @return True if the entity is a wildcard. Otherwise, false.
     */
    bool isWildcard() const;

    /**
     * Checks if the entity is a sub-expression.
     *
     * @return True if the entity is a sub-expression. Otherwise, false.
     */
    bool isSubExpression() const;

    /**
     * Checks if the entity is an expression.
     *
     * @return True if the entity is an expression. Otherwise, false.
     */
    bool isExpression() const;

    /**
     * Checks if the entity is a synonym type.
     *
     * @return True if the entity is a synonym type. Otherwise, false.
     */
    bool isSynonym() const;

    // Operator overloading
    friend bool operator==(const Entity& lhs, const Entity& rhs) {
      return lhs.type == rhs.type && lhs.value == rhs.value;
    }

    friend bool operator!=(const Entity& lhs, const Entity& rhs) {
      return !(lhs == rhs);
    }
  };

  /**
   * Represents a single clause in a PQL query.
  */
  class Clause {
  private:
    ClauseType type;
    std::vector<Entity> params;

  public:
    /**
     * Default constructor.
    */
    Clause();

    /**
     * Constructs a Clause object with the given type and parameters.
     *
     * @param type Clause type.
     * @param params Paramters of the clause.
    */
    Clause(const ClauseType& type, const std::vector<Entity>& params);

    /**
     * Get the clause type.
     *
     * @return Clause type.
     */
    ClauseType getType() const;

    /**
     * Get the clause params.
     *
     * @return Vector of clause params.
     */
    std::vector<Entity> getParams() const;

    /**
     * Sets the type of clause.
     *
     * @param type Clause type.
    */
    void setType(const ClauseType type);

    /**
     * Adds a parameter to the clause. Parameter must be added in the order of left to right.
     *
     * @param param Parameter to be added next.
    */
    void addParam(const Entity param);

    // Operator overloading
    friend bool operator==(const Clause& lhs, const Clause& rhs) {
      return lhs.type == rhs.type && lhs.params == rhs.params;
    }
  };

  /**
   * Represents a query object which contains the target synonym to be selected and the clauses.
  */
  class Query {
  private:
    Entity target;
    std::vector<Clause> clauses;

  public:
    /**
     * Get the query target.
     *
     * @return Query target.
     */
    Entity getTarget() const;

    /**
     * Get the query clauses.
     *
     * @return Vector of clauses.
     */
    std::vector<Clause> getClauses() const;

    /**
     * Sets the target to be selected.
     *
     * @param target Synonym to be selected.
    */
    void setTarget(const Entity target);

    /**
     * Add a clause to the query object. Clauses can be added in any order.
     *
     * @param clause Clause to be added.
    */
    void addClause(const Clause clause);

    // Operator overloading
    friend bool operator==(const Query& lhs, const Query& rhs) {
      return lhs.target == rhs.target && lhs.clauses == rhs.clauses;
    }
  };
}
