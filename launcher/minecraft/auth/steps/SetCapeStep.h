/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#pragma once
#include <QObject>

#include "QObjectPtr.h"
#include "minecraft/auth/AuthStep.h"


class SetCapeStep : public AuthStep {
    Q_OBJECT

public:
    explicit SetCapeStep(AccountData *data, const QString& capeId);
    virtual ~SetCapeStep() noexcept;

    void perform() override;

    QString describe() override;

signals:
    void apiError(const MojangError& error);

private slots:
    void onRequestDone(QNetworkReply::NetworkError, QByteArray, QList<QNetworkReply::RawHeaderPair>);

private:
    QString m_capeId;
};
