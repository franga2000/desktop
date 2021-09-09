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
#include "networkjobs.h"

namespace OCC {

UnifiedSearchResultsListModel::UnifiedSearchResultsListModel(AccountState *accountState, QObject *parent)
    : QAbstractListModel(parent)
    , _accountState(accountState)
{
}

UnifiedSearchResultsListModel::~UnifiedSearchResultsListModel()
{
    int a = 5;
    a = 6;
}

QVariant UnifiedSearchResultsListModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

int UnifiedSearchResultsListModel::rowCount(const QModelIndex &) const
{
    return 0;
}

void UnifiedSearchResultsListModel::setSearchTerm(const QString &term)
{
    if (term == _searchTerm) {
        return;
    }

    disconnect(&_unifiedSearchTextEditingFinishedTimer, &QTimer::timeout, this, &UnifiedSearchResultsListModel::slotSearchTermEditingFinished);

    if (_unifiedSearchTextEditingFinishedTimer.isActive()) {
        _unifiedSearchTextEditingFinishedTimer.stop();
    }

    _searchTerm = term;

    if (!searchTerm().isEmpty()) {
        _unifiedSearchTextEditingFinishedTimer.setInterval(400);
        connect(&_unifiedSearchTextEditingFinishedTimer, &QTimer::timeout, this, &UnifiedSearchResultsListModel::slotSearchTermEditingFinished);
        _unifiedSearchTextEditingFinishedTimer.start();
    }
}

QString UnifiedSearchResultsListModel::searchTerm() const
{
    return _searchTerm;
}

void UnifiedSearchResultsListModel::slotSearchTermEditingFinished()
{
    disconnect(&_unifiedSearchTextEditingFinishedTimer, &QTimer::timeout, this, &UnifiedSearchResultsListModel::slotSearchTermEditingFinished);
    QString term = searchTerm();

    if (_providers.isEmpty()) {
        auto *job = new JsonApiJob(_accountState->account(), QLatin1String("ocs/v2.php/search/providers"));
        QObject::connect(job, &JsonApiJob::jsonReceived, [&, this](const QJsonDocument &json) {
            const auto providerList = json.object().value("ocs").toObject().value("data").toVariant().toList();

            _providers.reserve(providerList.size());

            for (const auto& provider : providerList) {
                const auto providerMap = provider.toMap();
                const auto providerId = providerMap["id"].toString();
                if (!providerId.isEmpty()) {
                    _providers.push_back(providerId);
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

void UnifiedSearchResultsListModel::startSearch()
{
    for (const auto& provider : _providers) {
        startSearchForProvider(provider);
    }
}

void UnifiedSearchResultsListModel::startSearchForProvider(const QString &providerId)
{
    auto *job = new JsonApiJob(_accountState->account(), QLatin1String("ocs/v2.php/search/providers/%1/search").arg(providerId));
    QUrlQuery params;
    params.addQueryItem(QStringLiteral("term"), searchTerm());
    job->addQueryParams(params);
    QObject::connect(job, &JsonApiJob::jsonReceived, [&](const QJsonDocument &json) {
        auto data = json.object().value("ocs").toObject().value("data").toObject();
        if (!data.isEmpty()) {
            const auto name = data.value("name").toString();
            const auto isPaginated = data.value("isPaginated").toBool();
            const auto cursor = data.value("cursor").toInt();
            const auto entries = data.value("entries").toVariant();
        }
    });
    job->start();
}

}
