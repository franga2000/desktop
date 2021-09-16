/*
 * Copyright (C) by Camila Ayres <hello@camila.codes>
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

#ifndef ICONJOB_H
#define ICONJOB_H

#include "abstractnetworkjob.h"

namespace OCC {

/**
 * @brief Job to fetch a icon
 * @ingroup gui
 */
class IconJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit IconJob(AccountPtr account, const QUrl &url, QObject *parent = nullptr);

public slots:
    void start() override;
signals:
    /**
     * @param statusCode the HTTP status code
     * @param reply the content of the reply
     *
     * Signal that the job is done. If the statusCode is 200 (success) reply
     * will contain the image data in PNG. If the status code is different the content
     * of reply is undefined.
     */
    void jobFinished(int statusCode, QByteArray reply);
private slots:
    bool finished() override;

    private:
    QUrl _iconUrl;
};
}

#endif // ICONJOB_H
