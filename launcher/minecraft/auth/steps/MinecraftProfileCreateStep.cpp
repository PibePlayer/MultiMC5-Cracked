/* Copyright 2024 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include "MinecraftProfileCreateStep.h"

#include <QNetworkRequest>

#include "minecraft/auth/AuthRequest.h"
#include "minecraft/auth/Parsers.h"

#include "BuildConfig.h"
#include <QJsonDocument>

MinecraftProfileCreateStep::MinecraftProfileCreateStep(AccountData* data, const QString& profileName) : AuthStep(data), m_profileName(profileName) {
}

MinecraftProfileCreateStep::~MinecraftProfileCreateStep() noexcept = default;

QString MinecraftProfileCreateStep::describe() {
    return tr("Creating the Minecraft profile '%1'.").arg(m_profileName);
}


void MinecraftProfileCreateStep::perform() {
    auto url = QString("%1/minecraft/profile").arg(BuildConfig.API_BASE);
    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_data->yggdrasilToken.token).toUtf8());

    QString payloadTemplate("{\"profileName\":\"%1\"}");
    auto data = payloadTemplate.arg(m_profileName).toUtf8();

    AuthRequest *requestor = new AuthRequest(this);
    connect(requestor, &AuthRequest::finished, this, &MinecraftProfileCreateStep::onRequestDone);
    requestor->post(request, data);
}

void MinecraftProfileCreateStep::onRequestDone(
    QNetworkReply::NetworkError error,
    QByteArray data,
    QList<QNetworkReply::RawHeaderPair> headers
) {
    auto requestor = qobject_cast<AuthRequest *>(QObject::sender());
    requestor->deleteLater();

    if(error == QNetworkReply::NoError) {
        emit finished(AccountTaskState::STATE_WORKING, tr(""));
        return;
    }
    else {
        auto parsedError = MojangError::fromJSON(data, error);
        if(parsedError.detailsStatus == "ALREADY_REGISTERED")
        {
            emit finished(AccountTaskState::STATE_WORKING, tr(""));
            return;
        }
        QString errorString = parsedError.toString();
        emit apiError(parsedError);
        qWarning() << "Failed to set up player profile: " << errorString;
        emit finished(AccountTaskState::STATE_SUCCEEDED, tr(""));
    }
}
