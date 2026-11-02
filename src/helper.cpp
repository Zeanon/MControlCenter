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
#include "settings.h"
#include "helper.h"
#include "mainwindow.h"

#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusReply>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QVariant>

#include <cstdio>


Helper::Helper() {
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus system bus");
        return;
    }
    iface = new QDBusInterface(SERVICE_NAME, "/", INTERFACE_NAME, QDBusConnection::systemBus(), this);
}

bool Helper::updateData() const {
    if (!iface) return false;

    const QDBusReply<QVariantMap> reply = iface->call("getStatusSnapshot");

    if (!reply.isValid()) {
        printError(iface->lastError());
        return false;
    }

    statusSnapshot = reply.value();
    return statusSnapshot.value("valid").toBool();
}

void Helper::updateDataAsync()
{
    if (!iface)
        return;

    // Keine parallelen Statusabfragen starten
    if (updatePending)
        return;

    updatePending = true;

    auto *watcher = new QDBusPendingCallWatcher(
        iface->asyncCall("getStatusSnapshot"), this
    );

    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this](QDBusPendingCallWatcher *watcher) {

        const QDBusPendingReply<QVariantMap> reply = *watcher;

        bool valid = false;

        if (reply.isValid()) {
            statusSnapshot = reply.value();
            valid = statusSnapshot.value("valid").toBool();
        } else {
            printError(reply.error());
        }

        updatePending = false;

        emit dataUpdated(valid);

        watcher->deleteLater();
    });
}

QVariant Helper::value(const char *key, const QVariant &fallback) const {
    return statusSnapshot.value(QString::fromLatin1(key), fallback);
}

QVector<int> Helper::intList(const char *key) const
{
    QVector<int> result;

    const QVariant data = value(key);

    if (data.metaType().id() == QMetaType::QVariantList) {
        const QVariantList values = data.toList();

        result.reserve(values.size());

        for (const QVariant &item : values)
            result.append(item.toInt());

    } else if (data.metaType().id() == qMetaTypeId<QDBusArgument>()) {
        const QDBusArgument argument = data.value<QDBusArgument>();

        argument.beginArray();

        while (!argument.atEnd()) {
            QVariant item;
            argument >> item;
            result.append(item.toInt());
        }

        argument.endArray();
    }

    return result;
}

bool Helper::loadEcModule() const {
    if (!iface) return false;
    const QDBusReply<bool> reply = iface->call("loadEcModule");
    if (reply.isValid())
        return reply.value();
    printError(iface->lastError());
    return false;
}

bool Helper::isEcModuleLoaded() const { return value("ecModuleLoaded").toBool(); }
bool Helper::isAcpiEc() const { return value("isAcpiEc").toBool(); }
bool Helper::isEcSys() const { return value("isEcSys").toBool(); }
QString Helper::getModelName() const { return value("modelName").toString(); }
QString Helper::getEcVersion() const { return value("ecVersion").toString(); }
QString Helper::getEcBuild() const { return value("ecBuild").toString(); }
uint8_t Helper::getBatteryCharge() const { return static_cast<uint8_t>(value("batteryCharge", 0).toInt()); }
uint8_t Helper::getBatteryThreshold() const { return static_cast<uint8_t>(value("batteryThreshold", 0).toInt()); }
int Helper::getChargingStatus() const { return value("chargingStatus", 5).toInt(); }
uint8_t Helper::getCpuTemp() const { return static_cast<uint8_t>(value("cpuTemp", 0).toInt()); }
std::optional<uint8_t> Helper::getGpuTemp() const { const int t = value("gpuTemp", -1).toInt(); if (t < 0) return std::nullopt; return static_cast<uint8_t>(t); }
int Helper::getFan1Speed() const { return value("fan1Speed", 0).toInt(); }
std::optional<int> Helper::getFan2Speed() const { const int v = value("fan2Speed", -1).toInt(); if (v < 0) return std::nullopt; return v; }
QVector<int> Helper::getFan1SpeedSettings() const { return intList("fan1SpeedSettings"); }
QVector<int> Helper::getFan2SpeedSettings() const { return intList("fan2SpeedSettings"); }
QVector<int> Helper::getFan1TempSettings() const { return intList("fan1TempSettings"); }
QVector<int> Helper::getFan2TempSettings() const { return intList("fan2TempSettings"); }
uint8_t Helper::getKeyboardBacklightMode() const { return static_cast<uint8_t>(value("keyboardBacklightMode", 0).toInt()); }
uint8_t Helper::getKeyboardBrightness() const { return static_cast<uint8_t>(value("keyboardBrightness", 0).toInt()); }
bool Helper::getUsbPowerShareState() const { return value("usbPowerShareState", false).toBool(); }
bool Helper::getFnMetaSwapState() const { return value("fnMetaSwapState", false).toBool(); }
bool Helper::getCoolerBoostState() const { return value("coolerBoostState", false).toBool(); }
bool Helper::getTurboMode() const {
    return Settings::getValue("Settings/TurboMode").toBool();
}
int Helper::getUserMode() const { return value("userMode", 4).toInt(); }
int Helper::getFanMode() const { return value("fanMode", 4).toInt(); }

