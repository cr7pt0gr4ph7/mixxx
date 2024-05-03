#include "test/mixxxuitest.h"

#include <QApplication>
#include <QEventLoop>
#include <QKeyEvent>
#include <QKeySequence>
#include <QObject>
#include <QString>

namespace {
void internalPostEvent(QEvent* event) {
    QApplication::postEvent(QApplication::activeWindow(), event);
}
} // anonymous namespace

MixxxUITest::MixxxUITest() : m_rootObject(nullptr) {
}

MixxxUITest::~MixxxUITest() {
}

void MixxxUITest::findAndFocusWidget(const QString& widgetName) {
    EventScope eventScope(this);
    auto widget = m_rootObject->findChild<QWidget*>(widgetName,Qt::FindChildrenRecursively );
    if(!widget) {
        // TODO: Fail assertion
        return;
    }
    widget->setFocus();
}

void MixxxUITest::enterText(const QString& textToEnter) {
    EventScope::Defer eventScope(this);
    new QKeyEvent(QEvent::KeyPress, Qt::Key_H, Qt::ShiftModifier, "H");
    new QKeyEvent(QEvent::KeyPress, Qt::Key_E, Qt::NoModifier, "e");
    new QKeyEvent(QEvent::KeyPress, Qt::Key_L, Qt::NoModifier, "l");
    new QKeyEvent(QEvent::KeyPress, Qt::Key_L, Qt::NoModifier, "l");
    new QKeyEvent(QEvent::KeyPress, Qt::Key_O, Qt::NoModifier, "o");
}

void MixxxUITest::enterKeys(const QString& keysToPress) {
    enterKeys(QKeySequence(keysToPress));
}

void MixxxUITest::enterKeys(const QStringList& listOfKeysToPress) {
    EventScope::Defer eventScope(this);
    for (auto keysToPress : listOfKeysToPress) {
        enterKeys(keysToPress);
    }
}

void MixxxUITest::enterKeys(const QKeySequence& keysToPress) {
    EventScope eventScope(this);
    for(int i=0; i<keysToPress.count(); i++) {
        internalPostEvent(new QKeyEvent(
            QKeyEvent::KeyPress,
            keysToPress[i].key(),
            keysToPress[i].keyboardModifiers()
            // TODO: Generate text
        ));
        internalPostEvent(new QKeyEvent(
            QKeyEvent::KeyRelease,
            keysToPress[i].key(),
            keysToPress[i].keyboardModifiers()
            // TODO: Generate text
        ));
    }
}
