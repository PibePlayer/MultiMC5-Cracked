/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include "SkinsModel.h"

#include "SkinUtils.h"

#include <FileSystem.h>
#include <QMap>
#include <QEventLoop>
#include <QMimeData>
#include <QUrl>
#include <QFileSystemWatcher>
#include <QSet>
#include <QDebug>
#include <QSaveFile>

namespace{
    Skins::SkinEntry placeholderEntry = Skins::SkinEntry();
}

SkinsModel::SkinsModel(QString path, QObject *parent) : QAbstractListModel(parent)
{
    m_watcher.reset(new QFileSystemWatcher());
    m_isWatching = false;
    connect(m_watcher.get(), SIGNAL(directoryChanged(QString)), SLOT(directoryChanged(QString)));
    connect(m_watcher.get(), SIGNAL(fileChanged(QString)), SLOT(fileChanged(QString)));
    m_removalTimer.setSingleShot(true);
    m_removalTimer.setInterval(100);
    connect(&m_removalTimer, &QTimer::timeout, this, &SkinsModel::removalTimerTriggered);

    addDefaultSkin("Steve", ":/skins/textures/Steve_Slim.png", ":/skins/textures/Steve_Classic.png");
    addDefaultSkin("Alex", ":/skins/textures/Alex_Slim.png", ":/skins/textures/Alex_Classic.png");
    addDefaultSkin("Ari", ":/skins/textures/Ari_Slim.png", ":/skins/textures/Ari_Classic.png");
    addDefaultSkin("Efe", ":/skins/textures/Efe_Slim.png", ":/skins/textures/Efe_Classic.png");
    addDefaultSkin("Kai", ":/skins/textures/Kai_Slim.png", ":/skins/textures/Kai_Classic.png");
    addDefaultSkin("Makena", ":/skins/textures/Makena_Slim.png", ":/skins/textures/Makena_Classic.png");
    addDefaultSkin("Noor", ":/skins/textures/Noor_Slim.png", ":/skins/textures/Noor_Classic.png");
    addDefaultSkin("Sunny", ":/skins/textures/Sunny_Slim.png", ":/skins/textures/Sunny_Classic.png");
    addDefaultSkin("Zuri", ":/skins/textures/Zuri_Slim.png", ":/skins/textures/Zuri_Classic.png");

    directoryChanged(path);
}

QString SkinsModel::path() const
{
    return m_dir.path();
}

void SkinsModel::addDefaultSkin(const QString& name, const QString& slimPath, const QString& classicPath)
{
    int index = m_skins.size();
    m_skins.push_back(Skins::SkinEntry(name, slimPath, classicPath));
    m_reservedNames.insert(name);
    m_nameIndex[name] = index;
}


const Skins::SkinEntry & SkinsModel::at(int row) const
{
    if(row < 0 || row >= m_skins.size())
    {
        return placeholderEntry;
    }
    return m_skins[row];
}

