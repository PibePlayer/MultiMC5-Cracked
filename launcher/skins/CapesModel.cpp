/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include "CapesModel.h"
#include "CapeCache.h"
#include "Application.h"
#include "minecraft/auth/AccountTask.h"

#include <QMap>
#include <QDebug>

CapesModel::CapesModel(QObject *parent) : QAbstractListModel(parent)
{
    auto capeCache = APPLICATION->capeCache();
    connect(capeCache.get(), &CapeCache::capeReady, this, &CapesModel::capeImageUpdated);
}

void CapesModel::setAccount(MinecraftAccountPtr account)
{
    if(account == m_account)
    {
        return;
    }
    // beginResetModel();
    {
        m_capes.clear();
        m_uuidIndex.clear();

        m_account = account;
        auto capeCache = APPLICATION->capeCache();
        m_capes.push_back(Skins::CapeEntry{"", tr("Nothing"), QImage(":/skins/textures/no_cape.png").scaled(64, 64, Qt::AspectRatioMode::KeepAspectRatio)});
        m_uuidIndex[""] = 0;
        if(m_account)
        {
            for(auto& cape: m_account->accountData()->minecraftProfile.capes)
            {
                Skins::CapeEntry entry;
                entry.alias = cape.alias;
                entry.preview = capeCache->getCapeImage(cape.id).copy(1, 1, 10, 16).scaled(64, 64, Qt::AspectRatioMode::KeepAspectRatio);
                entry.uuid = cape.id;
                m_uuidIndex[cape.id] = m_capes.size();
                m_capes.push_back(entry);
            }
        }
    }
    endResetModel();
}

void CapesModel::capeImageUpdated(const QString& uuid)
{
    auto iter = m_uuidIndex.constFind(uuid);
    if(iter ==  m_uuidIndex.constEnd())
        return;

    auto capeCache = APPLICATION->capeCache();
    int row = iter.value();
    auto idx = index(row);
    m_capes[row].preview = capeCache->getCapeImage(uuid).copy(1, 1, 10, 16).scaled(64, 64, Qt::AspectRatioMode::KeepAspectRatio);
    emit dataChanged(idx, idx, {Qt::DecorationRole});
}


QVariant CapesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();

    if (row < 0 || row >= m_capes.size())
        return QVariant();

    switch (role)
    {
    case Qt::DecorationRole:
        return m_capes[row].preview;
    case Qt::DisplayRole:
        return m_capes[row].alias;
    case Qt::UserRole:
        return m_capes[row].uuid;
    default:
        return QVariant();
    }
}

int CapesModel::rowCount(const QModelIndex& parent) const
{
    return m_capes.count();
}

QString CapesModel::at(int row) const
{
    if(row < 0 || row >= m_capes.size())
    {
        return QString();
    }
    return m_capes[row].uuid;
}
