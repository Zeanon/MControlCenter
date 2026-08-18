/* Copyright (C) 2022  Dmitry Serov
 *
 * This file is part of MControlCenter.
 *
 * MControlCenter is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * MControlCenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MControlCenter. If not, see <https://www.gnu.org/licenses/>.
 */

#include "helper/service.h"
#include "helper.h"
#include "mainwindow.h"

#include <QCoreApplication>
#include <QDBusReply>

#include <cstdio>

#define EC_SPACE_SIZE 256

QByteArray ecData;

Helper::Helper() {
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus system bus");
        return;
    }
    iface = new QDBusInterface(SERVICE_NAME, "/", INTERFACE_NAME, QDBusConnection::systemBus());
}

bool Helper::isEcModuleLoaded() const {
    const QDBusReply<bool> reply = iface->call("isEcModuleLoaded");
    if (reply.isValid())
        return reply.value();
    printError(iface->lastError());
    return false;
}

bool Helper::loadEcModule() const {
    const QDBusReply<bool> reply = iface->call("loadEcSysModule");
    if (reply.isValid())
        return reply.value();
    printError(iface->lastError());
    return false;
}

bool Helper::isAcpiEc() const {
    const QDBusReply<bool> reply = iface->call("isAcpiEc");
    if (reply.isValid())
        return reply.value();
    printError(iface->lastError());
    return false;
}

bool Helper::isEcSys() const {
    const QDBusReply<bool> reply = iface->call("isEcSys");
    if (reply.isValid())
        return reply.value();
    printError(iface->lastError());
    return false;
}

QByteArray Helper::getModelName() const {
    const QDBusReply<QByteArray> reply = iface->call("getModelName");
    if (reply.isValid())
        return reply.value();
    printError(iface->lastError());
    return {};
}

bool Helper::updateData() const {
    const QDBusReply<QByteArray> reply = iface->call("getData");
    if (reply.isValid() && reply.value().size() == EC_SPACE_SIZE) {
        ecData = reply.value();
        return true;
    }
    printError(iface->lastError());
    return false;
}

void Helper::updateDataAsync() {
    const QDBusPendingCall async = iface->asyncCall("getData");
    const QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),this, SLOT(callFinishedSlot(QDBusPendingCallWatcher*)));
}

void Helper::callFinishedSlot(QDBusPendingCallWatcher *call) const {
    const QDBusPendingReply<QByteArray> reply = *call;
    if (reply.isError()) {
        printError(reply.error());
        MainWindow::setUpdateDataError(true);
    } else {
        ecData = reply.value();
        MainWindow::setUpdateDataError(false);
    }
    call->deleteLater();
}

std::optional<uint8_t> Helper::getOptionalValue(const uint8_t address) const {
    if (!ecData.isEmpty())
        return (uint8_t) ecData[address];
    return std::nullopt;
}

uint8_t Helper::getValue(const uint8_t address) const {
    return getOptionalValue(address).value_or(-1);
}

QByteArray Helper::getValues(const uint8_t startAddress, const uint8_t size) const {
    return ecData.mid(startAddress, size);
}

void Helper::putValue(const uint8_t address, const uint8_t value) const {
    if (getValue(address) == value)
        return;
    iface->call("putValue", address, value);
    printError(iface->lastError());
}

void Helper::quit() const {
    iface->call("quit");
    printError(iface->lastError());
}

void Helper::printError(const QDBusError &error) const {
    if (error.isValid())
        fprintf(stderr, "Call failed: %s\n", qPrintable(error.message()));
}
