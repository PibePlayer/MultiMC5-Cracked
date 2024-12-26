/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#pragma once
#include <QObject>

#include "QObjectPtr.h"
#include "minecraft/auth/AuthStep.h"
#include "skins/SkinTypes.h"

class SetSkinStep : public AuthStep {
    Q_OBJECT

public:
    explicit SetSkinStep(AccountData *data, Skins::Model model, QByteArray skinData);
    virtual ~SetSkinStep() noexcept;

    void perform() override;

    QString describe() override;

signals:
    void apiError(const MojangError& error);

private slots:
    void onRequestDone(QNetworkReply::NetworkError, QByteArray, QList<QNetworkReply::RawHeaderPair>);

private:
    Skins::Model m_model;
    QByteArray m_skinData;
};

