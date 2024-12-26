#include "MSA.h"

#include "minecraft/auth/steps/MSAStep.h"
#include "minecraft/auth/steps/XboxUserStep.h"
#include "minecraft/auth/steps/XboxAuthorizationStep.h"
#include "minecraft/auth/steps/LauncherLoginStep.h"
#include "minecraft/auth/steps/XboxProfileStep.h"
#include "minecraft/auth/steps/EntitlementsStep.h"
#include "minecraft/auth/steps/MinecraftProfileStep.h"
#include "minecraft/auth/steps/MinecraftProfileCreateStep.h"
#include "minecraft/auth/steps/GetSkinStep.h"
#include "minecraft/auth/steps/SetSkinStep.h"
#include "minecraft/auth/steps/SetCapeStep.h"

MSASilent::MSASilent(AccountData* data, QObject* parent) : AuthFlow(data, parent) {
    m_steps.append(new MSAStep(m_data, MSAStep::Action::Refresh));
    m_steps.append(new XboxUserStep(m_data));
    m_steps.append(new XboxAuthorizationStep(m_data, &m_data->xboxApiToken, "http://xboxlive.com", "Xbox"));
    m_steps.append(new XboxAuthorizationStep(m_data, &m_data->mojangservicesToken, "rp://api.minecraftservices.com/", "Mojang"));
    m_steps.append(new LauncherLoginStep(m_data));
    m_steps.append(new XboxProfileStep(m_data));
    m_steps.append(new EntitlementsStep(m_data));
    m_steps.append(new MinecraftProfileStep(m_data));
    m_steps.append(new GetSkinStep(m_data));
}

MSAInteractive::MSAInteractive(
    AccountData* data,
    QObject* parent
) : AuthFlow(data, parent) {
    m_steps.append(new MSAStep(m_data, MSAStep::Action::Login));
    m_steps.append(new XboxUserStep(m_data));
    m_steps.append(new XboxAuthorizationStep(m_data, &m_data->xboxApiToken, "http://xboxlive.com", "Xbox"));
    m_steps.append(new XboxAuthorizationStep(m_data, &m_data->mojangservicesToken, "rp://api.minecraftservices.com/", "Mojang"));
    m_steps.append(new LauncherLoginStep(m_data));
    m_steps.append(new XboxProfileStep(m_data));
    m_steps.append(new EntitlementsStep(m_data));
    m_steps.append(new MinecraftProfileStep(m_data));
    m_steps.append(new GetSkinStep(m_data));
}

MSACreateProfile::MSACreateProfile(AccountData* data, const QString& profileName, QObject* parent) : AuthFlow(data, parent) {
    m_steps.append(new MSAStep(m_data, MSAStep::Action::Refresh));
    m_steps.append(new XboxUserStep(m_data));
    m_steps.append(new XboxAuthorizationStep(m_data, &m_data->xboxApiToken, "http://xboxlive.com", "Xbox"));
    m_steps.append(new XboxAuthorizationStep(m_data, &m_data->mojangservicesToken, "rp://api.minecraftservices.com/", "Mojang"));
    m_steps.append(new LauncherLoginStep(m_data));
    m_steps.append(new XboxProfileStep(m_data));
    m_steps.append(new EntitlementsStep(m_data));
    auto apiCall = new MinecraftProfileCreateStep(m_data, profileName);
    m_steps.append(apiCall);
    connect(apiCall, &MinecraftProfileCreateStep::apiError, this, &AccountTask::apiError);
    m_steps.append(new MinecraftProfileStep(m_data));
    m_steps.append(new GetSkinStep(m_data));
}

MSASetSkin::MSASetSkin(AccountData* data, const QByteArray& skinData, Skins::Model model, const QString& uuid, QObject* parent) : AuthFlow(data, parent) {
    m_steps.append(new MSAStep(m_data, MSAStep::Action::Refresh));
    m_steps.append(new XboxUserStep(m_data));
    m_steps.append(new XboxAuthorizationStep(m_data, &m_data->xboxApiToken, "http://xboxlive.com", "Xbox"));
    m_steps.append(new XboxAuthorizationStep(m_data, &m_data->mojangservicesToken, "rp://api.minecraftservices.com/", "Mojang"));
    m_steps.append(new LauncherLoginStep(m_data));
    m_steps.append(new XboxProfileStep(m_data));
    m_steps.append(new EntitlementsStep(m_data));

    {
        auto apiCall = new SetSkinStep(m_data, model, skinData);
        m_steps.append(apiCall);
        connect(apiCall, &SetSkinStep::apiError, this, &AccountTask::apiError);
    }
    if(uuid != data->minecraftProfile.currentCape)
    {
        auto apiCall = new SetCapeStep(m_data, uuid);
        m_steps.append(apiCall);
        connect(apiCall, &SetCapeStep::apiError, this, &AccountTask::apiError);
    }
    m_steps.append(new MinecraftProfileStep(m_data));
    m_steps.append(new GetSkinStep(m_data));
}
