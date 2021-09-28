#include "syncstatusmodel.h"
#include "folderman.h"
#include "navigationpanehelper.h"
#include "syncresult.h"

#include <theme.h>

namespace OCC {

Q_LOGGING_CATEGORY(lcSyncStatusModel, "nextcloud.gui.syncstatusmodel", QtInfoMsg)

SyncStatusModel::SyncStatusModel(QObject *parent)
    : QObject(parent)
{
    const auto folderMan = FolderMan::instance();

    connect(folderMan, &FolderMan::folderListChanged,
        this, &SyncStatusModel::onFolderListChanged);
    connect(folderMan, &FolderMan::folderSyncStateChange,
        this, &SyncStatusModel::onFolderSyncStateChanged);

    connectToFoldersProgress(folderMan->map());
}

double SyncStatusModel::syncProgress() const
{
    return _progress;
}

QUrl SyncStatusModel::syncIcon() const
{
    return _syncIcon;
}

bool SyncStatusModel::syncing() const
{
    return _isSyncing;
}

void SyncStatusModel::onFolderListChanged(const OCC::Folder::Map &folderMap)
{
    connectToFoldersProgress(folderMap);
}

void SyncStatusModel::onFolderSyncStateChanged(const Folder *folder)
{
    if (!folder) {
        return;
    }

    switch (folder->syncResult().status()) {
    case SyncResult::Success:
        setSyncing(false);
        setSyncStatusString(tr("All synced!"));
        setSyncStatusDetailString("");
        setSyncIcon(Theme::instance()->syncStatusOk());
        break;
    case SyncResult::Error:
        setSyncing(false);
        setSyncStatusString(tr("Some files couldn't be synced!"));
        setSyncStatusDetailString(tr("See below for errors"));
        setSyncIcon(Theme::instance()->syncStatusError());
        break;
    case SyncResult::SyncRunning:
        setSyncing(true);
        setSyncStatusString(tr("Syncing"));
        setSyncStatusDetailString("");
        setSyncIcon(Theme::instance()->syncStatusRunning());
        break;
    case SyncResult::Paused:
        setSyncing(false);
        setSyncStatusString(tr("Sync paused"));
        setSyncStatusDetailString("");
        setSyncIcon(Theme::instance()->syncStatusPause());
        break;
    case SyncResult::Problem:
        setSyncing(false);
        setSyncStatusString(tr("Some files had problems during the sync!"));
        setSyncStatusDetailString(tr("See below for warnings"));
        setSyncIcon(Theme::instance()->syncStatusWarning());
        break;
    case SyncResult::Undefined:
    case SyncResult::NotYetStarted:
    case SyncResult::SyncPrepare:
    case SyncResult::SyncAbortRequested:
    case SyncResult::SetupError:
        qCWarning(lcSyncStatusModel) << "Sync state will not be handled";
        break;
    }
}

constexpr double calculateOverallPercent(qint64 totalFileCount, qint64 completedFile, qint64 totalSize, qint64 completedSize)
{
    int overallPercent = 0;
    if (totalFileCount > 0) {
        // Add one 'byte' for each file so the percentage is moving when deleting or renaming files
        overallPercent = qRound(double(completedSize + completedFile) / double(totalSize + totalFileCount) * 100.0);
    }
    overallPercent = qBound(0, overallPercent, 100);
    return overallPercent / 100.0;
}

void SyncStatusModel::onFolderProgressInfo(const ProgressInfo &progress)
{
    const qint64 completedSize = progress.completedSize();
    const qint64 currentFile = progress.currentFile();
    const qint64 completedFile = progress.completedFiles();
    const qint64 totalSize = qMax(completedSize, progress.totalSize());
    const qint64 totalFileCount = qMax(currentFile, progress.totalFiles());

    setSyncProgress(calculateOverallPercent(totalFileCount, completedFile, totalSize, completedSize));

    if (totalSize > 0) {
        const auto completedSizeString = Utility::octetsToString(completedSize);
        const auto totalSizeString = Utility::octetsToString(totalSize);

        if (progress.trustEta()) {
            setSyncString(tr("%1 of %2 · %3 left")
                                    .arg(completedSizeString, totalSizeString)
                                    .arg(Utility::durationToDescriptiveString1(progress.totalProgress().estimatedEta)));
        } else {
            setSyncString(tr("%1 of %2").arg(completedSizeString, totalSizeString));
        }
    }

    if (totalFileCount > 0) {
        setSyncStatusString(tr("Syncing file %1 of %2").arg(currentFile).arg(totalFileCount));
    }
}

void SyncStatusModel::setSyncing(bool value)
{
    if (value == _isSyncing) {
        return;
    }

    _isSyncing = value;
    emit syncingChanged();
}

QString SyncStatusModel::syncString() const
{
    return _syncString;
}

void SyncStatusModel::setSyncString(const QString &value)
{
    if (_syncString == value) {
        return;
    }

    _syncString = value;
    emit syncStringChanged();
}

void SyncStatusModel::setSyncProgress(double value)
{
    if (_progress == value) {
        return;
    }

    _progress = value;
    emit syncProgressChanged();
}

void SyncStatusModel::setSyncStatusString(const QString& value)
{
    if (_syncStatusString == value) {
        return;
    }

    _syncStatusString = value;
    emit syncStatusStringChanged();
}

QString SyncStatusModel::syncStatusString() const
{
    return _syncStatusString;
}

QString SyncStatusModel::syncStatusDetailString() const
{
    return _syncStatusDetailString;
}

void SyncStatusModel::setSyncIcon(const QUrl &value)
{
    if (_syncIcon == value) {
        return;
    }

    _syncIcon = value;
    emit syncIconChanged();
}

void SyncStatusModel::setSyncStatusDetailString(const QString &value)
{
    if (_syncStatusDetailString == value) {
        return;
    }

    _syncStatusDetailString = value;
    emit syncStatusDetailStringChanged();
}

void SyncStatusModel::connectToFoldersProgress(const Folder::Map &folderMap)
{
    for (const auto &folder : folderMap) {
        connect(folder, &Folder::progressInfo, this, &SyncStatusModel::onFolderProgressInfo, Qt::UniqueConnection);
    }
}
}
