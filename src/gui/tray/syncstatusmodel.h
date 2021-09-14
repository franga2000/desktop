#pragma once

#include "folderman.h"

#include <theme.h>
#include <folder.h>

#include <QObject>

namespace OCC {

class SyncStatusModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(double syncProgress READ syncProgress NOTIFY syncProgressChanged)
    Q_PROPERTY(QUrl syncIcon READ syncIcon NOTIFY syncIconChanged)
    Q_PROPERTY(bool syncing READ syncing NOTIFY syncingChanged)
    Q_PROPERTY(QString syncString READ syncString NOTIFY syncStringChanged)
    Q_PROPERTY(QString syncStatusString READ syncStatusString NOTIFY syncStatusStringChanged)
    Q_PROPERTY(QString syncStatusDetailString READ syncStatusDetailString NOTIFY syncStatusDetailStringChanged)

public:
    explicit SyncStatusModel(QObject *parent = nullptr);

    double syncProgress() const;
    QUrl syncIcon() const;
    bool syncing() const;
    QString syncString() const;
    QString syncStatusString() const;
    QString syncStatusDetailString() const;

signals:
    void syncProgressChanged();
    void syncIconChanged();
    void syncingChanged();
    void syncStringChanged();
    void syncStatusStringChanged();
    void syncStatusDetailStringChanged();

private:
    void connectToFoldersProgress(const Folder::Map &map);

    void onFolderListChanged(const OCC::Folder::Map &folderMap);
    void onFolderProgressInfo(const ProgressInfo &progress);
    void onFolderSyncStateChanged(const Folder *folder);

    void setSyncProgress(double value);
    void setSyncing(bool value);
    void setSyncString(const QString &value);
    void setSyncStatusString(const QString &value);
    void setSyncStatusDetailString(const QString &value);
    void setSyncIcon(const QUrl &value);

    QUrl _syncIcon = Theme::instance()->syncStatusOk();
    double _progress = 1.0;
    bool _isSyncing = false;
    QString _syncString;
    QString _syncStatusString;
    QString _syncStatusDetailString;
};
}
