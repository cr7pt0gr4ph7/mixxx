#pragma once

#include "library/starrating.h"
#include "widget/wwidget.h"

class QDomNode;
class SkinContext;

class WStarRating : public WWidget {
    Q_OBJECT
  public:
    WStarRating(QWidget* pParent);

    virtual void setup(const QDomNode& node, const SkinContext& context);
    QSize sizeHint() const override;

  public slots:
    void slotSetRating(int starCount);

  signals:
    void ratingChangeRequest(int starCount);

  protected:
    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent * /*unused*/) override;
    void fillDebugTooltip(QStringList* debug) override;

  private:
    bool m_focusedViaKeyboard;

    int m_starCount;

    StarRating m_visualStarRating;
    mutable QRect m_contentRect;

    void updateVisualRating(int starCount, bool forceRepaint = false);
    void resetVisualRating() {
        updateVisualRating(m_starCount);
    }
};
