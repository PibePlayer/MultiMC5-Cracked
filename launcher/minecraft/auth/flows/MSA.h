#pragma once
#include "AuthFlow.h"
#include "skins/SkinTypes.h"

class MSAInteractive : public AuthFlow
{
    Q_OBJECT
public:
    explicit MSAInteractive(
        AccountData *data,
        QObject *parent = 0
    );
};

class MSASilent : public AuthFlow
{
    Q_OBJECT
public:
    explicit MSASilent(
        AccountData * data,
        QObject *parent = 0
    );
};

class MSACreateProfile : public AuthFlow
{
    Q_OBJECT
public:
    explicit MSACreateProfile(
        AccountData * data,
        const QString& profileName,
        QObject *parent = 0
    );
};

class MSASetSkin : public AuthFlow
{
    Q_OBJECT
public:
    explicit MSASetSkin(
        AccountData * data,
        const QByteArray& skinData,
        Skins::Model model,
        const QString& uuid,
        QObject *parent = 0
    );
};
