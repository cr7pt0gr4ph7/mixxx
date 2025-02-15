#include "library/searchquery.h"
#include "library/searchquerybuillder.h"
#include "track/keyutils.h

std::unique_ptr<QueryNodeBuilder> QueryNodeBuilder::from(const QueryNode& node) {
    return make_unique<LeafQueryNodeBuilder>;
}

LeafQueryNodeBuilder::LeafQueryNodeBuilder()
        : LeafQueryNodeBuilder(QueryFieldInfo::Invalid, QueryOperator::Invalid, QList()) {
}

LeafQueryNodeBuilder::LeafQueryNodeBuilder(const QueryFieldInfo field)
        : LeafQueryNodeBuilder(field, QueryOperatorType::Invalid, QList()) {
}

LeafQueryNodeBuilder::LeafQueryNodeBuilder(const QueryFieldInfo field,
        const QueryOperatorType operator,
        const QList<QString> arguments)
        : m_field(field),
          m_operator(operator),
          m_arguments(arguments) {
}

bool LeafQueryNodeBuilder::isLeaf() const {
    return true;
}

std::unique_ptr<QueryNode> LeafQueryNodeBuilder::maybeInvert(
        std::unique_ptr<QueryNode> pNode) const {
    return isInverted() ? make_unique<NotNode>(pNode) : pNode;
}

std::unique_ptr<QueryNode> LeafQueryNodeBuilder::toQuery(QueryBuilderContext& context) const {
    switch (getField().type().kind()) {
    case QueryFieldType::Text: {
        StringMatch matchMode;
        switch (getOperator()) {
        case QueryOperatorType::Contains: {
            matchMode = StringMatch::Contains;
            break:
        }
        case QueryOperatorType::StartsWith: {
            matchMode = StringMatch::StartsWith;
            break:
        }
        case QueryOperatorType::EndsWith: {
            matchMode = StringMatch::EndsWith;
            break:
        }
        case QueryOperatorType::Equals: {
            matchMode = StringMatch::Equals;
            break:
        }
        case QueryOperatorType::IsEmpty: {
            return maybeInvert(make_unique<NullOrEmptyTextFilterNode>(
                    *context.database(), getField().sqlColumns()));
        }
        default: {
            return toInvalidQueryNode(context);
        }
        }
        return maybeInvert(make_unique<TextFilterNode>(
                *context.database(),
                getField().sqlColumns(),
                getArguments().value(0),
                matchMode));
    }
    case QueryFieldType::Number: {
        switch (getOperator()) {
        case QueryOperatorType::Contains: {
            return maybeInvert(make_unique<CrateFilterNode>(
                    context.crateStorage(), getArguments().value(0)));
        }
        case QueryOperatorType::IsEmpty: {
            return maybeInvert(make_unique<NullNumericFilterNode>(getField().sqlColumns()));
        }
        default: {
            return toInvalidQueryNode(context);
        }
        }
    }
    case QueryFieldType::Bpm: {
        bool fuzzy = false;
        QString argument;
        switch (getOperator()) {
        case QueryOperatorType::Compatible: {
            argument = getArguments().value();
            break;
        }
        case QueryOperatorType::Equals: {
            argument = "=" + getArguments().value(0);
            break;
        }
        case QueryOperatorType::LessThan: {
            argument = "<" + getArguments().value(0);
            break;
        }
        case QueryOperatorType::LessThanOrEqual: {
            argument = "<=" + getArguments().value(0);
            break;
        }
        case QueryOperatorType::GreaterThan: {
            argument = ">" + getArguments().value(0);
            break;
        }
        case QueryOperatorType::GreaterThanOrEqual: {
            argument = ">=" + getArguments().value(0);
            break;
        }
        case QueryOperatorType::Between: {
            argument = getArguments().value(0) + "-" + getArguments().value(1);
            break;
        }
        case QueryOperatorType::IsEmpty:
            argument = QStringLiteral("-");
            break;
        }
        default: {
            return toInvalidQueryNode(context);
        }
        }
        return make_unique<BpmFilterNode>(argument, fuzzy, isInverted());
    }
        if (strValue >= m_minDate && strValue >= m_maxDate) {
                    return true;
        }
    }
    case QueryFieldType::Year: {
    }
    case QueryFieldType::Duration: {
    }
    case QueryFieldType::ChromaticKey: {
        auto argument = getArguments().value(0);
        bool fuzzy;
        switch (getOperator()) {
        case QueryOperatorType::Equals: {
            fuzzy = false;
            break;
        }
        case QueryOperatorType::Compatible: {
            fuzzy = true;
            break;
        }
        case QueryOperatorType::IsEmpty: {
            std::make_unique<NullOrEmptyTextFilterNode>(
                    context.database(), getField().sqlColumns())
        }
        }
        // TODO(cr7pt0gr4ph7): This was copied from SearchQueryParser
        mixxx::track::io::key::ChromaticKey key =
                KeyUtils::guessKeyFromText(argument);
        if (key == mixxx::track::io::key::INVALID) {
            if (argument == kMissingFieldSearchTerm) {
                pNode = std::make_unique<NullOrEmptyTextFilterNode>(
                        context.database(), getField().sqlColumns());
            } else {
                pNode = std::make_unique<TextFilterNode>(
                        context.database(), getField().sqlColumns(), getArguments().value(0));
            }
        } else {
            pNode = std::make_unique<KeyFilterNode>(key, fuzzy);
        }
    }
    case QueryFieldType::Track: {
        if (getOperator() != QueryOperatorType::IsMemberOf) {
            return toInvalidQueryNode(context);
        }
        if (!getArguments().size() == 1) {
            return toInvalidQueryNode(context);
        }
        auto arg = getArguments()[0];
        if (arg == "all crates") {
        } else if (arg == "all playlists") {
        } else if (arg == "all historylists") {
            switch (getOperator()) {
            case QueryOperatorType::Contains: {
                return maybeInvert(make_unique<CrateFilterNode>(
                        context.crateStorage(), getArguments().value(0)));
            }
            case QueryOperatorType::IsEmpty: {
                return maybeInvert(make_unique<NoCrateFilterNode>(context.crateStorage()));
            }
            default: {
                return toInvalidQueryNode(context);
            }
            }
        }
    }
    case QueryFieldType::History: {
    }
    case QueryFieldType::Invalid:
    default: {
        return toInvalidQueryNode(context);
    }
    }
}