void Helper::setBatteryThreshold(const uint8_t value) const { if (!iface) return; iface->call("setBatteryThreshold", value); printError(iface->lastError()); }
void Helper::setKeyboardBacklightMode(const bool enabled) const { if (!iface) return; iface->call("setKeyboardBacklightMode", enabled); printError(iface->lastError()); }
void Helper::setKeyboardBrightness(const uint8_t value) const { if (!iface) return; iface->call("setKeyboardBrightness", value); printError(iface->lastError()); }
void Helper::setUsbPowerShareState(const bool enabled) const { if (!iface) return; iface->call("setUsbPowerShareState", enabled); printError(iface->lastError()); }
void Helper::setFnMetaSwapState(const bool enabled) const { if (!iface) return; iface->call("setFnMetaSwapState", enabled); printError(iface->lastError()); }
void Helper::setCoolerBoostState(const bool enabled) const { if (!iface) return; iface->call("setCoolerBoostState", enabled); printError(iface->lastError()); }
void Helper::setTurboMode(const bool enabled) const {
    if (!iface)
        return;
    Settings::setValue("Settings/TurboMode", enabled);
    iface->call("setTurboMode", enabled);
    printError(iface->lastError());
}
void Helper::setUserMode(const int mode) const { if (!iface) return; iface->call("setUserMode", mode); printError(iface->lastError()); }
void Helper::setFan1SpeedSettings(const QVector<int> &values) const { if (!iface) return; const QList<int> list(values.cbegin(), values.cend()); iface->call("setFan1SpeedSettings", QVariant::fromValue(list)); printError(iface->lastError()); }
void Helper::setFan2SpeedSettings(const QVector<int> &values) const { if (!iface) return; const QList<int> list(values.cbegin(), values.cend()); iface->call("setFan2SpeedSettings", QVariant::fromValue(list)); printError(iface->lastError()); }
void Helper::setFan1TempSettings(const QVector<int> &values) const { if (!iface) return; const QList<int> list(values.cbegin(), values.cend()); iface->call("setFan1TempSettings", QVariant::fromValue(list)); printError(iface->lastError()); }
void Helper::setFan2TempSettings(const QVector<int> &values) const { if (!iface) return; const QList<int> list(values.cbegin(), values.cend()); iface->call("setFan2TempSettings", QVariant::fromValue(list)); printError(iface->lastError()); }
void Helper::setFanModeAdvanced(const bool enabled, const int userMode) const { if (!iface) return; iface->call("setFanModeAdvanced", enabled, userMode); printError(iface->lastError()); }

void Helper::quit() const {
    if (!iface) return;
    iface->call("quit");
    printError(iface->lastError());
}

void Helper::printError(const QDBusError &error) const {
    if (error.isValid())
        fprintf(stderr, "Call failed: %s\n", qPrintable(error.message()));
}
