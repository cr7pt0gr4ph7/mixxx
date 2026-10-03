#pragma once

#include <QPointer>

#include "widget/wmenu.h"

class Track;
class FindOnWebLast;

class WFindOnWebMenu : public WMenu {
    Q_OBJECT
  public:
    explicit WFindOnWebMenu(const QPointer<QMenu>& pParent, QPointer<FindOnWebLast> pFindOnWebLast);
    ~WFindOnWebMenu() override = default;

    void addActionToServiceMenu(
            const QString& actionId,
            const QString& actionText,
            const QUrl& serviceUrl);

    static bool hasEntriesForTrack(const Track& track);

  protected:
    QString composeSearchQuery(const QString& artist, const QString& trackAlbumOrTitle);
    void openInBrowser(const QUrl& url);

  private:
    const QPointer<FindOnWebLast> m_pFindOnWebLast;
};
