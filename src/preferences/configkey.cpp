#include "preferences/configkey.h"

// static
ConfigKey ConfigKey::parseCommaSeparated(const QString& key) {
    int comma = key.indexOf(",");
    ConfigKey configKey(key.left(comma), key.mid(comma + 1));
    return configKey;
}
