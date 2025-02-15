#pragma once

#include <QList>
#include <memory>

#include "library/searchquerymeta.h"

class CrateStorage;
class QSqlDatabase;
class QueryNode;

/// Encapsulates the context required by QueryNodeBuilder
/// for creating valid QueryNode instances.
class QueryBuilderContext {
  public:
    QueryBuilderContext(QSqlDatabase* database, CrateStorage* crateStorage)
            : m_hasError(false),
              m_database(database),
              m_crateStorage(crateStorage) {
    }

    bool hasErrors() const {
        return m_hasError;
    }

    void addError() {
        m_hasError = true;
    }

    QSqlDatabase* database() const {
        return m_database;
    }

    CrateStorage* crateStorage() const {
        return m_crateStorage;
    }

  private:
    bool m_hasError;
    QSqlDatabase* m_database;
    CrateStorage* m_crateStorage;
};

class QueryNodeBuilder {
  public:
    QueryNodeBuilder()
            : m_inverted(false) {
    }
    virtual bool isLeaf() const = 0;

    bool isGroup() const {
        return !isLeaf();
    }

    bool isInverted() const {
        return m_inverted;
    }
    void setInverted(bool inverted) {
        m_inverted = inverted;
    }

    static std::unique_ptr<QueryNodeBuilder> from(const QueryNode& node);
    virtual std::unique_ptr<QueryNode> toQuery(QueryBuilderContext& context) const = 0;

  protected:
    std::unique_ptr<QueryNode> toInvalidQueryNode(QueryBuilderContext& context) const;

  private:
    bool m_inverted;
};

class LeafQueryNodeBuilder : public QueryNodeBuilder {
  public:
    LeafQueryNodeBuilder();
    LeafQueryNodeBuilder(const QueryFieldInfo field);
    LeafQueryNodeBuilder(const QueryFieldInfo field,
            const QueryOperatorType
            operator,
            const QList<QString> arguments);

    QueryFieldInfo getField() const {
        return m_field;
    }
    void setField(QueryFieldInfo field) {
        m_field = field;
    }

    QueryOperatorType getOperator() const {
        return m_operator;
    }
    void setOperator(QueryOperatorType op) {
        m_operator = op;
    }

    QList<QString> getArguments() const {
        return m_arguments;
    }
    void setArguments(const QList<QString>& arguments) {
        m_arguments = arguments;
    }

    bool isLeaf() const final;
    std::unique_ptr<QueryNode> toQuery(QueryBuilderContext& context) const override;

  private:
    std::unique_ptr<QueryNode> maybeInvert(std::unique_ptr<QueryNode> pNode);

    QueryFieldInfo m_field;
    QueryOperatorType m_operator;
    QList<QString> m_arguments;
};

class GroupQueryNodeBuilder : public QueryNodeBuilder {
  public:
    GroupQueryNodeBuilder();
    GroupQueryNodeBuilder(QueryCombinatorType type);
    GroupQueryNodeBuilder(QueryCombinatorType type, QList<QueryNodeBuilder> children);

    QueryCombinatorType getType() const {
        return m_type;
    }
    void setType(QueryCombinatorType type) {
        m_type = type;
    }

    QList<QueryNodeBuilder> children() {
        return m_children;
    }
    void setChildren(QList<QueryNodeBuilder> children) {
        m_children = children;
    }

    bool isLeaf() const final;
    std::unique_ptr<QueryNode> toQuery(QueryBuilderContext& context) const override;

  private:
    QueryCombinatorType m_type;
    QList<QueryNodeBuilder> m_children;
};
