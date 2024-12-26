/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include "SetSkinStep.h"

#include <QNetworkRequest>

#include "minecraft/auth/AuthRequest.h"
#include "minecraft/auth/Parsers.h"

#include "BuildConfig.h"
#include <QJsonDocument>
#include <QHttpMultiPart>

namespace {
QByteArray getVariant(Skins::Model model) {
    switch (model) {
        default:
            qWarning() << "Unknown skin type!";
        case Skins::Model::Classic:
            return "CLASSIC";
        case Skins::Model::Slim:
            return "SLIM";
    }
}
}

SetSkinStep::SetSkinStep(AccountData* data, Skins::Model model, QByteArray skinData) : AuthStep(data), m_model(model), m_skinData(skinData) {
}

SetSkinStep::~SetSkinStep() noexcept = default;

QString SetSkinStep::describe() {
    if(m_skinData.isEmpty())
    {
        return tr("Clearing skin");
    }
    else
    {
        return tr("Uploading skin");
    }
}

void SetSkinStep::perform() {
    auto url = QString("%1/minecraft/profile/skins").arg(BuildConfig.API_BASE);
    QNetworkRequest request = QNetworkRequest(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_data->yggdrasilToken.token).toUtf8());

    if(m_skinData.isEmpty())
    {
        AuthRequest *requestor = new AuthRequest(this);
        connect(requestor, &AuthRequest::finished, this, &SetSkinStep::onRequestDone);
        requestor->deleteResource(request);
    }
    else
    {
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QHttpPart skin;
        skin.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
        skin.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"skin.png\""));
        skin.setBody(m_skinData);

        QHttpPart model;
        model.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"variant\""));
        model.setBody(getVariant(m_model));

        multiPart->append(skin);
        multiPart->append(model);

        AuthRequest *requestor = new AuthRequest(this);
        multiPart->setParent(requestor);
        connect(requestor, &AuthRequest::finished, this, &SetSkinStep::onRequestDone);
        requestor->post(request, multiPart);
    }
}

void SetSkinStep::onRequestDone(
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
        qWarning() << "Failed to set player skin: " << errorString;
        emit finished(AccountTaskState::STATE_SUCCEEDED, tr(""));
    }
}


