#pragma once

#include <QList>
#include <QString>
#include <QWidget>

class ControlWidgetPropertyConnection;
class ControlParameterWidgetConnection;

class WBaseWidget {
  public:
    explicit WBaseWidget(QWidget* pWidget);
    virtual ~WBaseWidget();

    virtual void Init();

    QWidget* toQWidget() {
        return m_pWidget;
    }

    void appendBaseTooltip(const QString& tooltip) {
        m_baseTooltip.append(tooltip);
        updateBaseTooltipOptShortcuts();
    }

    void prependBaseTooltip(const QString& tooltip) {
        m_baseTooltip.prepend(tooltip);
        updateBaseTooltipOptShortcuts();
    }

    void setBaseTooltip(const QString& tooltip) {
        m_baseTooltip = tooltip;
        updateBaseTooltipOptShortcuts();
    }

    void appendShortcutTooltip(const QString& tooltip) {
        m_shortcutTooltip.append(tooltip);
        updateBaseTooltipOptShortcuts();
    }

    QString baseTooltip() const {
        return m_baseTooltip;
    }

    QString shortcutHints() const {
        return m_shortcutTooltip;
    }

    QString baseTooltipOptShortcuts() const {
        return m_baseTooltipOptShortcuts;
    }

    /// Append/remove shortcuts hint when shortcuts are toggled
    void toggleKeyboardShortcutHints(bool enabled) {
        m_showKeyboardShortcuts = enabled;
        updateBaseTooltipOptShortcuts();
    }

    void updateBaseTooltipOptShortcuts() {
        QString tooltip;
        tooltip += m_baseTooltip;
        if (m_showKeyboardShortcuts && !m_shortcutTooltip.isEmpty()) {
            tooltip += "\n";
            tooltip += m_shortcutTooltip;
        }
        m_baseTooltipOptShortcuts = tooltip;
        m_pWidget->setToolTip(tooltip);
    }

    void addLeftConnection(ControlParameterWidgetConnection* pConnection);
    void addRightConnection(ControlParameterWidgetConnection* pConnection);
    void addConnection(ControlParameterWidgetConnection* pConnection);

    void addPropertyConnection(ControlWidgetPropertyConnection* pConnection);

    // Set a ControlWidgetConnection to be the display connection for the
    // widget. The connection should also be added via an addConnection method
    // or it will not be deleted or receive updates.
    void setDisplayConnection(ControlParameterWidgetConnection* pConnection);

    double getControlParameter() const;
    double getControlParameterLeft() const;
    double getControlParameterRight() const;
    double getControlParameterDisplay() const;

    inline const QList<ControlParameterWidgetConnection*>& connections() const {
        return m_connections;
    };
    inline const QList<ControlParameterWidgetConnection*>& leftConnections() const {
        return m_leftConnections;
    };

  protected:
    // Whenever a connected control is changed, onConnectedControlChanged is
    // called. This allows the widget implementor to respond to the change and
    // gives them both the parameter and its corresponding value.
    virtual void onConnectedControlChanged(double dParameter, double dValue) {
        Q_UNUSED(dParameter);
        Q_UNUSED(dValue);
    }

    void resetControlParameter();
    void setControlParameter(double v);
    void setControlParameterDown(double v);
    void setControlParameterUp(double v);
    void setControlParameterLeftDown(double v);
    void setControlParameterLeftUp(double v);
    void setControlParameterRightDown(double v);
    void setControlParameterRightUp(double v);

    // Tooltip handling. We support "debug tooltips" which are basically a way
    // to expose debug information about widgets via the tooltip. To enable
    // this, when widgets should call updateTooltip before they are about to
    // display a tooltip.
    void updateTooltip();
    virtual void fillDebugTooltip(QStringList* debug);

    QList<ControlParameterWidgetConnection*> m_connections;
    ControlParameterWidgetConnection* m_pDisplayConnection;
    QList<ControlParameterWidgetConnection*> m_leftConnections;
    QList<ControlParameterWidgetConnection*> m_rightConnections;

    QList<ControlWidgetPropertyConnection*> m_propertyConnections;

  private:
    QWidget* m_pWidget;

    QString m_baseTooltip;
    QString m_shortcutTooltip;
    QString m_baseTooltipOptShortcuts;

    bool m_showKeyboardShortcuts;

    friend class ControlParameterWidgetConnection;
};
