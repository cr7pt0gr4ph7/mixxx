#pragma once

#include <QAction>
#include <QString>

#include "preferences/configobject.h"

/// Abstract interface implemented by KeyboardEventFilter.
class KeyboardShortcutManager {
  public:
    /// Bind the shortcut of the \a pAction to the shortcut specified
    /// by the configuration setting identified by \a command.
    virtual void registerActionForShortcut(
            QAction* pAction,
            const ConfigKey& command,
            const QString& defaultShortcut,
            bool useDefaultIfKeyboardDisabled = false) = 0;
};
