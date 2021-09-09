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

#ifndef UNIFIEDSEARCHRESULTSLISTMODEL_H
#define UNIFIEDSEARCHRESULTSLISTMODEL_H

#include <QtCore>

namespace OCC {

class AccountState;

/**
 * @brief The UnifiedSearchResultsListModel
 * @ingroup gui
 *
 * Simple list model to provide the list view with data for the Unified Search results.
 */

class UnifiedSearchResultsListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit UnifiedSearchResultsListModel(AccountState *accountState, QObject *parent = nullptr);
    ~UnifiedSearchResultsListModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void setSearchTerm(const QString &term);
    QString searchTerm() const;

private slots:
    void slotSearchTermEditingFinished();

private:
    void startSearch();
    void startSearchForProvider(const QString &providerId);

private:
    QTimer _unifiedSearchTextEditingFinishedTimer;
    QString _searchTerm;
    QStringList _providers;
    AccountState *_accountState;
};
}

#endif // UNIFIEDSEARCHRESULTSLISTMODEL_H
