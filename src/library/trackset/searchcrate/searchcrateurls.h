#pragma once

#include "library/trackset/searchcrate/searchcrateid.h"
#include "util/urlhelper.h"

class SearchCrateURLs {
  private:
    static const QString kUrlTemplate;

  public:
    // Returns the URL representing the specified, valid crateId,
    // or an empty QUrl if crateId is invalid.
    static QUrl toUrl(SearchCrateId crateId) {
        return UrlHelper::urlFromTemplate(kUrlTemplate, crateId);
    }

    // Returns the SearchCrateId represented by the specified QUrl,
    // or an invalid SearchCrateId if the URL does not represent a crate.
    static SearchCrateId parseUrl(const QUrl& url) {
        return UrlHelper::idFromUrl<SearchCrateId>(kUrlTemplate, url);
    }

    // Parses the list of URLs, and returns the corresponding list of search crate ids.
    // Urls that do not represent search crate references are ignored.
    static QList<SearchCrateId> parseUrls(const QList<QUrl>& urls) {
        return UrlHelper::idsFromUrls<SearchCrateId>(kUrlTemplate, url);
    }
};

const QString SearchCrateURLs::kUrlTemplate = QStringLiteral("mixxx://library/searchcrates");
