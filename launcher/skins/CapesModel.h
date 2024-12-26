/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#pragma once

#include <QMutex>
#include <QMap>
#include <QVector>
#include <QAbstractListModel>

#include "minecraft/auth/MinecraftAccount.h"

#include "SkinTypes.h"

class CapesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CapesModel(QObject *parent = 0);
    virtual ~CapesModel() noexcept = default;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void setAccount(MinecraftAccountPtr account);
    MinecraftAccountPtr account() const {
        return m_account;
    }

    QString at(int row) const;

private slots:
    void capeImageUpdated(const QString& uuid);

private:
    MinecraftAccountPtr m_account;
    QMap<QString, int> m_uuidIndex;
    QVector<Skins::CapeEntry> m_capes;
};
