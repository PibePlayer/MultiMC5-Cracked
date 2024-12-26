/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include "SetCapeStep.h"

#include <QNetworkRequest>

#include "minecraft/auth/AuthRequest.h"
#include "minecraft/auth/Parsers.h"

#include "BuildConfig.h"
#include <QJsonDocument>

SetCapeStep::SetCapeStep(AccountData* data, const QString& capeId) : AuthStep(data), m_capeId(capeId) {
}

SetCapeStep::~SetCapeStep() noexcept = default;

QString SetCapeStep::describe() {
    return tr("Setting cape.");
}


void SetCapeStep::perform() {
    auto url = QString("%1/minecraft/profile/capes/active").arg(BuildConfig.API_BASE);
    QNetworkRequest request = QNetworkRequest(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_data->yggdrasilToken.token).toUtf8());

    if(m_capeId.isEmpty())
    {
        AuthRequest *requestor = new AuthRequest(this);
        connect(requestor, &AuthRequest::finished, this, &SetCapeStep::onRequestDone);
        requestor->deleteResource(request);
    }
    else
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Accept", "application/json");

        QString payloadTemplate("{\"capeId\":\"%1\"}");
        auto data = payloadTemplate.arg(m_capeId).toUtf8();

        AuthRequest *requestor = new AuthRequest(this);
        connect(requestor, &AuthRequest::finished, this, &SetCapeStep::onRequestDone);
        requestor->put(request, data);
    }
}

void SetCapeStep::onRequestDone(
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
        QString errorString = parsedError.toString();
        emit apiError(parsedError);
        qWarning() << "Failed to set player cape: " << errorString;
        emit finished(AccountTaskState::STATE_SUCCEEDED, tr(""));
    }
}

