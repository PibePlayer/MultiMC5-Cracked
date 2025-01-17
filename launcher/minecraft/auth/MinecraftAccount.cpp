/* Copyright 2013-2021 MultiMC Contributors
 *
 * Authors: Orochimarufan <orochimarufan.x3@gmail.com>
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

#include "MinecraftAccount.h"

#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegExp>
#include <QStringList>
#include <QJsonDocument>

#include <QDebug>

#include <QPainter>

#include "flows/MSA.h"

#include "skins/CapeCache.h"
#include "skins/SkinUtils.h"
#include "skins/TextureMappings.h"

#include <Application.h>

MinecraftAccount::MinecraftAccount(QObject* parent) : QObject(parent) {
    data.internalId = QUuid::createUuid().toString().remove(QRegExp("[{}-]"));
}

MinecraftAccountPtr MinecraftAccount::loadFromJsonV3(const QJsonObject& json) {
    MinecraftAccountPtr account(new MinecraftAccount());
    if(account->data.resumeStateFromV3(json)) {
        return account;
    }
    return nullptr;
}

MinecraftAccountPtr MinecraftAccount::createBlankMSA()
{
    MinecraftAccountPtr account(new MinecraftAccount());
    return account;
}


QJsonObject MinecraftAccount::saveToJson() const
{
    return data.saveState();
}

AccountState MinecraftAccount::accountState() const {
    return data.accountState;
}

QString MinecraftAccount::accountStateText() const
{
    switch(data.accountState)
    {
        case AccountState::Unchecked: {
            return tr("Unchecked", "Account status");
        }
        case AccountState::Offline: {
            return tr("Offline", "Account status");
        }
        case AccountState::Online: {
            return tr("Online", "Account status");
        }
        case AccountState::Working: {
            return tr("Working", "Account status");
        }
        case AccountState::Errored: {
            return tr("Errored", "Account status");
        }
        case AccountState::Expired: {
            return tr("Expired", "Account status");
        }
        case AccountState::Gone: {
            return tr("Gone", "Account status");
        }
        case AccountState::MustMigrate: {
            return tr("Must Migrate", "Account status");
        }
        default: {
            return tr("Unknown", "Account status");
        }
    }
}


void MinecraftAccount::updateCapeCache() const
{
    auto capeCache = APPLICATION->capeCache();
    for(const auto& cape: data.minecraftProfile.capes)
    {
        capeCache->addCapeImage(cape.id, cape.url);
    }
}

QString MinecraftAccount::getCurrentCape() const
{
    return data.minecraftProfile.currentCape;
}

QByteArray MinecraftAccount::getSkin() const
{
    return data.minecraftProfile.skin.data;
}

Skins::Model MinecraftAccount::getSkinModel() const
{
    if(data.minecraftProfile.skin.variant == "CLASSIC")
        return Skins::Model::Classic;
    return Skins::Model::Slim;
}

QPixmap MinecraftAccount::getFace() const {
    QString textureID;
    QImage texture;
    if(!Skins::readSkinFromData(data.minecraftProfile.skin.data, texture, textureID))
    {
        return QPixmap();
    }
    QPixmap head = QPixmap(72, 72);
    head.fill(Qt::transparent);
    QPainter painter(&head);
    painter.fillRect(4,4,64,64, Qt::black);
    painter.drawImage(4, 4, texture.copy(Skins::head.front.x, Skins::head.front.y, Skins::head.front.w, Skins::head.front.h).scaled(64, 64));
    painter.drawImage(0, 0, texture.copy(Skins::head_cover.front.x, Skins::head_cover.front.y, Skins::head_cover.front.w, Skins::head_cover.front.h).scaled(72, 72));
    return head;
}

shared_qobject_ptr<AccountTask> MinecraftAccount::loginMSA() {
    Q_ASSERT(m_currentTask.get() == nullptr);

    m_currentTask.reset(new MSAInteractive(&data));
    connect(m_currentTask.get(), SIGNAL(succeeded()), SLOT(authSucceeded()));
    connect(m_currentTask.get(), SIGNAL(failed(QString)), SLOT(authFailed(QString)));
    emit activityChanged(true);
    return m_currentTask;
}

shared_qobject_ptr<AccountTask> MinecraftAccount::refresh() {
    if(m_currentTask) {
        return m_currentTask;
    }

    m_currentTask.reset(new MSASilent(&data));

    connect(m_currentTask.get(), SIGNAL(succeeded()), SLOT(authSucceeded()));
    connect(m_currentTask.get(), SIGNAL(failed(QString)), SLOT(authFailed(QString)));
    emit activityChanged(true);
    return m_currentTask;
}

shared_qobject_ptr<AccountTask> MinecraftAccount::createMinecraftProfile(const QString& profileName) {
    if(m_currentTask) {
        return nullptr;
    }

    m_currentTask.reset(new MSACreateProfile(&data, profileName));

    connect(m_currentTask.get(), SIGNAL(succeeded()), SLOT(authSucceeded()));
    connect(m_currentTask.get(), SIGNAL(failed(QString)), SLOT(authFailed(QString)));
    emit activityChanged(true);
    return m_currentTask;
}

shared_qobject_ptr<AccountTask> MinecraftAccount::setSkin(Skins::Model model, QByteArray texture, const QString& capeUUID) {
    if(m_currentTask) {
        return nullptr;
    }

    m_currentTask.reset(new MSASetSkin(&data, texture, model, capeUUID));

    connect(m_currentTask.get(), SIGNAL(succeeded()), SLOT(authSucceeded()));
    connect(m_currentTask.get(), SIGNAL(failed(QString)), SLOT(authFailed(QString)));
    emit activityChanged(true);
    return m_currentTask;
}


shared_qobject_ptr<AccountTask> MinecraftAccount::currentTask() {
    return m_currentTask;
}


void MinecraftAccount::authSucceeded()
{
    m_currentTask.reset();
    updateCapeCache();
    emit changed();
    emit activityChanged(false);
}

void MinecraftAccount::authFailed(QString reason)
{
    switch (m_currentTask->taskState()) {
        case AccountTaskState::STATE_OFFLINE:
        case AccountTaskState::STATE_FAILED_MUST_MIGRATE:
        case AccountTaskState::STATE_FAILED_SOFT: {
            // NOTE: this doesn't do much. There was an error of some sort.
        }
        break;
        case AccountTaskState::STATE_FAILED_HARD: {
            data.msaToken.token = QString();
            data.msaToken.refresh_token = QString();
            data.msaToken.validity = Katabasis::Validity::None;
            data.validity_ = Katabasis::Validity::None;
            emit changed();
        }
        break;
        case AccountTaskState::STATE_FAILED_GONE: {
            data.validity_ = Katabasis::Validity::None;
            emit changed();
        }
        break;
        case AccountTaskState::STATE_CREATED:
        case AccountTaskState::STATE_WORKING:
        case AccountTaskState::STATE_SUCCEEDED: {
            // Not reachable here, as they are not failures.
        }
    }
    m_currentTask.reset();
    emit activityChanged(false);
}

bool MinecraftAccount::isActive() const {
    return m_currentTask;
}

bool MinecraftAccount::shouldRefresh() const {
    /*
     * Never refresh accounts that are being used by the game, it breaks the game session.
     * Always refresh accounts that have not been refreshed yet during this session.
     * Don't refresh broken accounts.
     * Refresh accounts that would expire in the next 12 hours (fresh token validity is 24 hours).
     */
    if(isInUse()) {
        return false;
    }
    switch(data.validity_) {
        case Katabasis::Validity::Certain: {
            break;
        }
        case Katabasis::Validity::None: {
            return false;
        }
        case Katabasis::Validity::Assumed: {
            return true;
        }
    }
    auto now = QDateTime::currentDateTimeUtc();
    auto issuedTimestamp = data.yggdrasilToken.issueInstant;
    auto expiresTimestamp = data.yggdrasilToken.notAfter;

    if(!expiresTimestamp.isValid()) {
        expiresTimestamp = issuedTimestamp.addSecs(24 * 3600);
    }
    if (now.secsTo(expiresTimestamp) < (12 * 3600)) {
        return true;
    }
    return false;
}

