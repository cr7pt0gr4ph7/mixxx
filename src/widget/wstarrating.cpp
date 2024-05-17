#include "widget/wstarrating.h"

#include <QMouseEvent>
#include <QStyleOption>
#include <QStylePainter>

#include "moc_wstarrating.cpp"

class QEvent;
class QWidgets;

WStarRating::WStarRating(QWidget* pParent)
        : WWidget(pParent),
          m_starCount(0),
          m_visualStarRating(m_starCount) {
}

void WStarRating::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);
}

QSize WStarRating::sizeHint() const {
    return m_visualStarRating.sizeHint();
}

void WStarRating::slotSetRating(int starCount) {
    if (starCount == m_starCount || !m_visualStarRating.verifyStarCount(starCount)) {
        return;
    }
    m_starCount = starCount;
    updateVisualRating(starCount);
    emit ratingChangeRequest(starCount);
}

void WStarRating::paintEvent(QPaintEvent * /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter painter(this);

    // Center rating horizontally and vertically
    QSize ratingHint = m_visualStarRating.sizeHint();
    QRect contentRect(
            (size().width() - ratingHint.width()) / 2,
            (size().height() - ratingHint.height()) / 2,
            ratingHint.width(),
            ratingHint.height());

    painter.drawPrimitive(QStyle::PE_Widget, option);

    m_visualStarRating.paint(&painter, contentRect);
}

void WStarRating::keyPressEvent(QKeyEvent* event) {
    // Change rating when certain keys are pressed
    QKeyEvent* ke = static_cast<QKeyEvent*>(event);
    int newRating = m_visualStarRating.starCount();
    switch (ke->key()) {
    case Qt::Key_0: {
        newRating = 0;
        break;
    }
    case Qt::Key_1: {
        newRating = 1;
        break;
    }
    case Qt::Key_2: {
        newRating = 2;
        break;
    }
    case Qt::Key_3: {
        newRating = 3;
        break;
    }
    case Qt::Key_4: {
        newRating = 4;
        break;
    }
    case Qt::Key_5: {
        newRating = 5;
        break;
    }
    case Qt::Key_6: {
        newRating = 6;
        break;
    }
    case Qt::Key_7: {
        newRating = 7;
        break;
    }
    case Qt::Key_8: {
        newRating = 8;
        break;
    }
    case Qt::Key_9: {
        newRating = 9;
        break;
    }
    case Qt::Key_Right:
    case Qt::Key_Plus: {
        newRating++;
        break;
    }
    case Qt::Key_Left:
    case Qt::Key_Minus: {
        newRating--;
        break;
    }
    default: {
        event->ignore();
        return;
    }
    }
    newRating = math_clamp(newRating, StarRating::kMinStarCount, m_visualStarRating.maxStarCount());
    updateVisualRating(newRating);
    m_starCount = newRating;
}

void WStarRating::mouseMoveEvent(QMouseEvent *event) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const int pos = event->position().toPoint().x();
#else
    const int pos = event->x();
#endif
    int star = m_visualStarRating.starAtPosition(pos, rect());

    if (star == StarRating::kInvalidStarCount) {
        resetVisualRating();
    } else {
        updateVisualRating(star);
    }
}

void WStarRating::leaveEvent(QEvent* /*unused*/) {
    resetVisualRating();
}

void WStarRating::updateVisualRating(int starCount) {
    if (starCount == m_visualStarRating.starCount()) {
        return;
    }
    m_visualStarRating.setStarCount(starCount);
    update();
}

void WStarRating::mouseReleaseEvent(QMouseEvent* /*unused*/) {
    int starCount = m_visualStarRating.starCount();
    emit ratingChangeRequest(starCount);
}

void WStarRating::fillDebugTooltip(QStringList* debug) {
    WWidget::fillDebugTooltip(debug);

    QString currentRating;
    currentRating.setNum(m_starCount);
    QString maximumRating = QString::number(m_visualStarRating.maxStarCount());

    *debug << QString("Rating: %1/%2").arg(currentRating, maximumRating);
}
