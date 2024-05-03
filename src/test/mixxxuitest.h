#pragma once

#include "test/mixxxtest.h"

class MixxxUITest : public MixxxTest {
  public:
    MixxxUITest();
    ~MixxxUITest() override;

    class EventScope;

    void findAndFocusWidget(const QString& widgetName);
    void enterText(const QString& textToEnter);
    void enterKeys(const QString& keysToPress);
    void enterKeys(const QStringList& keysToPress);
    void enterKeys(const QKeySequence& keysToPress);

    template<typename... Args>
    void enterKeys(const QString& firstKey, const QString& secondKey, Args... args) {
        enterKeys((QStringList() << firstKey << secondKey << ... << args));
    };

  private:
    QObject* m_rootObject;
    bool m_deferProcessEvents;
};


class MixxxUITest::EventScope {
  public:
    EventScope(MixxxUITest* pParent)
    : m_pParent(pParent) {

    };

    virtual ~EventScope() {
      if (!m_pParent->m_deferProcessEvents) {
        m_eventLoop.processEvents();
      }
    }

    class Defer;

  protected:
    QEventLoop m_eventLoop;
    MixxxUITest* m_pParent;
};

class MixxxUITest::EventScope::Defer : public EventScope {
  public:
    Defer(MixxxUITest* pParent) : EventScope(pParent), m_outerDeferProcessEvents(pParent->m_deferProcessEvents){};
    ~Defer() override {
      m_pParent->m_deferProcessEvents = m_outerDeferProcessEvents;
    }

  private:
    bool m_outerDeferProcessEvents;
};
