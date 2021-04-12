#pragma once

#include <string>
#include <vector>

namespace Pql {
  enum class EntityType {
    // default
    UNDEFINED,

    // design-entities
    STMT,
    READ,
    PRINT,
    CALL,
    WHILE,
    IF,
    ASSIGN,
    VARIABLE,
    CONSTANT,
    PROG_LINE,
    PROCEDURE,

    // other-entities
    NUMBER,
    WILDCARD,
    NAME,
    SUB_EXPRESSION,
    EXPRESSION
  };

  enum class AttributeRefType {
    // default
    NONE,

    PROC_NAME,
    VAR_NAME,
    VALUE,
    STMT_NUMBER
  };

  enum class ClauseType {
    // default
    UNDEFINED,

    FOLLOWS,
    FOLLOWS_T,
    PARENT,
    PARENT_T,
    CALLS,
    CALLS_T,
    NEXT,
    NEXT_T,
    AFFECTS,
    AFFECTS_T,
    NEXT_BIP,
    NEXT_BIP_T,
    AFFECTS_BIP,
    AFFECTS_BIP_T,
    USES_S,
    USES_P,
    MODIFIES_S,
    MODIFIES_P,
    PATTERN_ASSIGN,
    PATTERN_IF,
    PATTERN_WHILE,
    WITH
  };

  /**
   * Represents an entity of a PQL query. An entity can be a synonym or any type of parameter in the PQL query.
  */
  class Entity {
  private:
    EntityType type;
    std::string value;
    AttributeRefType attributeRefType;

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
     * Constructor.
     *
     * @param type Entity type.
     * @param value Entity value.
     * @param attrivuteRef attribute reference type.
     */
    Entity(const EntityType& type, const std::string& value, const AttributeRefType& attributeRefType);

    /**
     * Get the entity type.
     *
     * @return Entity type.
     */
    EntityType getType() const;

    /**
     * Get the entity value.
     *
     * @return Entity value.
     */
    std::string getValue() const;

    /**
     * Get the entity attribute reference type.
     *
     * @return Entity attribute reference type.
     */
    AttributeRefType getAttributeRefType() const;

    /**
     * Checks if the entity is an attribute reference.
     *
     * @return True if the entity is an attribute reference. Otherwise, false.
     */
    bool isAttributeRef() const;

    /**
     * Checks if the entity is a number.
     *
     * @return True if the entity is a number. Otherwise, false.
     */
    bool isNumber() const;

    /**
     * Checks if the entity is a name.
     *
     * @return True if the entity is a name. Otherwise, false.
     */
    bool isName() const;

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

    /**
     * Checks if the entity is a 'stmt' synonym type.
     *
     * @return True if the entity is a stmt synonym type. Otherwise, false.
     */
    bool isStmtSynonym() const;

    /**
     * Checks if the entity is a 'read' synonym type.
     *
     * @return True if the entity is a read synonym type. Otherwise, false.
     */
    bool isReadSynonym() const;

    /**
     * Checks if the entity is a 'print' synonym type.
     *
     * @return True if the entity is a print synonym type. Otherwise, false.
     */
    bool isPrintSynonym() const;

    /**
     * Checks if the entity is a 'while' synonym type.
     *
     * @return True if the entity is a while synonym type. Otherwise, false.
     */
    bool isWhileSynonym() const;

    /**
     * Checks if the entity is a 'if' synonym type.
     *
     * @return True if the entity is a if synonym type. Otherwise, false.
     */
    bool isIfSynonym() const;

    /**
     * Checks if the entity is a 'prog_line' synonym type.
     *
     * @return True if the entity is a prog_line synonym type. Otherwise, false.
     */
    bool isProgLineSynonym() const;


    // Operator overloading
    friend bool operator==(const Entity& lhs, const Entity& rhs) {
      return lhs.type == rhs.type && lhs.value == rhs.value && lhs.attributeRefType == rhs.attributeRefType;
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
    std::string semanticErrorMessage;
    std::vector<Entity> targets;
    std::vector<Clause> clauses;

  public:
    /**
     * Checks if the query object is a BOOLEAN query. A boolean query have no targets.
     *
     * @return True if the PQL query is a BOOLEAN query.
     */
    bool isBoolean() const;

    /**
     * Gets the query targets.
     *
     * @return Vector of query targets.
     */
    std::vector<Entity> getTargets() const;

    /**
     * Gets the query clauses.
     *
     * @return Vector of clauses.
     */
    std::vector<Clause> getClauses() const;

    /**
     * Adds a select target to the query obect. Must be added in left-right order.
     *
     * @param target Target to be selected.
    */
    void addTarget(const Entity target);

    /**
     * Adds a clause to the query object. Clauses can be added in any order.
     *
     * @param clause Clause to be added.
    */
    void addClause(const Clause clause);


    /**
     * Checks if the PQL query has any semantic error.
     *
     * @return True if the query has any semantic error. Otherwise, return false.
    */
    bool hasSemanticError();

    /**
     * Adds a semantic error message.
     *
     * @param message Message to be added.
     */
    void setSemanticErrorMessage(const std::string& message);

    /**
     * Returns the semantic error message of the PQL query. Returns an empty string if
     * there is no semantic error.
     *
     * @return Semantic error message if any semantic error exist. Otherwise, return an empty string.
     */
    std::string getSemanticErrorMessage();

    // Operator overloading
    friend bool operator==(const Query& lhs, const Query& rhs) {
      return lhs.targets == rhs.targets && lhs.clauses == rhs.clauses;
    }
  };
}

namespace std {
  /**
   * Hash function for the Entity class.
   */
  template<> struct hash<Pql::Entity> {
    size_t operator()(Pql::Entity const& entity) const noexcept {
      size_t seed = hash<Pql::EntityType>{}(entity.getType());
      seed ^= hash<string>{}(entity.getValue())
        + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      seed ^= hash<Pql::AttributeRefType>{}(entity.getAttributeRefType())
        + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };

  /**
   * Hash function for the Clause class.
   */
  template<> struct hash<Pql::Clause> {
    size_t operator()(Pql::Clause const& clause) const noexcept {
      const vector<Pql::Entity>& params = clause.getParams();
      size_t seed = hash<Pql::ClauseType>{}(clause.getType());
      for (const Pql::Entity& param : params) {
        seed ^= hash<Pql::Entity>{}(param)
          + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
    }
  };
}