#pragma once

#include <QFileSystemWatcher>
#include <QLocale>
#include <QMultiHash>
#include <QObject>

#include "control/controlobject.h"
#include "preferences/configobject.h"
#include "util/keyboardshortcutmanager.h"

class ControlObject;
class QEvent;
class QKeyEvent;
class WBaseWidget;

// This class provides handling of keyboard events.
class KeyboardEventFilter : public QObject, public KeyboardShortcutManager {
    Q_OBJECT
  public:
    KeyboardEventFilter(UserSettingsPointer pConfig,
            QLocale& locale,
            QObject* parent = nullptr,
            const char* name = nullptr);
    virtual ~KeyboardEventFilter();

    bool eventFilter(QObject* obj, QEvent* e);

    // Set the keyboard config object. KeyboardEventFilter does NOT take
    // ownership of pKbdConfigObject.
    std::shared_ptr<ConfigObject<ConfigValueKbd>> getKeyboardConfig();

    void setEnabled(bool enabled);
    bool isEnabled() {
        return m_enabled;
    }

    void registerShortcutWidget(WBaseWidget* pWidget);
    void updateWidgets();
    void clearWidgets();
    const QString buildShortcutString(const QString& shortcut, const QString& cmd) const;

    void registerActionForShortcut(
            QAction* pAction,
            const ConfigKey& command,
            const QString& defaultShortcut,
            bool useDefaultIfKeyboardDisabled = false) override;

    const QString registerMenuBarActionGetKeySeqString(
            QAction* pAction,
            const ConfigKey& command,
            const QString& defaultShortcut);
    void updateActions();

  public slots:
    void reloadKeyboardConfig();

  signals:
    // We're only the relay here: CoreServices -> this -> WBaseWidget
    void shortcutsEnabled(bool enabled);

  private:
    struct KeyDownInformation {
        KeyDownInformation(int keyId, int modifiers, ControlObject* pControl)
                : keyId(keyId),
                  modifiers(modifiers),
                  pControl(pControl) {
        }

        int keyId;
        int modifiers;
        ControlObject* pControl;
    };

    // Returns a valid QString with modifier keys from a QKeyEvent
    QKeySequence getKeySeq(QKeyEvent *e);

    // Run through list of active keys to see if the pressed key is already active
    // and is not a control that repeats when held.
    bool shouldSkipHeldKey(int keyId) {
        return std::any_of(
                m_qActiveKeyList.cbegin(),
                m_qActiveKeyList.cend(),
                [&](const KeyDownInformation& keyDownInfo) {
                    return keyDownInfo.keyId == keyId && !keyDownInfo.pControl->getKbdRepeatable();
                });
    }

    void createKeyboardConfig();

    // List containing keys which is currently pressed
    QList<KeyDownInformation> m_qActiveKeyList;

    UserSettingsPointer m_pConfig;
    QLocale m_locale;
    bool m_enabled;

    // Pointer to keyboard config object
    std::shared_ptr<ConfigObject<ConfigValueKbd>> m_pKbdConfig;

    QFileSystemWatcher m_fileWatcher;

    struct ShortcutInformation {
        ShortcutInformation(const ConfigKey& key,
                const QString& defaultShortcut,
                bool useDefaultIfKeyboardDisabled)
                : key(key),
                  defaultShortcut(defaultShortcut),
                  useDefaultIfKeyboardDisabled(useDefaultIfKeyboardDisabled) {
        }

        ConfigKey key;
        QString defaultShortcut;
        bool useDefaultIfKeyboardDisabled;
    };

    // Gets the shortcut that should be displayed in tooltips,
    // as determined by the current configuration
    QKeySequence getKeySequence(const ShortcutInformation& cmdInfo) const;

    // Actions in the menu bar and elsewhere
    // Value pair is the ConfigKey and the default QKeySequence (as QString).
    // Menu bar shortcuts are available even when keyboard shortcuts are disabled.
    QHash<QAction*, ShortcutInformation> m_actions;

    // Widgets that have mappable connections, registered by LegacySkinParser
    // during skin construction.
    QList<WBaseWidget*> m_widgets;

    // Multi-hash of key sequence to
    QMultiHash<ConfigValueKbd, ConfigKey> m_keySequenceToControlHash;
};
