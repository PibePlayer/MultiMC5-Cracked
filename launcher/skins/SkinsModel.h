/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#pragma once

#include <QMutex>
#include <QAbstractListModel>
#include <QFile>
#include <QDir>
#include <QSet>
#include <QTimer>
#include <QtGui/QIcon>
#include <memory>

#include "settings/Setting.h"

#include "QObjectPtr.h"
#include <nonstd/optional>

#include "SkinTypes.h"

class QFileSystemWatcher;

class SkinsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SkinsModel(QString path, QObject *parent = 0);
    virtual ~SkinsModel() noexcept {};

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual QStringList mimeTypes() const override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    void installSkins(const QStringList &paths);
    bool installSkin(const QString &path);
    QModelIndex installSkin(const QByteArray& data, const QString &playerName);

    bool deleteSkin(const QString &key);
    bool skinFileExists(const QString &key) const;

    const Skins::SkinEntry& at(int row) const;
    const Skins::SkinEntry& skinEntry(const QString& key) const;
    const Skins::SkinEntry& skinEntryByTextureID(const QString& textureID) const;

    QString path() const;

signals:
    void skinUpdated(const QString &key);
    void listUpdated();

private:
    void startWatching();
    void stopWatching();

    int getSkinIndex(const QString &key) const;
    void scheduleRemoval(const QString &key);
    bool cancelRemoval(const QString &key);
    void reindex();
    void addDefaultSkin(const QString& name, const QString& slimPath, const QString& classicPath);

public slots:
    void directoryChanged(const QString &path);

private slots:
    void fileChanged(const QString &path);
    void SettingChanged(const Setting & setting, QVariant value);
    void removalTimerTriggered();

private:
    shared_qobject_ptr<QFileSystemWatcher> m_watcher;
    bool m_isWatching = false;
    QMap<QString, int> m_nameIndex;
    QVector<Skins::SkinEntry> m_skins;
    QDir m_dir;
    QTimer m_removalTimer;
    QSet<QString> m_toRemove;
    QSet<QString> m_reservedNames;
};
