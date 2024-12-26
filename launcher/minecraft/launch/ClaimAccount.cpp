#include "ClaimAccount.h"
#include <launch/LaunchTask.h>

#include "Application.h"
#include "minecraft/auth/AccountList.h"

ClaimAccount::ClaimAccount(LaunchTask* parent, AuthSessionPtr session): LaunchStep(parent)
{
    m_playerName = session->player_name;
    if(session->status == AuthSession::Status::PlayableOnline && !session->demo)
    {
        online = true;
        auto accounts = APPLICATION->accounts();
        int index;
        accounts->getAccountByProfileName(m_playerName, m_account, index);
    }
}

void ClaimAccount::executeTask()
{
    if(online)
    {
        if(m_account)
        {
            m_lock.reset(new UseLock(m_account));
            emitSucceeded();
        }
        else
        {
            emitFailed(tr("Failed to claim account by profile: %1 was not found.").arg(m_playerName));
        }
    }
    else
    {
        emitSucceeded();
    }
}

void ClaimAccount::finalize()
{
    m_lock.reset();
}
