/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#pragma once

#include <QDialog>
#include "minecraft/auth/AccountList.h"
#include "minecraft/auth/AccountTask.h"

#include <QTimer>
#include <QUrl>
#include <QString>
#include <QDialog>
#include <QNetworkReply>
#include <QIcon>

#include <nonstd/optional>

class QAbstractButton;
class QMenu;
class QEvent;
class QAction;

namespace Ui
{
class AccountsDialog;
}

struct SkinState
{
    QString cape;
    Skins::Model model;
    Skins::SkinEntry skinEntry;
};

class AccountsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AccountsDialog(QWidget *parent = 0, const QString& internalId = QString());
    virtual ~AccountsDialog();

    enum class NameStatus
    {
        NotSet,
        Pending,
        Available,
        Exists,
        Error
    } nameStatus = NameStatus::NotSet;

protected:
    void closeEvent(QCloseEvent *event) override;

    // Skins stuff
private slots:
    void onSkinSelectionChanged(const class QItemSelection &selected, const class QItemSelection &deselected);
    void onCapeSelectionChanged(const class QItemSelection &selected, const class QItemSelection &deselected);
    void onModelRadioClicked(QAbstractButton* radio);
    void onRevertChangesClicked(bool);
    void onApplyChangesClicked(bool);
    void onSaveSkinClicked(bool);
    void onOpenSkinsFolderClicked(bool);

    void onSkinUpdated(const QString& key);
    void onSkinModelUpdated();
    void onCapeUpdated(const QString& uuid);

private:
    bool startSkinEdit();
    bool hasEdits() const
    {
        return m_skinEdit != nonstd::nullopt;
    }
    void revertEdits();

    void editCape(const QString& cape);
    const QString& effectiveCape() const;

    void editModel(Skins::Model model);
    Skins::Model effectiveModel() const;

    void editSkin(const Skins::SkinEntry& skin);
    const Skins::SkinEntry& effectiveSkin() const;

    void updateModelToMatchSkin();
    void updateSkinDisplay();

private:
    SkinState m_playerSkinState;
    nonstd::optional<SkinState> m_skinEdit;

private slots:
    // Account display page
    void onRefreshButtonClicked(bool);
    void onSignOutButtonClicked(bool);
    void onRemoveAndSignInButtonClicked(bool);
    void onGetMinecraftButtonClicked(bool);

    void onAccountChanged(MinecraftAccount * account);
    void onAccountActivityChanged(MinecraftAccount * account, bool active);

private:
    void updateStates();

    // Login page
private:
    void stopLogin();
    void startLogin();

private slots:
    void onGetFreshCodeButtonClicked(bool);
    void onQrButtonClicked(bool);
    void onCopyLinkButtonClicked(bool);
    void onLoginTaskFailed(const QString &reason);
    void onLoginTaskSucceeded();
    void onLoginTaskStatus(const QString &status);
    void onLoginTaskProgress(qint64 current, qint64 total);
    void showVerificationUriAndCode(const QUrl &uri, const QString &code, int expiresIn);
    void hideVerificationUriAndCode();
    void externalLoginTick();

// Profile setup stuff
private slots:
    void onCreateProfileButtonClicked(bool);

    void onNameEdited(const QString &name);
    void onNameCheckFinished(
        QNetworkReply::NetworkError error,
        QByteArray data,
        QList<QNetworkReply::RawHeaderPair> headers
    );
    void onNameCheckTimerTriggered();
    void onProfileCreationError(const MojangError& error);

private:
    void scheduleCheck(const QString &name);
    void checkName(const QString &name);
    void setNameStatus(NameStatus status, QString errorString);

private:
    QIcon m_goodIcon;
    QIcon m_yellowIcon;
    QIcon m_badIcon;
    QAction * m_validityAction = nullptr;

    QString m_queuedCheck;
    bool m_isChecking = false;
    QString m_currentCheck;
    QTimer m_checkStartTimer;

// Other
private:
    void changeEvent(QEvent * event) override;

private:
    Ui::AccountsDialog *ui;
    class QStatusBar* m_statusBar = nullptr;
    shared_qobject_ptr<AccountList> m_accounts;

    MinecraftAccountPtr m_currentAccount;
    class CapesModel* m_capesModel = nullptr;
    class SkinsModel* m_skinsModel = nullptr;

    MinecraftAccountPtr m_loginAccount;
    shared_qobject_ptr<AccountTask> m_loginTask;
    QTimer m_externalLoginTimer;
    QString m_code;
    QUrl m_codeUrl;
    int m_externalLoginElapsed = 0;
    int m_externalLoginTimeout = 0;
};
