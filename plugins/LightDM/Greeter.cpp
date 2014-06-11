/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Michael Terry <michael.terry@canonical.com>
 */

#include "Greeter.h"
#include <QLightDM/Greeter>

class GreeterPrivate
{
public:
    explicit GreeterPrivate(Greeter *parent);

    QLightDM::Greeter *m_greeter;
    bool wasPrompted;
    bool promptless;

protected:
    Greeter * const q_ptr;

private:
    Q_DECLARE_PUBLIC(Greeter)
};


GreeterPrivate::GreeterPrivate(Greeter* parent)
  : m_greeter(new QLightDM::Greeter(parent)),
    wasPrompted(false),
    promptless(false),
    q_ptr(parent)
{
}

Greeter::Greeter(QObject* parent)
  : QObject(parent),
    d_ptr(new GreeterPrivate(this))
{
    Q_D(Greeter);

    connect(d->m_greeter, SIGNAL(showMessage(QString, QLightDM::Greeter::MessageType)),
            this, SLOT(showMessageFilter(QString, QLightDM::Greeter::MessageType)));
    connect(d->m_greeter, SIGNAL(showPrompt(QString, QLightDM::Greeter::PromptType)),
            this, SLOT(showPromptFilter(QString, QLightDM::Greeter::PromptType)));
    connect(d->m_greeter, SIGNAL(authenticationComplete()),
            this, SLOT(authenticationCompleteFilter()));

    d->m_greeter->connectSync();
}

bool Greeter::isAuthenticated() const
{
    Q_D(const Greeter);
    return d->m_greeter->isAuthenticated();
}

QString Greeter::authenticationUser() const
{
    Q_D(const Greeter);
    return d->m_greeter->authenticationUser();
}

bool Greeter::promptless() const
{
    Q_D(const Greeter);
    return d->promptless;
}

void Greeter::authenticate(const QString &username)
{
    Q_D(Greeter);
    d->wasPrompted = false;
    if (d->promptless) {
        d->promptless = false;
        Q_EMIT promptlessChanged();
    }

    d->m_greeter->authenticate(username);
    Q_EMIT authenticationUserChanged(username);
}

void Greeter::respond(const QString &response)
{
    Q_D(Greeter);
    d->m_greeter->respond(response);
}

bool Greeter::startSessionSync(const QString &session)
{
    Q_D(Greeter);
    return d->m_greeter->startSessionSync(session);
}

void Greeter::showPromptFilter(const QString &text, QLightDM::Greeter::PromptType type)
{
    Q_D(Greeter);
    d->wasPrompted = true;

    // Strip prompt of any colons at the end
    QString trimmedText = text.trimmed();
    if (trimmedText.endsWith(":") || trimmedText.endsWith("：")) {
        trimmedText.chop(1);
    }

    Q_EMIT showPrompt(trimmedText, type == QLightDM::Greeter::PromptTypeSecret);
}

void Greeter::showMessageFilter(const QString &text, QLightDM::Greeter::MessageType type)
{
    Q_EMIT showMessage(text, type == QLightDM::Greeter::MessageTypeError);
}

void Greeter::authenticationCompleteFilter()
{
    Q_D(Greeter);
    if (!d->wasPrompted) {
        d->promptless = true;
        Q_EMIT promptlessChanged();
    }

    Q_EMIT authenticationComplete();
}