void SkinsModel::directoryChanged(const QString &path)
{
    QDir new_dir (path);
    if(m_dir.absolutePath() != new_dir.absolutePath())
    {
        m_dir.setPath(path);
        m_dir.refresh();
        if(m_isWatching)
            stopWatching();
        startWatching();
    }
    if(!m_dir.exists())
    {
        if(!FS::ensureFolderPathExists(m_dir.absolutePath()))
        {
            return;
        }
    }
    m_dir.refresh();

    auto new_list = m_dir.entryList(QDir::Files, QDir::Name);
    QSet<QString> new_set;
    for (auto it = new_list.begin(); it != new_list.end(); it++)
    {
        QString &foo = (*it);
        foo = m_dir.filePath(foo);
        QFileInfo fooInfo(foo);
        // Do not recognize files that match name with internal skins
        if(m_reservedNames.contains(fooInfo.baseName()))
            continue;
        new_set.insert(foo);
    }

    QList<QString> current_list;
    for (auto &it : m_skins)
    {
        // Do not take internal skins into account
        if(it.internal)
        {
            continue;
        }
        current_list.push_back(it.filename);
    }
    QSet<QString> current_set = current_list.toSet();

    QSet<QString> to_remove = current_set;
    to_remove -= new_set;

    QSet<QString> to_add = new_set;
    to_add -= current_set;

    bool removed = false;
    for (auto remove : to_remove)
    {
        m_watcher->removePath(remove);
        QFileInfo rmfile(remove);
        QString key = rmfile.baseName();
        int idx = getSkinIndex(key);
        if (idx == -1)
            continue;
        beginRemoveRows(QModelIndex(), idx, idx);
        m_skins.remove(idx);
        endRemoveRows();
        removed = true;
    }
    if(removed)
    {
        reindex();
    }

    bool added = false;
    for (auto add : to_add)
    {
        m_watcher->addPath(add);
        QByteArray data;
        QImage image;
        QString key;
        QString textureId;
        if(Skins::readSkinFromFile(add, data, image, key, textureId))
        {
            auto idx = m_skins.size();
            beginInsertRows(QModelIndex(), idx, idx);
            m_skins.push_back(Skins::SkinEntry(key, add, image, textureId, data));
            m_nameIndex[key] = idx;
            endInsertRows();
            added = true;
        }
    }
    if(removed || added)
    {
        emit listUpdated();
    }
}

void SkinsModel::fileChanged(const QString &path)
{
    QByteArray data;
    QImage image;
    QString key;
    QString textureId;
    if(Skins::readSkinFromFile(path, data, image, key, textureId))
    {
        cancelRemoval(key);
        // new file is valid
        int row = getSkinIndex(key);
        if(row != -1)
        {
            auto rowIndex = index(row);
            m_skins[row] = Skins::SkinEntry(key, path, image, textureId, data);
            emit dataChanged(rowIndex, rowIndex);
            emit skinUpdated(key);
            emit listUpdated();
        }
        else
        {
            row = m_skins.size();
            beginInsertRows(QModelIndex(), row, row);
            m_skins.push_back(Skins::SkinEntry(key, path, image, textureId, data));
            m_nameIndex[key] = row;
            endInsertRows();
            emit listUpdated();
        }
    }
    else
    {
        // new file is not valid
        int row = getSkinIndex(key);
        if(row != -1)
        {
            // file became invalid. We remove it.
            scheduleRemoval(key);
        }
        else
        {
            // it was invalid, it is still invalid -> do nothing.
        }
    }
}

void SkinsModel::scheduleRemoval(const QString& key)
{
    m_toRemove.insert(key);
    m_removalTimer.start();
}

bool SkinsModel::cancelRemoval(const QString& key)
{
    bool removed = m_toRemove.remove(key);
    if(m_toRemove.isEmpty())
    {
        m_removalTimer.stop();
    }
    return removed;
}

void SkinsModel::removalTimerTriggered()
{
    bool removed = false;
    for(auto& name: m_toRemove)
    {
        int row = getSkinIndex(name);
        if(row != -1)
        {
            removed = true;
            beginRemoveRows(QModelIndex(), row, row);
            m_skins.remove(row);
            endRemoveRows();
            emit listUpdated();
        }
    }
    if(removed)
    {
        reindex();
    }
}


void SkinsModel::SettingChanged(const Setting &setting, QVariant value)
{
    if(setting.id() != "SkinsDir")
        return;

    directoryChanged(value.toString());
}

void SkinsModel::startWatching()
{
    auto abs_path = m_dir.absolutePath();
    FS::ensureFolderPathExists(abs_path);
    m_isWatching = m_watcher->addPath(abs_path);
    if (m_isWatching)
    {
        qDebug() << "Started watching " << abs_path;
    }
    else
    {
        qDebug() << "Failed to start watching " << abs_path;
    }
}

void SkinsModel::stopWatching()
{
    m_watcher->removePaths(m_watcher->files());
    m_watcher->removePaths(m_watcher->directories());
    m_isWatching = false;
}

QStringList SkinsModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list";
    return types;
}
Qt::DropActions SkinsModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

