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

#ifndef UNIFIEDSEARHRESULT_H
#define UNIFIEDSEARHRESULT_H

#include <QtCore>
#include <QIcon>

namespace OCC {
/**
 * @brief The UnifiedSearchResult class
 */

class UnifiedSearchResult
{
    Q_GADGET

    Q_PROPERTY(QString title MEMBER _title)

public:
    QString _title;
};

bool operator==(const UnifiedSearchResult &rhs, const UnifiedSearchResult &lhs);
bool operator<(const UnifiedSearchResult &rhs, const UnifiedSearchResult &lhs);
}

Q_DECLARE_METATYPE(OCC::UnifiedSearchResult)

#endif // UNIFIEDSEARHRESULT_H
