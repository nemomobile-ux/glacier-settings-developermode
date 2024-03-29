/*
 * Copyright (C) 2022 Chupligin Sergey <neochapay@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "systemdconnector.h"

#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDebug>

SystemDConnector::SystemDConnector(QObject* parent)
    : QObject { parent }
    , m_systemDBusConnection(QDBusConnection::systemBus())
    , m_sshdActive(false)
{
    if (!m_systemDBusConnection.isConnected()) {
        qFatal("Can't connect to system bus");
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(
        s_serviceName,
        s_servicePath,
        s_propertiesIface,
        "Get");

    msg.setArguments({ s_unitIface, s_propertyActiveState });

    QDBusPendingReply<QVariant> msgPending = QDBusConnection::systemBus().asyncCall(msg);
    QDBusPendingCallWatcher* msgWatcher = new QDBusPendingCallWatcher(msgPending);

    connect(msgWatcher, &QDBusPendingCallWatcher::finished, this, &SystemDConnector::startSShdState);

    m_systemDBusConnection.connect(
        QString(),
        s_servicePath,
        s_propertiesIface,
        "PropertiesChanged",
        this,
        SLOT(propertiesChanged(QString, QVariantMap, QStringList)));
}

void SystemDConnector::enableDeveloperMode(bool enable)
{
    QString action = "Stop";
    if (enable) {
        action = "Start";
    }
    QDBusMessage msg = QDBusMessage::createMethodCall(
        s_serviceName,
        s_servicePath,
        s_unitIface,
        action);

    qDebug() << Q_FUNC_INFO << action;

    msg.setArguments({ "replace" });
    QDBusConnection::systemBus().call(msg);
}

void SystemDConnector::propertiesChanged(const QString& data, const QVariantMap& properties, const QStringList& invalidatedProperties)
{
    QString activeStateStr = properties.value(s_propertyActiveState, QString()).toString();

    if ((activeStateStr == "active") && !m_sshdActive) {
        m_sshdActive = true;
        emit sshActiveChanged(m_sshdActive);
        qDebug() << Q_FUNC_INFO << activeStateStr << m_sshdActive;
    } else if (((activeStateStr == "inactive") || (activeStateStr == "failed")) && m_sshdActive) {
        m_sshdActive = false;
        emit sshActiveChanged(m_sshdActive);
        qDebug() << Q_FUNC_INFO << activeStateStr << m_sshdActive;
    } else {
        qDebug() << Q_FUNC_INFO << activeStateStr;
    }
}

void SystemDConnector::startSShdState(QDBusPendingCallWatcher* watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<QVariant> reply = *watcher;
    if (reply.isError()) {
        qWarning() << "ERROR" << reply.error().message();
        return;
    }

    qDebug() << Q_FUNC_INFO << reply.value();
}
