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

#include "UnifiedSearchResultImageProvider.h"

#include "accountmanager.h"
#include "iconjob.h"

#include "tray/UserModel.h"

#include <QImage>
#include <QThreadPool>

namespace OCC {

class AsyncImageResponseRunnable : public QObject, public QRunnable
{
    Q_OBJECT

signals:
    void done(QImage image);

public:
    AsyncImageResponseRunnable(const QString &id, const QSize &requestedSize)
        : m_id(id)
        , m_requestedSize(requestedSize)
    {
    }

    void run() override
    {
        const QUrl iconUrl = QUrl(m_id);

        if (!iconUrl.isValid() || iconUrl.scheme().isEmpty()) {
            emit done(QImage());
            return;
        }

        auto image = QImage(16, 16, QImage::Format_ARGB32);

        QEventLoop loop;
        auto *iconJob = new IconJob(_account, iconUrl, this);
        connect(iconJob, &IconJob::jobFinished, this, [&loop, &image](int statusCode, const QByteArray reply) {
            if (statusCode == 200) {
                image = QImage::fromData(reply);
                bool isValid = !image.isNull();
            }
            loop.quit();
        });
        iconJob->start();
        loop.exec();
        emit done(image);
    }

    void setAccount(AccountPtr acount)
    {
        _account = acount;
    }

private:
    QString m_id;
    AccountPtr _account;
    QSize m_requestedSize;
};

class AsyncImageResponse : public QQuickImageResponse
{
public:
    AsyncImageResponse()
    {
    }

    void init(const QString &id, const QSize &requestedSize, QThreadPool *pool)
    {
        auto runnable = new AsyncImageResponseRunnable(id, requestedSize);
        if (_account) {
            runnable->setAccount(_account);
        }
        connect(runnable, &AsyncImageResponseRunnable::done, this, &AsyncImageResponse::handleDone);
        pool->start(runnable);
    }

    void setAccount(AccountPtr acount)
    {
        _account = acount;
    }

    void handleDone(QImage image)
    {
        m_image = image;
        emit finished();
    }

    QQuickTextureFactory *textureFactory() const override
    {
        return QQuickTextureFactory::textureFactoryForImage(m_image);
    }

    QImage m_image;

    AccountPtr _account;
};

QQuickImageResponse *UnifiedSearchResultImageProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    AsyncImageResponse *response = new AsyncImageResponse;
    if (UserModel::instance()->currentUser()) {
        response->setAccount(UserModel::instance()->currentUser()->account());
    }
    response->init(id, requestedSize, &pool);
    return response;
}

}
#include "UnifiedSearchResultImageProvider.moc"