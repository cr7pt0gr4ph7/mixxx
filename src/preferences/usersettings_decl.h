#pragma once

#include <QSharedPointer>
#include <QWeakPointer>

class ConfigValue;
template<class ValueType>
class ConfigObject;

typedef ConfigObject<ConfigValue> UserSettings;
typedef QSharedPointer<UserSettings> UserSettingsPointer;
typedef QWeakPointer<UserSettings> UserSettingsWeakPointer;
