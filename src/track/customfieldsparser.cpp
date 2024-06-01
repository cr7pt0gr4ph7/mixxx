#include "track/customfieldsparser.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "util/assert.h"
#include "util/keyvalueiterable.h"

namespace {

const QString kCapturingGroupName = QStringLiteral("name");
const QString kCapturingGroupValue = QStringLiteral("value");

} // namespace

CustomFieldsTextParser::CustomFieldsTextParser()
        : m_regex(
                  "^(?<name>[a-zA-Z0-9_]+): ?(?<value>.+)$",
                  QRegularExpression::CaseInsensitiveOption |
                          QRegularExpression::MultilineOption) {
}

CustomFieldValues CustomFieldsTextParser::parse(const QString& text) const {
    CustomFieldValues result;
    auto allMatches = m_regex.globalMatch(text);
    while (allMatches.hasNext()) {
        auto match = allMatches.next();
        result.setField(
                CustomFieldName(match.captured(kCapturingGroupName)),
                match.captured(kCapturingGroupValue));
    }
    return result;
}

QString CustomFieldsTextParser::serialize(const CustomFieldValues& data) const {
    return QString();
}

QString CustomFieldsTextParser::serializeInto(
        const QString& oldText, const CustomFieldValues& data) const {
    return oldText;
}

CustomFieldValues CustomFieldsJsonParser::parse(const QString& text) const {
    CustomFieldValues result;
    QJsonParseError err;

    // An empty string means no custom field values.
    if (text.isEmpty()) {
        return result;
    }

    auto doc = QJsonDocument::fromJson(text.toUtf8(), &err);
    if (doc.isNull()) {
        qWarning() << "Failed to parse custom fields JSON:"
                   << err.error << err.errorString();
    }
    VERIFY_OR_DEBUG_ASSERT(doc.isObject()) {
        qWarning() << "Failed to parse custom fields JSON:"
                   << "JSON is valid but does not contain an object at the root";
        return result;
    }

    // TODO: Normalize by sorting alphabetically
    for (auto [key, value] : asKeyValueIterable(doc.object())) {
        result.setField(key, value.toString());
    }

    return result;
}

QString CustomFieldsJsonParser::serializeInto(
        const QString& oldText, const CustomFieldValues& data) const {
    Q_UNUSED(oldText);

    // Ignore oldText and just replace it with the new data
    return serialize(data);
}

QString CustomFieldsJsonParser::serialize(const CustomFieldValues& data) const {
    QJsonObject obj;
    for (auto [key, value] : asKeyValueIterable(data.m_namesToValues)) {
        obj[key] = QJsonValue(value);
    }
    QJsonDocument doc(obj);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}
