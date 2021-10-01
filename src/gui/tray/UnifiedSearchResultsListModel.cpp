/*
 * Copyright (C) by Oleksandr Zolotov <alex@nextcloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "UnifiedSearchResultsListModel.h"

#include "account.h"
#include "accountstate.h"
#include "guiutility.h"
#include "networkjobs.h"

#include <algorithm>

#include "UserModel.h"

#include <QAbstractListModel>
#include <QDesktopServices>

namespace OCC {

UnifiedSearchResultsListModel::UnifiedSearchResultsListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant UnifiedSearchResultsListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= _results.size()) {
        return QVariant();
    }

    switch (role) {
    case ProviderNameRole: {
        return _results.at(index.row())._providerName;
    }
    case ProviderIdRole: {
        return _results.at(index.row())._providerId;
    }
    case ImagePlaceholderRole: {
        const auto resultInfo = _results.at(index.row());

        if (resultInfo._providerId.contains(QStringLiteral("message"))
            || resultInfo._providerId.contains(QStringLiteral("talk"))) {
            return QStringLiteral("qrc:///client/theme/black/wizard-talk.svg");
        } else if (resultInfo._providerId.contains(QStringLiteral("file"))) {
            return QStringLiteral("qrc:///client/theme/black/edit.svg");
        } else if (resultInfo._providerId.contains(QStringLiteral("calendar"))) {
            return QStringLiteral("qrc:///client/theme/black/calendar.svg");
        } else if (resultInfo._providerId.contains(QStringLiteral("mail"))) {
            return QStringLiteral("qrc:///client/theme/black/email.svg");
        } else if (resultInfo._providerId.contains(QStringLiteral("comment"))) {
            return QStringLiteral("qrc:///client/theme/account.svg");
        }

        return QString();
    }
    case IconsRole: {
        return _results.at(index.row())._icons;
    }
    case TitleRole: {
        return _results.at(index.row())._title;
    }
    case SublineRole: {
        return _results.at(index.row())._subline;
    }
    case ResourceUrlRole: {
        return _results.at(index.row())._resourceUrl;
    }
    case RoundedRole: {
        return _results.at(index.row())._isRounded;
    }
    case TypeRole: {
        return _results.at(index.row())._type;
    }
    case TypeAsStringRole: {
        return UnifiedSearchResult::typeAsString(_results.at(index.row())._type);
    }
    }

    return QVariant();
}

int UnifiedSearchResultsListModel::rowCount(const QModelIndex &) const
{
    return _results.size();
}

QHash<int, QByteArray> UnifiedSearchResultsListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ProviderNameRole] = "providerName";
    roles[ProviderIdRole] = "providerId";
    roles[IconsRole] = "icons";
    roles[ImagePlaceholderRole] = "imagePlaceholder";
    roles[TitleRole] = "resultTitle";
    roles[SublineRole] = "subline";
    roles[ResourceUrlRole] = "resourceUrlRole";
    roles[TypeRole] = "type";
    roles[TypeAsStringRole] = "typeAsString";
    roles[RoundedRole] = "isRounded";
    return roles;
}

QString UnifiedSearchResultsListModel::searchTerm() const
{
    return _searchTerm;
}

void UnifiedSearchResultsListModel::setSearchTerm(const QString &term)
{
    if (term == _searchTerm) {
        return;
    }

    _searchTerm = term;
    emit searchTermChanged();

    if (!_errorString.isEmpty()) {
        _errorString.clear();
        emit errorStringChanged();
    }

    disconnect(&_unifiedSearchTextEditingFinishedTimer, &QTimer::timeout, this,
        &UnifiedSearchResultsListModel::slotSearchTermEditingFinished);

    if (_unifiedSearchTextEditingFinishedTimer.isActive()) {
        _unifiedSearchTextEditingFinishedTimer.stop();
    }

    if (!_searchTerm.isEmpty()) {
        _unifiedSearchTextEditingFinishedTimer.setInterval(800);
        connect(&_unifiedSearchTextEditingFinishedTimer, &QTimer::timeout, this,
            &UnifiedSearchResultsListModel::slotSearchTermEditingFinished);
        _unifiedSearchTextEditingFinishedTimer.start();
    } else {
        if (!_searchJobConnections.isEmpty()) {
            for (const auto &connection : _searchJobConnections) {
                if (connection) {
                    QObject::disconnect(connection);
                }
            }

            _searchJobConnections.clear();
            emit isSearchInProgressChanged();
        }

        beginResetModel();
        _results.clear();
        endResetModel();
    }
}

bool UnifiedSearchResultsListModel::isSearchInProgress() const
{
    return !_searchJobConnections.isEmpty();
}

void UnifiedSearchResultsListModel::resultClicked(int resultIndex)
{
    if (resultIndex < 0 || resultIndex >= _results.size()) {
        return;
    }

    if (isSearchInProgress()) {
        return;
    }

    const auto modelIndex = index(resultIndex);

    const auto categoryId = data(modelIndex, ProviderIdRole).toString();

    const auto foundProviderIt = std::find_if(std::begin(_providers), std::end(_providers),
        [&categoryId](const UnifiedSearchProvider &current) { return current._id == categoryId; });

    const auto providerInfo = foundProviderIt != std::end(_providers) ? *foundProviderIt : UnifiedSearchProvider();

    if (!providerInfo._id.isEmpty() && providerInfo._id == categoryId) {
        const auto type = data(modelIndex, TypeRole).toUInt();

        if (type == UnifiedSearchResult::Type::FetchMoreTrigger) {
            if (providerInfo._isPaginated) {
                // Load more items
                const auto providerFound = _providers.find(providerInfo._name);
                if (providerFound != _providers.end()) {
                    _currentFetchMoreInProgressCategoryId = providerInfo._id;
                    emit currentFetchMoreInProgressCategoryIdChanged();
                    startSearchForProvider(*providerFound, providerInfo._cursor);
                }
            }
        } else {
            const auto resourceUrl = QUrl(data(modelIndex, ResourceUrlRole).toString());
            if (resourceUrl.isValid()) {
                if (categoryId.contains("file")) {
                    const auto currentUser = UserModel::instance()->currentUser();

                    if (!currentUser || !currentUser->account()) {
                        return;
                    }

                    const auto urlQuery = QUrlQuery(resourceUrl);
                    const auto dir =
                        urlQuery.queryItemValue(QStringLiteral("dir"), QUrl::ComponentFormattingOption::FullyDecoded);
                    const auto fileName = urlQuery.queryItemValue(
                        QStringLiteral("scrollto"), QUrl::ComponentFormattingOption::FullyDecoded);

                    if (!dir.isEmpty() && !fileName.isEmpty()) {
                        const QString relativePath = dir + QLatin1Char('/') + fileName;
                        if (!relativePath.isEmpty()) {
                            const auto localFiles = FolderMan::instance()->findFileInLocalFolders(
                                QFileInfo(relativePath).path(), currentUser->account());

                            if (!localFiles.isEmpty()) {
                                QDesktopServices::openUrl(localFiles.constFirst());
                                return;
                            }
                        }
                    }
                }
                Utility::openBrowser(resourceUrl);
            }
        }
    }
}

void UnifiedSearchResultsListModel::fetchMoreTriggerClicked(const QString &providerId)
{
    const auto foundProviderIt = std::find_if(std::begin(_providers), std::end(_providers),
        [&providerId](const UnifiedSearchProvider &current) { return current._id == providerId; });

    const auto providerInfo = foundProviderIt != std::end(_providers) ? *foundProviderIt : UnifiedSearchProvider();

    if (!providerInfo._id.isEmpty() && providerInfo._id == providerId) {
        if (providerInfo._isPaginated) {
            // Load more items
            const auto providerFound = _providers.find(providerInfo._name);
            if (providerFound != _providers.end()) {
                _currentFetchMoreInProgressCategoryId = providerInfo._id;
                emit currentFetchMoreInProgressCategoryIdChanged();
                startSearchForProvider(*providerFound, providerInfo._cursor);
            }
        }
    }
}

void UnifiedSearchResultsListModel::slotSearchTermEditingFinished()
{
    disconnect(&_unifiedSearchTextEditingFinishedTimer, &QTimer::timeout, this,
        &UnifiedSearchResultsListModel::slotSearchTermEditingFinished);

    if (_providers.isEmpty()) {
        const auto currentUser = UserModel::instance()->currentUser();

        if (!currentUser || !currentUser->account()) {
            return;
        }
        auto *job = new JsonApiJob(currentUser->account(), QLatin1String("ocs/v2.php/search/providers"));
        QObject::connect(job, &JsonApiJob::jsonReceived, [&, this](const QJsonDocument &json, int statusCode) {
            if (statusCode != 200) {
                _errorString +=
                    tr("Failed to fetch search providers for '%1'. Error: %2").arg(_searchTerm).arg(job->errorString())
                    + QLatin1Char('\n');
                emit errorStringChanged();
                return;
            }
            const auto providerList = json.object()
                                          .value(QStringLiteral("ocs"))
                                          .toObject()
                                          .value(QStringLiteral("data"))
                                          .toVariant()
                                          .toList();

            for (const auto &provider : providerList) {
                const auto providerMap = provider.toMap();
                const auto id = providerMap[QStringLiteral("id")].toString();
                const auto name = providerMap[QStringLiteral("name")].toString();
                UnifiedSearchProvider newProvider;
                if (!name.isEmpty() && id != QStringLiteral("talk-message-current")) {
                    newProvider._name = name;
                    newProvider._id = id;
                    newProvider._order = providerMap[QStringLiteral("order")].toInt();
                    _providers.insert(newProvider._name, newProvider);
                }
            }

            if (!_providers.empty()) {
                startSearch();
            }
        });
        job->start();
    } else {
        startSearch();
    }
}

void UnifiedSearchResultsListModel::slotSearchForProviderFinished(const QJsonDocument &json, int statusCode)
{
    bool appendResults = false;

    if (const auto job = qobject_cast<JsonApiJob *>(sender())) {
        appendResults = job->property("appendResults").toBool();
        const auto providerId = job->property("providerId").toString();

        if (!_searchJobConnections.isEmpty()) {
            _searchJobConnections.remove(providerId);

            if (_searchJobConnections.isEmpty()) {
                emit isSearchInProgressChanged();
            }
        }

        if (!_currentFetchMoreInProgressCategoryId.isEmpty()) {
            _currentFetchMoreInProgressCategoryId.clear();
            emit currentFetchMoreInProgressCategoryIdChanged();
        }

        if (statusCode != 200) {
            _errorString += tr("Search has failed for '%1'. Error: %2").arg(_searchTerm).arg(job->errorString())
                + QLatin1Char('\n');
            emit errorStringChanged();
            return;
        }
    }

    if (_searchTerm.isEmpty()) {
        return;
    }

    QList<UnifiedSearchResult> newEntries;

    const auto data = json.object().value(QStringLiteral("ocs")).toObject().value(QStringLiteral("data")).toObject();
    if (!data.isEmpty()) {
        const auto dataMap = data.toVariantMap();
        const auto name = data.value(QStringLiteral("name")).toString();
        auto &providerForResults = _providers[name];
        const auto isPaginated = data.value(QStringLiteral("isPaginated")).toBool();
        const auto cursor = data.value(QStringLiteral("cursor")).toInt();
        const auto entries = data.value(QStringLiteral("entries")).toVariant().toList();

        if (!providerForResults._id.isEmpty() && !entries.isEmpty()) {
            providerForResults._isPaginated = isPaginated;
            providerForResults._cursor = cursor;

            if (providerForResults._pageSize == -1) {
                providerForResults._pageSize = cursor;
            }

            if (providerForResults._pageSize != -1 && entries.size() < providerForResults._pageSize) {
                // for some providers we are still getting a non-null cursor and isPaginated true even thought there are
                // no more results to paginate
                providerForResults._isPaginated = false;
            }

            for (const auto &entry : entries) {
                UnifiedSearchResult result;
                result._providerId = providerForResults._id;
                result._order = providerForResults._order;
                result._providerName = providerForResults._name;
                QString icon = entry.toMap().value(QStringLiteral("icon")).toString();

                if (icon.contains(QLatin1Char('/')) || icon.contains(QLatin1Char('\\'))) {
                    const QUrl urlForIcon(icon);

                    if (!urlForIcon.isValid() || urlForIcon.scheme().isEmpty()) {
                        if (const auto currentUser = UserModel::instance()->currentUser()) {
                            auto serverUrl = QUrl(currentUser->server(false));
                            // some icons may contain parameters after (?)
                            const QStringList iconPathSplitted =
                                icon.contains(QLatin1Char('?')) ? icon.split(QLatin1Char('?')) : QStringList{icon};
                            serverUrl.setPath(iconPathSplitted[0]);
                            icon = serverUrl.toString();
                            if (iconPathSplitted.size() > 1) {
                                icon += QLatin1Char('?') + iconPathSplitted[1];
                            }
                        }
                    }
                } else {
                    const QUrl urlForIcon(icon);

                    if (!urlForIcon.isValid() || urlForIcon.scheme().isEmpty()) {
                        if (icon.contains(QStringLiteral("folder"))) {
                            icon = QStringLiteral(":/client/theme/black/folder.svg");
                        } else if (icon.contains(QStringLiteral("deck"))) {
                            icon = QStringLiteral(":/client/theme/black/deck.svg");
                        } else if (icon.contains(QStringLiteral("calendar"))) {
                            icon = QStringLiteral(":/client/theme/black/calendar.svg");
                        } else if (icon.contains(QStringLiteral("mail"))) {
                            icon = QStringLiteral(":/client/theme/black/email.svg");
                        }
                    }
                }

                result._isRounded = entry.toMap().value(QStringLiteral("rounded")).toBool();
                result._title = entry.toMap().value(QStringLiteral("title")).toString();
                result._subline = entry.toMap().value(QStringLiteral("subline")).toString();
                result._resourceUrl = entry.toMap().value(QStringLiteral("resourceUrl")).toString();
                QString thumbnailUrl = entry.toMap().value(QStringLiteral("thumbnailUrl")).toString();

                if (thumbnailUrl.contains(QLatin1Char('/')) || thumbnailUrl.contains(QLatin1Char('\\'))) {
                    const QUrl urlForIcon(thumbnailUrl);

                    if (!urlForIcon.isValid() || urlForIcon.scheme().isEmpty()) {
                        if (const auto currentUser = UserModel::instance()->currentUser()) {
                            auto serverUrl = QUrl(currentUser->server(false));
                            // some icons may contain parameters after (?)
                            const QStringList thumbnailUrlSplitted = thumbnailUrl.contains(QLatin1Char('?'))
                                ? thumbnailUrl.split(QLatin1Char('?'))
                                : QStringList{thumbnailUrl};
                            serverUrl.setPath(thumbnailUrlSplitted[0]);
                            thumbnailUrl = serverUrl.toString();
                            if (thumbnailUrlSplitted.size() > 1) {
                                thumbnailUrl += QLatin1Char('?') + thumbnailUrlSplitted[1];
                            }
                        }
                    }
                }

                const QStringList listImages = {thumbnailUrl, icon};
                result._icons = listImages.join(QLatin1Char(';'));

                newEntries.push_back(result);
            }

            if (appendResults) {
                appendResultsToProvider(providerForResults, newEntries);
            } else {
                combineResults(newEntries, providerForResults);
            }
        } else if (entries.isEmpty() && !providerForResults._id.isEmpty()) {
            // we may have received false pagination information from the server, such as, we expect more results
            // available via pagination, but, there are no more left, so, we need to stop paginating for this
            // provider

            providerForResults._isPaginated = false;

            if (appendResults) {
                appendResultsToProvider(providerForResults, {});
            }
        }
    }
}

void UnifiedSearchResultsListModel::startSearch()
{
    for (auto &connection : _searchJobConnections) {
        if (connection) {
            QObject::disconnect(connection);
        }
    }

    beginResetModel();
    _results.clear();
    endResetModel();

    for (const auto &provider : _providers) {
        startSearchForProvider(provider);
    }
}

void UnifiedSearchResultsListModel::startSearchForProvider(const UnifiedSearchProvider &provider, qint32 cursor)
{
    const auto currentUser = UserModel::instance()->currentUser();

    if (!currentUser || !currentUser->account()) {
        return;
    }

    auto *job = new JsonApiJob(
        currentUser->account(), QLatin1String("ocs/v2.php/search/providers/%1/search").arg(provider._id));
    QUrlQuery params;
    params.addQueryItem(QStringLiteral("term"), _searchTerm);
    if (cursor > 0) {
        params.addQueryItem(QStringLiteral("cursor"), QString::number(cursor));
        job->setProperty("appendResults", true);
    }
    job->setProperty("providerId", provider._id);
    job->addQueryParams(params);
    const auto wasSearchInProgress = isSearchInProgress();
    _searchJobConnections.insert(provider._id,
        QObject::connect(
            job, &JsonApiJob::jsonReceived, this, &UnifiedSearchResultsListModel::slotSearchForProviderFinished));
    if (isSearchInProgress() && !wasSearchInProgress) {
        emit isSearchInProgressChanged();
    }
    job->start();
}

void UnifiedSearchResultsListModel::combineResults(
    const QList<UnifiedSearchResult> &newEntries, const UnifiedSearchProvider &provider)
{
    ;
    auto newEntriesCopy = newEntries;

    const auto newEntriesOrder = newEntriesCopy.first()._order;
    const auto newEntriesName = newEntriesCopy.first()._providerName;

    UnifiedSearchResult categorySeparator;
    categorySeparator._providerId = newEntries.first()._providerId;
    categorySeparator._providerName = newEntriesName;
    categorySeparator._order = newEntriesOrder;
    categorySeparator._type = UnifiedSearchResult::Type::CategorySeparator;

    newEntriesCopy.push_front(categorySeparator);


    if (provider._cursor > 0 && provider._isPaginated) {
        UnifiedSearchResult fetchMoreTrigger;
        fetchMoreTrigger._providerId = provider._id;
        fetchMoreTrigger._providerName = provider._name;
        fetchMoreTrigger._order = newEntriesOrder;
        fetchMoreTrigger._type = UnifiedSearchResult::Type::FetchMoreTrigger;
        newEntriesCopy.push_back(fetchMoreTrigger);
    }


    if (_results.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, newEntriesCopy.size() - 1);
        _results = newEntriesCopy;
        endInsertRows();
        return;
    }

    auto itToInsertTo = std::find_if(std::begin(_results), std::end(_results),
        [newEntriesOrder, newEntriesName](const UnifiedSearchResult &current) {
            if (current._order > newEntriesOrder) {
                return true;
            } else {
                if (current._order == newEntriesOrder) {
                    return current._providerName > newEntriesName;
                }

                return false;
            }
        });

    if (itToInsertTo != std::end(_results)) {
        const auto first = itToInsertTo - std::begin(_results);
        const auto last = first + newEntriesCopy.size() - 1;

        beginInsertRows(QModelIndex(), first, last);
        std::copy(std::begin(newEntriesCopy), std::end(newEntriesCopy), std::inserter(_results, itToInsertTo));
        endInsertRows();
    } else {
        const auto first = _results.size();
        const auto last = first + newEntriesCopy.size() - 1;

        beginInsertRows(QModelIndex(), first, last);
        std::copy(std::begin(newEntriesCopy), std::end(newEntriesCopy), std::back_inserter(_results));
        endInsertRows();
    }
}

void UnifiedSearchResultsListModel::appendResultsToProvider(
    const UnifiedSearchProvider &provider, const QList<UnifiedSearchResult> &results)
{
    // Let's insert new results
    if (results.size() > 0) {
        const auto foundLastResultForProviderReverse =
            std::find_if(std::rbegin(_results), std::rend(_results), [&provider](const UnifiedSearchResult &result) {
                return result._providerId == provider._id && result._type == UnifiedSearchResult::Type::Default;
            });

        if (foundLastResultForProviderReverse != std::rend(_results)) {
            const auto numRowsToInsert = results.size();
            const auto foundLastResultForProvider = (foundLastResultForProviderReverse + 1).base();
            const auto first = foundLastResultForProvider + 1 - std::begin(_results);
            const auto last = first + numRowsToInsert - 1;
            beginInsertRows(QModelIndex(), first, last);
            std::copy(std::begin(results), std::end(results), std::inserter(_results, foundLastResultForProvider + 1));
            if (provider._isPaginated) {
                const auto foundLastResultForProviderReverse = std::find_if(
                    std::rbegin(_results), std::rend(_results), [&provider](const UnifiedSearchResult &result) {
                        return result._providerId == provider._id
                            && result._type == UnifiedSearchResult::Type::FetchMoreTrigger;
                    });
            }
            endInsertRows();
            if (!provider._isPaginated) {
                // Let's remove the FetchMoreTrigger item if present
                const auto providerId = provider._id;
                const auto foudFetchMoreTriggerForProviderReverse = std::find_if(
                    std::rbegin(_results), std::rend(_results), [providerId](const UnifiedSearchResult &result) {
                        return result._providerId == providerId
                            && result._type == UnifiedSearchResult::Type::FetchMoreTrigger;
                    });

                if (foudFetchMoreTriggerForProviderReverse != std::rend(_results)) {
                    const auto foundFetchMoreTriggerForProvider = (foudFetchMoreTriggerForProviderReverse + 1).base();

                    if (foundFetchMoreTriggerForProvider != std::end(_results)
                        && foundFetchMoreTriggerForProvider != std::begin(_results)) {
                        Q_ASSERT(foundFetchMoreTriggerForProvider->_type == UnifiedSearchResult::Type::FetchMoreTrigger
                            && foundFetchMoreTriggerForProvider->_providerId == providerId);
                        beginRemoveRows(QModelIndex(), foundFetchMoreTriggerForProvider - std::begin(_results),
                            foundFetchMoreTriggerForProvider - std::begin(_results));
                        _results.erase(foundFetchMoreTriggerForProvider);
                        endRemoveRows();
                    }
                }
            }
        }
    }
}

}