void MinecraftAccount::fillSession(AuthSessionPtr session)
{
    if(ownsMinecraft() && !hasProfile()) {
        session->status = AuthSession::RequiresProfileSetup;
    }
    else {
        if(session->wants_online) {
            session->status = AuthSession::PlayableOnline;
        }
        else {
            session->status = AuthSession::PlayableOffline;
        }
    }

    // NOTE: removed because of MSA
    session->username = "";
    // volatile auth token
    session->access_token = data.accessToken();
    // NOTE: removed because of MSA
    session->client_token = "";
    // profile name
    session->player_name = data.profileName();
    // profile ID
    session->uuid = data.profileId();
    // 'legacy' or 'mojang', depending on account type
    session->user_type = typeString();
    if (!session->access_token.isEmpty())
    {
        session->session = "token:" + data.accessToken() + ":" + data.profileId();
    }
    else
    {
        session->session = "-";
    }
}

void MinecraftAccount::decrementUses()
{
    Usable::decrementUses();
    if(!isInUse())
    {
        emit changed();
        // FIXME: we now need a better way to identify accounts...
        qWarning() << "Profile" << data.profileId() << "is no longer in use.";
    }
}

void MinecraftAccount::incrementUses()
{
    bool wasInUse = isInUse();
    Usable::incrementUses();
    if(!wasInUse)
    {
        emit changed();
        // FIXME: we now need a better way to identify accounts...
        qWarning() << "Profile" << data.profileId() << "is now in use.";
    }
}

void MinecraftAccount::replaceDataWith(MinecraftAccountPtr other)
{
    data = other->data;
    emit changed();
}
