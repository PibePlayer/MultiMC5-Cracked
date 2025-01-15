/* Copyright 2013-2024 MultiMC Contributors
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

#include "AccountList.h"
#include "AccountData.h"
#include "AccountTask.h"

#include <QIODevice>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDir>
#include <QTimer>
#include <QIcon>

#include <QDebug>

#include <Application.h>
#include <FileSystem.h>
#include <QSaveFile>

#include <chrono>
#include <Application.h>

enum AccountListVersion {
    MojangOnly = 2,
    MojangMSA = 3
};

AccountList::AccountList(QObject *parent) : QAbstractListModel(parent) {
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setSingleShot(true);
    connect(m_refreshTimer, &QTimer::timeout, this, &AccountList::fillQueue);
    m_nextTimer = new QTimer(this);
    m_nextTimer->setSingleShot(true);
    connect(m_nextTimer, &QTimer::timeout, this, &AccountList::tryNext);
}

AccountList::~AccountList() noexcept {}

bool AccountList::getAccountByXID(const QString& xid, MinecraftAccountPtr& pointer, int& index) const {
    for (int i = 0; i < count(); i++) {
        auto entry = at(i);
        if(!entry.isAccount)
        {
            continue;
        }
        if (entry.account->xid() == xid) {
            pointer = entry.account;
            index = i;
            return true;
        }
    }
    pointer = nullptr;
    index = -1;
    return false;
}

bool AccountList::getAccountByProfileName(const QString& profileName, MinecraftAccountPtr& pointer, int& index) const {
    for (int i = 0; i < count(); i++) {
        auto entry = at(i);
        if(!entry.isAccount)
        {
            continue;
        }
        if (entry.account->profileName() == profileName) {
            pointer = entry.account;
            index = i;
            return true;
        }
    }
    pointer = nullptr;
    index = -1;
    return false;
}

bool AccountList::getAccountById(const QString& internalId, MinecraftAccountPtr& pointer, int& index) const {
    for (int i = 0; i < count(); i++) {
        auto entry = at(i);
        if(!entry.isAccount)
        {
            continue;
        }
        if (entry.account->internalId() == internalId) {
            pointer = entry.account;
            index = i;
            return true;
        }
    }
    pointer = nullptr;
    index = -1;
    return false;
}

const AccountList::Entry& AccountList::at(int i) const
{
    return m_accounts.at(i);
}

QStringList AccountList::profileNames() const {
    QStringList out;
    for(auto & entry: m_accounts) {
        if(!entry.isAccount)
        {
            continue;
        }
        auto profileName =  entry.account->profileName();
        if(profileName.isEmpty())
        {
            continue;
        }
        out.append(profileName);
    }
    return out;
}

QModelIndex AccountList::addAccount(const MinecraftAccountPtr account)
{
    // NOTE: Do not allow adding something that's already there
    int i = 0;
    for(const auto& entry: m_accounts)
    {
        if(entry.account == account)
        {
            return index(i);
        }
    }

    // override/replace existing account with the same XUID
    auto xid = account->xid();
    MinecraftAccountPtr existingAccountPtr;
    int existingIndex;
    if(getAccountByXID(xid, existingAccountPtr, existingIndex)) {
        existingAccountPtr->replaceDataWith(account);
        auto modelIndex = index(existingIndex);
        emit dataChanged(modelIndex, modelIndex);
        onListChanged();
        return modelIndex;
    }

    // hook up notifications for changes in the account
    connect(account.get(), &MinecraftAccount::changed, this, &AccountList::onAccountChanged);
    connect(account.get(), &MinecraftAccount::activityChanged, this, &AccountList::onAccountActivityChanged);

    // if we don't have this profileId yet, add the account to the end
    int row = m_accounts.count();
    beginInsertRows(QModelIndex(), row, row);
    m_accounts.append(Entry{true, account});
    endInsertRows();
    onListChanged();
    return index(row);
}

void AccountList::removeAccount(const QString& internalId)
{
    int row;
    MinecraftAccountPtr account;
    if(!getAccountById(internalId, account, row))
    {
        return;
    }

    if(account == m_defaultAccount)
    {
        m_defaultAccount = nullptr;
        onDefaultAccountChanged();
    }
    account->disconnect(this);

    beginRemoveRows(QModelIndex(), row, row);
    m_accounts.removeAt(row);
    endRemoveRows();
    onListChanged();
}

QModelIndex AccountList::defaultAccountIndex() const
{
    if(!m_defaultAccount)
    {
        return QModelIndex();
    }

    for (int i = 0; i < count(); i++) {
        auto entry = at(i);
        if(!entry.isAccount)
        {
            continue;
        }
        if (entry.account == m_defaultAccount) {
            return index(i);
        }
    }
    return QModelIndex();
}


MinecraftAccountPtr AccountList::defaultAccount() const
{
    return m_defaultAccount;
}

void AccountList::setDefaultAccount(MinecraftAccountPtr newAccount)
{
    if (!newAccount && m_defaultAccount)
    {
        int idx = 0;
        auto previousDefaultAccount = m_defaultAccount;
        m_defaultAccount = nullptr;
        for (auto& entry : m_accounts)
        {
            if(entry.isAccount && entry.account == previousDefaultAccount)
            {
                emit dataChanged(index(idx), index(idx));
            }
            idx ++;
        }
        onDefaultAccountChanged();
    }
    else
    {
        auto currentDefaultAccount = m_defaultAccount;
        int currentDefaultAccountIdx = -1;
        auto newDefaultAccount = m_defaultAccount;
        int newDefaultAccountIdx = -1;
        int idx = 0;
        for (auto& entry : m_accounts)
        {
            if(!entry.isAccount)
            {
                continue;
            }
            if (entry.account == newAccount)
            {
                newDefaultAccount = entry.account;
                newDefaultAccountIdx = idx;
            }
            if(currentDefaultAccount == entry.account)
            {
                currentDefaultAccountIdx = idx;
            }
            idx++;
        }
        if(currentDefaultAccount != newDefaultAccount)
        {
            emit dataChanged(index(currentDefaultAccountIdx), index(currentDefaultAccountIdx));
            emit dataChanged(index(newDefaultAccountIdx), index(newDefaultAccountIdx));
            m_defaultAccount = newDefaultAccount;
            onDefaultAccountChanged();
        }
    }
}

void AccountList::onAccountChanged()
{
    // TODO: factor out
    MinecraftAccount *account = qobject_cast<MinecraftAccount *>(sender());
    bool found = false;
    for (int i = 0; i < count(); i++) {
        auto entry = at(i);
        if(!entry.isAccount)
            continue;
        if (entry.account.get() == account) {
            emit dataChanged(index(i),  index(i));
            found = true;
            break;
        }
    }
    if(found)
    {
        emit accountChanged(account);
        // the list changed. there is no doubt.
        onListChanged();
    }
}

void AccountList::onAccountActivityChanged(bool active)
{
    // TODO: factor out
    MinecraftAccount *account = qobject_cast<MinecraftAccount *>(sender());
    bool found = false;
    for (int i = 0; i < count(); i++) {
        auto entry = at(i);
        if(!entry.isAccount)
            continue;
        if (entry.account.get() == account) {
            emit dataChanged(index(i),  index(i));
            found = true;
            break;
        }
    }
    if(found) {
        emit accountActivityChanged(account, active);
        if(active) {
            beginActivity();
        }
        else {
            endActivity();
        }
    }
}


void AccountList::onListChanged()
{
    if (m_autosave)
        // TODO: Alert the user if this fails.
        saveList();

    emit listChanged();
}

void AccountList::onDefaultAccountChanged()
{
    if (m_autosave)
        saveList();

    emit defaultAccountChanged();
}

int AccountList::count() const
{
    return m_accounts.count();
}

QVariant AccountList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() > count())
        return QVariant();

    auto entry = at(index.row());
    if(!entry.isAccount)
    {
        switch (role)
        {
            case Qt::DisplayRole:
                return tr("Add New Account");
            case Qt::DecorationRole:
                return APPLICATION->getThemedIcon("accounts");
            case PointerRole:
                return QVariant::fromValue(MinecraftAccountPtr());
        }
        return QVariant();
    }
    auto account = entry.account;

    switch (role)
    {
        case Qt::DisplayRole:
        {
            return account->profileName() + "\n" + account->gamerTag() + "\n" + account->accountStateText();
        }
        case AccountNameRole:
            return account->gamerTag();
        case ProfileNameRole:
            return account->profileName();
        case AccountStatusRole:
            return account->accountStateText();

        case IconRole:
        case Qt::DecorationRole:
        {
            QPixmap face = account->getFace();
            if(face.isNull())
            {
                return APPLICATION->getThemedIcon("noaccount");
            }
            return QIcon(face);
        }


        case PointerRole:
            return QVariant::fromValue(account);

        case Qt::CheckStateRole:
            return account == m_defaultAccount ? Qt::Checked : Qt::Unchecked;

        default:
            return QVariant();
    }
}



int AccountList::rowCount(const QModelIndex &) const
{
    // Return count
    return count();
}

Qt::ItemFlags AccountList::flags(const QModelIndex &index) const
{
    if (index.row() < 0 || index.row() >= rowCount(index) || !index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool AccountList::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (idx.row() < 0 || idx.row() >= rowCount(idx) || !idx.isValid())
    {
        return false;
    }
    auto entry = at(idx.row());
    if(!entry.isAccount)
    {
        return false;
    }

    if(role == Qt::CheckStateRole)
    {
        if(value == Qt::Checked)
        {
            setDefaultAccount(entry.account);
        }
        else if(value == Qt::Unchecked)
        {
            setDefaultAccount(nullptr);
        }
    }

    emit dataChanged(idx, idx);
    return true;
}

bool AccountList::loadList()
{
    if (m_listFilePath.isEmpty())
    {
        qCritical() << "Can't load account list. No file path given and no default set.";
        return false;
    }

    m_accounts.append(Entry{false, nullptr});

    QFile file(m_listFilePath);

    // Try to open the file and fail if we can't.
    // TODO: We should probably report this error to the user.
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << QString("Failed to read the account list file (%1).").arg(m_listFilePath).toUtf8();
        return false;
    }

    // Read the file and close it.
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    // Fail if the JSON is invalid.
    if (parseError.error != QJsonParseError::NoError)
    {
        qCritical() << QString("Failed to parse account list file: %1 at offset %2")
                            .arg(parseError.errorString(), QString::number(parseError.offset))
                            .toUtf8();
        return false;
    }

    // Make sure the root is an object.
    if (!jsonDoc.isObject())
    {
        qCritical() << "Invalid account list JSON: Root should be an array.";
        return false;
    }

    QJsonObject root = jsonDoc.object();

    // Make sure the format version matches.
    auto listVersion = root.value("formatVersion").toVariant().toInt();
    switch(listVersion) {
        case AccountListVersion::MojangMSA: {
            return loadV3(root);
        }
        break;
        default: {
            QString newName = "accounts-old.json";
            qWarning() << "Unknown format version when loading account list. Existing one will be renamed to" << newName;
            // Attempt to rename the old version.
            file.rename(newName);
            return false;
        }
    }
}

bool AccountList::loadV3(QJsonObject& root) {
    beginResetModel();
    QJsonArray accounts = root.value("accounts").toArray();
    for (QJsonValue accountVal : accounts)
    {
        QJsonObject accountObj = accountVal.toObject();
        MinecraftAccountPtr account = MinecraftAccount::loadFromJsonV3(accountObj);
        if (account.get() != nullptr)
        {
            // Note: protection against duplication in the file
            auto xid = account->xid();
            if(xid.size()) {
                MinecraftAccountPtr dummy;
                int dummyRow;
                if(getAccountByXID(xid, dummy, dummyRow)) {
                    continue;
                }
            }
            connect(account.get(), &MinecraftAccount::changed, this, &AccountList::onAccountChanged);
            connect(account.get(), &MinecraftAccount::activityChanged, this, &AccountList::onAccountActivityChanged);
            account->updateCapeCache();
            m_accounts.append(Entry{true, account});
            if(accountObj.value("active").toBool(false)) {
                m_defaultAccount = account;
            }
        }
        else
        {
            qWarning() << "Failed to load an account.";
        }
    }
    endResetModel();
    return true;
}


bool AccountList::saveList()
{
    if (m_listFilePath.isEmpty())
    {
        qCritical() << "Can't save account list. No file path given and no default set.";
        return false;
    }

    // make sure the parent folder exists
    if(!FS::ensureFilePathExists(m_listFilePath))
        return false;

    // make sure the file wasn't overwritten with a folder before (fixes a bug)
    QFileInfo finfo(m_listFilePath);
    if(finfo.isDir())
    {
        QDir badDir(m_listFilePath);
        badDir.removeRecursively();
    }

    qDebug() << "Writing account list to" << m_listFilePath;

    qDebug() << "Building JSON data structure.";
    // Build the JSON document to write to the list file.
    QJsonObject root;

    root.insert("formatVersion", AccountListVersion::MojangMSA);

    // Build a list of accounts.
    qDebug() << "Building account array.";
    QJsonArray accounts;
    for (auto& entry : m_accounts)
    {
        if(!entry.isAccount)
        {
            continue;
        }
        QJsonObject accountObj = entry.account->saveToJson();
        if(m_defaultAccount == entry.account) {
            accountObj["active"] = true;
        }
        accounts.append(accountObj);
    }

    // Insert the account list into the root object.
    root.insert("accounts", accounts);

    // Create a JSON document object to convert our JSON to bytes.
    QJsonDocument doc(root);

    // Now that we're done building the JSON object, we can write it to the file.
    qDebug() << "Writing account list to file.";
    QSaveFile file(m_listFilePath);

    // Try to open the file and fail if we can't.
    // TODO: We should probably report this error to the user.
    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical() << QString("Failed to read the account list file (%1).").arg(m_listFilePath).toUtf8();
        return false;
    }

    // Write the JSON to the file.
    file.write(doc.toJson());
    file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadUser|QFile::WriteUser);
    if(file.commit()) {
        qDebug() << "Saved account list to" << m_listFilePath;
        return true;
    }
    else {
        qDebug() << "Failed to save accounts to" << m_listFilePath;
        return false;
    }
}

void AccountList::setListFilePath(QString path, bool autosave)
{
    m_listFilePath = path;
    m_autosave = autosave;
}

bool AccountList::anyAccountIsValid()
{
    for(auto& entry: m_accounts)
    {
        if(entry.account && entry.account->ownsMinecraft()) {
            return true;
        }
    }
    return false;
}

void AccountList::fillQueue() {

    if(m_defaultAccount && m_defaultAccount->shouldRefresh()) {
        auto idToRefresh = m_defaultAccount->internalId();
        m_refreshQueue.push_back(idToRefresh);
        qDebug() << "AccountList: Queued default account with internal ID " << idToRefresh << " to refresh first";
    }

    for(int i = 0; i < count(); i++) {
        auto entry = at(i);
        if(!entry.isAccount)
        {
            continue;
        }
        if(entry.account == m_defaultAccount)
        {
            continue;
        }

        if(entry.account->shouldRefresh()) {
            auto idToRefresh = entry.account->internalId();
            queueRefresh(idToRefresh);
        }
    }
    tryNext();
}

void AccountList::requestRefresh(QString accountId) {
    auto index = m_refreshQueue.indexOf(accountId);
    if(index != -1) {
        m_refreshQueue.removeAt(index);
    }
    m_refreshQueue.push_front(accountId);
    qDebug() << "AccountList: Pushed account with internal ID " << accountId << " to the front of the queue";
    if(!isActive()) {
        tryNext();
    }
}

void AccountList::queueRefresh(QString accountId) {
    if(m_refreshQueue.indexOf(accountId) != -1) {
        return;
    }
    m_refreshQueue.push_back(accountId);
    qDebug() << "AccountList: Queued account with internal ID " << accountId << " to refresh";
}


void AccountList::tryNext() {
    while (m_refreshQueue.length()) {
        auto accountId = m_refreshQueue.front();
        m_refreshQueue.pop_front();
        for(int i = 0; i < count(); i++) {
            auto entry = at(i);
            if(!entry.isAccount)
            {
                continue;
            }
            if(entry.account->internalId() == accountId) {
                m_currentTask = entry.account->refresh();
                if(m_currentTask) {
                    connect(m_currentTask.get(), &AccountTask::succeeded, this, &AccountList::authSucceeded);
                    connect(m_currentTask.get(), &AccountTask::failed, this, &AccountList::authFailed);
                    m_currentTask->start();
                    qDebug() << "RefreshSchedule: Processing account " << entry.account->gamerTag() << " with internal ID " << accountId;
                    return;
                }
            }
        }
        qDebug() << "RefreshSchedule: Account with with internal ID " << accountId << " not found.";
    }
    // if we get here, no account needed refreshing. Schedule refresh in an hour.
    m_refreshTimer->start(1000 * 3600);
}

void AccountList::authSucceeded() {
    qDebug() << "RefreshSchedule: Background account refresh succeeded";
    m_currentTask.reset();
    m_nextTimer->start(1000 * 20);
}

void AccountList::authFailed(QString reason) {
    qDebug() << "RefreshSchedule: Background account refresh failed: " << reason;
    m_currentTask.reset();
    m_nextTimer->start(1000 * 20);
}

bool AccountList::isActive() const {
    return m_activityCount != 0;
}

void AccountList::beginActivity() {
    m_activityCount++;
}

void AccountList::endActivity() {
    if(m_activityCount == 0) {
        qWarning() << m_name << " - Activity count would become below zero";
        return;
    }
    m_activityCount--;
}
