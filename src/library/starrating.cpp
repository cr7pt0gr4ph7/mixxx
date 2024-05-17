#include "library/starrating.h"

#include <QPainter>
#include <QRect>

#include "util/math.h"
#include "util/painterscope.h"

namespace {
/// The default size of a single stars bounding box in pixels.
constexpr int PaintingScaleFactor = 15;

void applyPalette(QPainter* painter, const StarRating::Palette::PaletteGroup& pg) {
    painter->setBrush(pg.fill);
    if (pg.outline.style() == Qt::NoBrush) {
        painter->setPen(Qt::NoPen);
    } else {
        painter->setPen(QPen(pg.outline, 1.0 / PaintingScaleFactor));
    }
}
} // namespace

StarRating::StarRating(
        int starCount,
        int maxStarCount)
        : m_starCount(starCount),
          m_maxStarCount(maxStarCount) {
    DEBUG_ASSERT(verifyStarCount(m_starCount));

    constexpr double OuterRadius = 0.5;
    constexpr double SqrtOf5 = 2.236067977499789696;
    constexpr double InnerRadius = OuterRadius * 0.5 * (3 - SqrtOf5);

    // 1st star cusp is at 0° of the unit circle (i.e. pointing to the right).
    // The star's center is shifted to (0.5, 0.5).
    for (int i = 0; i < 5; ++i) {
        // Add points for the outline of the star polygon.
        // All points lay on either the inner or the outer circle.
        m_starPolygon << QPointF(
                                 0.5 + OuterRadius * cos(0.4 * i * M_PI),
                                 0.5 + OuterRadius * sin(0.4 * i * M_PI))
                      << QPointF(
                                 0.5 + InnerRadius * cos((0.2 + 0.4 * i) * M_PI),
                                 0.5 + InnerRadius * sin((0.2 + 0.4 * i) * M_PI));

        m_starOutlinePolygon
                << QPointF(0.5 +
                                   (OuterRadius - 1 / PaintingScaleFactor) *
                                           cos(0.4 * i * M_PI),
                           0.5 +
                                   (OuterRadius - 1 / PaintingScaleFactor) *
                                           sin(0.4 * i * M_PI))
                << QPointF(0.5 +
                                   (InnerRadius - 1 / PaintingScaleFactor) *
                                           cos((0.2 + 0.4 * i) * M_PI),
                           0.5 +
                                   (InnerRadius - 1 / PaintingScaleFactor) *
                                           sin((0.2 + 0.4 * i) * M_PI));
    }

    // Create 4 points for a tiny diamond/rhombe (square turned by 45°)
    m_diamondPolygon << QPointF(0.4, 0.5) << QPointF(0.5, 0.4)
                     << QPointF(0.6, 0.5) << QPointF(0.5, 0.6);
}

QSize StarRating::sizeHint() const {
    return PaintingScaleFactor * QSize(m_maxStarCount, 1);
}

void StarRating::paint(QPainter* painter, const QRect& rect) const {
    paintImpl(painter, rect, false, Palette());
}

void StarRating::paint(QPainter* painter, const QRect& rect, const Palette& palette) const {
    paintImpl(painter, rect, true, palette);
}
void StarRating::paintImpl(QPainter* painter,
        const QRect& rect,
        bool usePalette,
        const Palette& palette) const {
    PainterScope painterScope(painter);
    // Assume the painter is configured with the right brush.
    painter->setRenderHint(QPainter::Antialiasing, true);
    // Don't draw outlines, only fill the polygons
    painter->setPen(Qt::NoPen);

    // Center vertically inside the table cell, and also center horizontally
    // if the cell is wider than the minimum stars width.
    int xOffset = std::max((rect.width() - sizeHint().width()) / 2, 0);
    int yOffset = (rect.height() - PaintingScaleFactor) / 2;
    painter->translate(rect.x() + xOffset, rect.y() + yOffset);
    painter->scale(PaintingScaleFactor, PaintingScaleFactor);

    // Determine number of stars that are possible to paint
    int n = rect.width() / PaintingScaleFactor;
    if (usePalette) {
        applyPalette(painter, palette.normal);
    }

    for (int i = 0; i < m_maxStarCount && i < n; ++i) {
        if (i == m_starCount - 1 && usePalette) {
            applyPalette(painter, palette.highlight);
            if (palette.highlight.outline.style() != Qt::NoBrush) {
                painter->drawPolygon(m_starOutlinePolygon, Qt::WindingFill);
            } else {
                painter->drawPolygon(m_starPolygon, Qt::WindingFill);
            }
            applyPalette(painter, palette.normal);
        } else if (i < m_starCount) {
            painter->drawPolygon(m_starPolygon, Qt::WindingFill);
        } else {
            painter->drawPolygon(m_diamondPolygon, Qt::WindingFill);
        }
        painter->translate(1.0, 0.0);
    }
}

int StarRating::starAtPosition(int x, const QRect& rect) const {
    // The star rating is drawn centered in the parent (WStarRating or
    // cell of StarDelegate, so we need to shift the x input as well.
    int starsWidth = sizeHint().width();
    int xOffset = std::max((rect.width() - starsWidth) / 2, 0);
    // Only shift if the parent is wider than the star rating
    x -= std::max(xOffset, 0);

    // Return invalid if the pointer left the star rectangle at either side.
    // If the the parent is wider than the star rating, add a half star margin
    // at the left to simplify setting 0.
    double leftVoid = xOffset > starsWidth * 0.05 ? starsWidth * -0.05 : 0;
    if (x < leftVoid || x >= starsWidth) {
        return StarRating::kInvalidStarCount;
    } else if (x < starsWidth * 0.05) {
        // If the pointer is very close to the left edge, set 0 stars.
        return 0;
    }

    int star = (x / (starsWidth / maxStarCount())) + 1;

    if (star <= 0 || star > maxStarCount()) {
        return 0;
    }
    return star;
}