bool SkinsModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data || !(action & supportedDropActions()))
        return false;

    if (data->hasUrls())
    {
        auto urls = data->urls();
        QStringList skinFiles;
        for (auto url : urls)
        {
            if (!url.isLocalFile())
                continue;
            skinFiles += url.toLocalFile();
            // TODO: add drag and drop from websites
        }
        installSkins(skinFiles);
        return true;
    }
    return false;
}

Qt::ItemFlags SkinsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);
    if (index.isValid())
        return Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QVariant SkinsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();

    if (row < 0 || row >= m_skins.size())
        return QVariant();

    switch (role)
    {
    case Qt::DecorationRole:
        return m_skins[row].getListTexture();
    case Qt::DisplayRole:
        return m_skins[row].name;
    default:
        return QVariant();
    }
}

int SkinsModel::rowCount(const QModelIndex &parent) const
{
    return m_skins.size();
}

void SkinsModel::installSkins(const QStringList &iconFiles)
{
    for (QString file : iconFiles)
    {
        QFileInfo fileinfo(file);
        if (!fileinfo.isReadable() || !fileinfo.isFile())
            continue;

        QString suffix = fileinfo.suffix().toLower();
        if (suffix != "png")
            continue;

        QString target = FS::PathCombine(m_dir.dirName(), fileinfo.fileName());
        if (!QFile::copy(file, target))
            continue;
    }
}

bool SkinsModel::installSkin(const QString &file)
{
    QFileInfo fileinfo(file);
    if(!fileinfo.isReadable() || !fileinfo.isFile())
    {
        return false;
    }
    QString target = FS::PathCombine(m_dir.dirName(), fileinfo.baseName());

    return QFile::copy(file, target);
}

QModelIndex SkinsModel::installSkin(const QByteArray& data, const QString& playerName)
{
    int num = 0;
    QString path;
    QString key;
    bool found = false;
    while(!found)
    {
        if(num == 0)
        {
            key = playerName;
        }
        else
        {
            key = playerName + "_" + QString::number(num);
        }

        if (num >= 100)
            return QModelIndex();
        num++;
        path = FS::PathCombine(m_dir.path(), key  + ".png");
        found = !QFileInfo(path).exists();
    };
    QImage image;
    QString textureId;
    if(Skins::readSkinFromData(data, image, textureId))
    {
        QSaveFile out(path);
        out.open(QIODevice::WriteOnly);
        out.write(data);
        if(out.commit())
        {
            int row = m_skins.size();
            beginInsertRows(QModelIndex(), row, row);
            m_skins.push_back(Skins::SkinEntry(key, path, image, textureId, data));
            m_nameIndex[key] = row;
            endInsertRows();
            emit listUpdated();
            return index(row);
        }
    }
    return QModelIndex();
}


bool SkinsModel::skinFileExists(const QString &key) const
{
    return getSkinIndex(key) != -1;
}

bool SkinsModel::deleteSkin(const QString &key)
{
    int idx = getSkinIndex(key);
    if (idx == -1)
        return false;
    auto &entry = m_skins[idx];
    return QFile::remove(entry.filename);
}

void SkinsModel::reindex()
{
    m_nameIndex.clear();
    int i = 0;
    for (auto &iter : m_skins)
    {
        m_nameIndex[iter.name] = i;
        i++;
    }
}

int SkinsModel::getSkinIndex(const QString &key) const
{
    auto iter = m_nameIndex.find(key);
    if (iter != m_nameIndex.end())
        return *iter;

    return -1;
}

const Skins::SkinEntry & SkinsModel::skinEntry(const QString& key) const
{
    auto iter = m_nameIndex.find(key);
    if (iter != m_nameIndex.end())
    {
        return m_skins[*iter];
    }

    return placeholderEntry;
}

const Skins::SkinEntry & SkinsModel::skinEntryByTextureID(const QString& textureID) const
{
    for(const auto& entry: m_skins)
    {
        if(entry.matchesId(textureID))
        {
            return entry;
        }
    }
    return placeholderEntry;
}
