/* Copyright 2024 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#pragma once
#include <QObject>

#include "QObjectPtr.h"
#include "minecraft/auth/AuthStep.h"


class MinecraftProfileCreateStep : public AuthStep {
    Q_OBJECT

public:
    explicit MinecraftProfileCreateStep(AccountData *data, const QString& profileName);
    virtual ~MinecraftProfileCreateStep() noexcept;

    void perform() override;

    QString describe() override;

signals:
    void apiError(const MojangError& error);

private slots:
    void onRequestDone(QNetworkReply::NetworkError, QByteArray, QList<QNetworkReply::RawHeaderPair>);

private:
    QString m_profileName;
};

