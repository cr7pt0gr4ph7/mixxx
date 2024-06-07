#pragma once

#include <QDebug>
#include <QHash>
#include <QString>

#include "track/customfield.h"
#include "util/assert.h"

class CustomFieldsTextParser;

class CustomFieldValues {
    friend class CustomFieldsTextParser;
    friend QDebug operator<<(QDebug dbg, CustomFieldValues arg);

  public:
    QString getField(const CustomFieldName& fieldName) const {
        VERIFY_OR_DEBUG_ASSERT(fieldName.isValid()) {
            return QString();
        }
        return m_namesToValues.value(fieldName.normalizedName());
    }

    void setField(const CustomFieldName& fieldName, const QString& value) {
        VERIFY_OR_DEBUG_ASSERT(fieldName.isValid()) {
            return;
        }
        m_namesToValues[fieldName.normalizedName()] = value;
    }

  private:
    QHash<QString, QString> m_namesToValues;
};

QDebug operator<<(QDebug dbg, CustomFieldValues arg);