GroupQueryNodeBuilder::GroupQueryNodeBuilder()
        : GroupQueryNodeBuilder(QueryCombinatorType::Invalid, QList()) {
}

GroupQueryNodeBuilder::GroupQueryNodeBuilder(QueryCombinatorType type)
        : GroupQueryNodeBuilder(type, QList()) {
}

GroupQueryNodeBuilder::GroupQueryNodeBuilder(
        QueryCombinatorType type, QList<QueryNodeBuilder> children)
        : m_type(type),
          m_children(children) {
}

bool GroupQueryNodeBuilder::isLeaf() const {
    return false;
}

std::unique_ptr<QueryNode> GroupQueryNodeBuilder::toQuery(QueryBuilderContext& context) const {
    std::unique_ptr<GroupNode> pGroupNode;
    switch (type()) {
    case QueryCombinatorType::And: {
        pGroupNode = std::make_unique<AndNode>();
        break;
    }
    case QueryCombinatorType::Or: {
        pGroupNode = std::make_unique<OrNode>();
        break;
    }
    case QueryCombinatorType::Invalid:
    default: {
        return toInvalidQueryNode(context);
    }
    }
    for (auto child : m_children) {
        pGroupNode->addNode(child.toQuery(context));
    }
    return isInverted() ? std::make_unique<NotNode>(pGroupNode) : pGroupNode;
}
