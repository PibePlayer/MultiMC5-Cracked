/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "AccountTask.h"
#include "MinecraftAccount.h"

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QByteArray>

#include <QDebug>
#include "Parsers.h"

AccountTask::AccountTask(AccountData *data, QObject *parent)
    : Task(parent), m_data(data)
{
    changeState(AccountTaskState::STATE_CREATED);
}

QString AccountTask::getStateMessage() const
{
    switch (m_taskState)
    {
    case AccountTaskState::STATE_CREATED:
        return "Waiting...";
    case AccountTaskState::STATE_WORKING:
        return tr("Sending request to auth servers...");
    case AccountTaskState::STATE_SUCCEEDED:
        return tr("Authentication task succeeded.");
    case AccountTaskState::STATE_OFFLINE:
        return tr("Failed to contact the authentication server.");
    case AccountTaskState::STATE_FAILED_SOFT:
        return tr("Encountered an error during authentication.");
    case AccountTaskState::STATE_FAILED_MUST_MIGRATE:
        return tr("Failed to authenticate. The account must be migrated to a Microsoft account to be usable.");
    case AccountTaskState::STATE_FAILED_HARD:
        return tr("Failed to authenticate. The session has expired.");
    case AccountTaskState::STATE_FAILED_GONE:
        return tr("Failed to authenticate. The account no longer exists.");
    default:
        return tr("...");
    }
}

bool AccountTask::changeState(AccountTaskState newState, QString reason)
{
    m_taskState = newState;
    setStatus(getStateMessage());
    switch(newState) {
        case AccountTaskState::STATE_CREATED: {
            m_data->errorString.clear();
            return true;
        }
        case AccountTaskState::STATE_WORKING: {
            m_data->accountState = AccountState::Working;
            return true;
        }
        case AccountTaskState::STATE_SUCCEEDED: {
            m_data->accountState = AccountState::Online;
            emitSucceeded();
            return false;
        }
        case AccountTaskState::STATE_OFFLINE: {
            m_data->errorString = reason;
            m_data->accountState = AccountState::Offline;
            emitFailed(reason);
            return false;
        }
        case AccountTaskState::STATE_FAILED_SOFT: {
            m_data->errorString = reason;
            m_data->accountState = AccountState::Errored;
            emitFailed(reason);
            return false;
        }
        case AccountTaskState::STATE_FAILED_MUST_MIGRATE: {
            m_data->errorString = reason;
            m_data->accountState = AccountState::MustMigrate;
            emitFailed(reason);
            return false;
        }
        case AccountTaskState::STATE_FAILED_HARD: {
            m_data->errorString = reason;
            m_data->accountState = AccountState::Expired;
            emitFailed(reason);
            return false;
        }
        case AccountTaskState::STATE_FAILED_GONE: {
            m_data->errorString = reason;
            m_data->accountState = AccountState::Gone;
            emitFailed(reason);
            return false;
        }
        default: {
            QString error = tr("Unknown account task state: %1").arg(int(newState));
            m_data->accountState = AccountState::Errored;
            emitFailed(error);
            return false;
        }
    }
}

MojangError MojangError::fromJSON(QByteArray data, QNetworkReply::NetworkError networkError)
{
    MojangError out;
    out.rawError = QString::fromUtf8(data);
    out.networkError = networkError;

    auto doc = QJsonDocument::fromJson(data, &out.parseError);
    if(out.parseError.error != QJsonParseError::NoError)
    {
        out.jsonParsed = false;
    }
    else
    {
        auto object = doc.object();
        Parsers::getString(object.value("path"), out.path);
        QJsonValue details = object.value("details");
        if(details.isObject())
        {
            QJsonObject detailsObj = details.toObject();
            Parsers::getString(detailsObj.value("status"), out.detailsStatus);
        }
        Parsers::getString(object.value("error"), out.error);
        Parsers::getString(object.value("errorMessage"), out.errorMessage);
        out.jsonParsed = true;
    }


    return out;
}

QString MojangError::toString() const
{
    QString outString;
    QTextStream out(&outString);
    out << "Network error:" << networkError << "\n";
    if(jsonParsed)
    {
        if(!path.isNull())
        {
            out << "path: " << path << "\n";
        }
        if(!error.isNull())
        {
            out << "error: " << error << "\n";
        }
        if(!errorMessage.isNull())
        {
            out << "errorMessage: " << errorMessage << "\n";
        }
        if(!detailsStatus.isNull())
        {
            out << "details.status: " << detailsStatus << "\n";
        }
    }
    else
    {
        out << "Mojang error failed to parse with error: " << parseError.errorString() << "\n";
        out << "Raw contents:\n" << rawError << "\n";
    }
    return outString;
}
