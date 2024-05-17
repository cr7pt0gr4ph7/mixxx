#pragma once

#include <QBrush>
#include <QPen>
#include <QPolygonF>
#include <QSize>

#include "track/trackrecord.h"

QT_FORWARD_DECLARE_CLASS(QPainter);
QT_FORWARD_DECLARE_CLASS(QRect);

/// The StarRating class represents a rating as a number of stars.
/// In addition to holding the data, it is also capable of painting the stars
/// on a QPaintDevice, which in this example is either a view or an editor.
/// The m_starCount member variable stores the current rating, and
/// m_maxStarCount stores the highest possible rating (typically 5).
class StarRating {
  public:
    enum EditMode { Editable, ReadOnly };
    class Palette {
      public:
        class PaletteGroup {
          public:
            /// Set the fill color for this PaletteGroup
            inline void setFill(const QColor& color) {
                fill = color;
            }

            /// Set the fill brush for this PaletteGroup
            inline void setFill(const QBrush& brush) {
                fill = brush;
            }

            /// Set the outline color for this PaletteGroup
            inline void setOutline(const QColor& color) {
                outline = color;
            }

            /// Set the outline brush for this PaletteGroup
            inline void setOutline(const QBrush& brush) {
                outline = brush;
            }

            /// The brush used for shape interiors, or Qt::NoBrush.
            QBrush fill;

            /// The brush used for shape outlines, or Qt::NoBrush.
            QBrush outline;
        };

        /// Set the fill color for all PaletteGroups
        void setFill(const QColor& color) {
            normal.setFill(color);
            highlight.setFill(color);
        }

        /// Set the fill brush for all PaletteGroups
        void setFill(const QBrush& brush) {
            normal.setFill(brush);
            highlight.setFill(brush);
        }

        /// Set the outline color for all PaletteGroups
        inline void setOutline(const QColor& color) {
            normal.setOutline(color);
            highlight.setOutline(color);
        }

        /// Set the outline brush for all PaletteGroups
        inline void setOutline(const QBrush& brush) {
            normal.setOutline(brush);
            highlight.setOutline(brush);
        }

        /// The fill and outline used for all shapes
        /// except the currently selected star.
        PaletteGroup normal;

        /// The fill and outline used for the currently selected star.
        PaletteGroup highlight;
    };

    static constexpr int kMinStarCount = 0;
    static constexpr int kInvalidStarCount = -1;

    explicit StarRating(
            int starCount = kMinStarCount,
            int maxStarCount = mixxx::TrackRecord::kMaxRating - mixxx::TrackRecord::kMinRating);

    void paint(QPainter* painter, const QRect& rect) const;
    void paint(QPainter* painter, const QRect& rect, const Palette& palette) const;
    QSize sizeHint() const;

    int starCount() const {
        return m_starCount;
    }
    int maxStarCount() const {
        return m_maxStarCount;
    }

    /// x is the x-position inside the parent rectangle rect
    int starAtPosition(int x, const QRect& rect) const;

    bool verifyStarCount(int starCount) {
        return starCount >= kMinStarCount && starCount <= m_maxStarCount;
    }

    void setStarCount(int starCount) {
        VERIFY_OR_DEBUG_ASSERT(verifyStarCount(starCount)) {
            return;
        }
        m_starCount = starCount;
    }

  private:
    void paintImpl(QPainter* painter,
            const QRect& rect,
            bool useBrushes,
            const Palette& palette) const;

    QPolygonF m_starPolygon;
    QPolygonF m_starOutlinePolygon;
    QPolygonF m_diamondPolygon;
    int m_starCount;
    int m_maxStarCount;
};

Q_DECLARE_METATYPE(StarRating)
