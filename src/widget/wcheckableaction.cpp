#include "widget/wcheckableaction.h"

#include <QEvent>
#include <QCoreApplication>

#include "moc_wcheckableaction.cpp"
#include "util/assert.h"

WCheckableAction::WCheckableAction(QObject* parent)
        : WCheckableAction(QIcon(), QStringLiteral(""), parent) {
}

WCheckableAction::WCheckableAction(const QString& text, QObject* parent)
        : WCheckableAction(QIcon(), text, parent) {
}

WCheckableAction::WCheckableAction(const QIcon& icon, const QString& text, QObject* parent)
        : QAction(icon, text, parent),
          m_bTriState(false),
          m_bNoChange(false),
          m_bInSetCheckState(false),
          m_ePublishedState(Qt::Unchecked) {
    connect(this, &QAction::toggled, this, &WCheckableAction::onToggled, Qt::DirectConnection);
}

Qt::CheckState WCheckableAction::checkState() const {
    if (m_bTriState && m_bNoChange)
        return Qt::PartiallyChecked;

    return isChecked() ? Qt::Checked : Qt::Unchecked;
}

void WCheckableAction::setCheckState(Qt::CheckState state) {
    // This replicates parts of the logic from QAction to make sure
    // that change events are sent as expected, even though we don't
    // have access to the private implementation details of QAction.
    //
    // This means that we let QAction send its normal change events,
    // and only send change events ourselves in the cases where it doesn't.
    if (!isCheckable()) {
        // Do nothing if the action is not checkable
        return;
    }

    const bool bWasChecked = isChecked();
    const Qt::CheckState eWasState = checkState();
    const bool bChecked = state == Qt::Checked;

    if (state == Qt::PartiallyChecked) {
        m_bTriState = true;
        m_bNoChange = true;
    } else {
        m_bNoChange = false;
    }

    if (bChecked != bWasChecked) {
        // The base class will emit the following events/signals:
        // - QAction::toggled(checked) => Intercepted to emit checkStateChanged(...)
        // - QActionEvent(QEvent::ActionChanged)
        // - QAction::changed
        // - QAction::triggered(checked)
        DEBUG_ASSERT(!m_bInSetCheckState);
        m_bInSetCheckState = true;
        setChecked(bChecked);
        VERIFY_OR_DEBUG_ASSERT(!m_bInSetCheckState) {
            // The flag should have been reset by onToggled
            m_bInSetCheckState = false;
        }
    } else if (state != eWasState) {
        // This is a transition from Qt::Checked to Qt::PartiallyChecked,
        // or vice versa. isChecked() doesn't change, but checkState() does.
        //
        // Manually sent the change events that would have been sent by QAction::setChecked.
        m_ePublishedState = state;
        emit checkStateChanged(state);
        // QActionEvent e(QEvent::ActionChanged, this);
        // QCoreApplication::sendEvent(this, &e);
        emit changed();
        emit triggered(bChecked);
    } else {
        // Nothing has changed, so no change events will be sent.
    }
}

void WCheckableAction::onToggled(bool checked) {
    if (m_bInSetCheckState && m_bTriState && m_bNoChange && checked) {
        // This change originated from a call to setCheckState(Qt::PartiallyChecked),
        // and should therefore enter the "partially-checked" state.
        m_bNoChange = true;
    } else {
        // Exit the "partially-checked" state, transitioning
        // to either "checked" or "unchecked"
        m_bNoChange = false;
    }

    // Reset the flag now to avoid issues with reentrant calls of setCheckState(...).
    m_bInSetCheckState = false;

    Qt::CheckState state = checkState();
    if (state != m_ePublishedState) {
        m_ePublishedState = state;
        emit checkStateChanged(state);
    }
}
