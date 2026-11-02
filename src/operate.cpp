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
#include "operate.h"
#include "settings.h"
#include <QFile>

#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusReply>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QVariant>

inline constexpr uint8_t fanSpeedSettingsCount = 7;
inline constexpr uint8_t fanTempSettingsCount = 6;

const QString settingsGroup = "Settings/";

Operate::Operate() {
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus system bus");
        return;
    }
    iface = new QDBusInterface(SERVICE_NAME, "/", INTERFACE_NAME, QDBusConnection::systemBus(), this);
}

void Operate::closeHelperApp() const {
    if (!iface) return;
    iface->call("quit");
    printError(iface->lastError());
}

bool Operate::isEcModuleLoaded() const {
    return value("ecModuleLoaded").toBool();
}

bool Operate::loadEcModule() const {
    if (!iface) return false;
    const QDBusReply<bool> reply = iface->call("loadEcModule");
    if (reply.isValid())
        return reply.value();
    printError(iface->lastError());
    return false;
}

bool Operate::updateEcData() const {
    if (!iface) return false;

    const QDBusReply<QVariantMap> reply = iface->call("getStatusSnapshot");

    if (!reply.isValid()) {
        printError(iface->lastError());
        return false;
    }

    statusSnapshot = reply.value();
    return statusSnapshot.value("valid").toBool();
}

void Operate::updateEcDataAsync() {
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

QVariant Operate::value(const char *key, const QVariant &fallback) const {
    return statusSnapshot.value(QString::fromLatin1(key), fallback);
}

QVector<int> Operate::intList(const char *key) const
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

QString Operate::getEcVersion() const { return value("ecVersion").toString(); }
QString Operate::getEcBuild() const { return value("ecBuild").toString(); }
QString Operate::getModelName() const { return value("modelName").toString(); }
bool Operate::isAcpiEc() const { return value("isAcpiEc").toBool(); }
bool Operate::isEcSys() const { return value("isEcSys").toBool(); }
uint8_t Operate::getBatteryCharge() const { return static_cast<uint8_t>(value("batteryCharge", 0).toInt()); }
uint8_t Operate::getBatteryThreshold() const { return static_cast<uint8_t>(value("batteryThreshold", 0).toInt()); }
charging_state Operate::getChargingStatus() const { return static_cast<charging_state>(value("chargingStatus", 5).toInt()); }
uint8_t Operate::getCpuTemp() const { return static_cast<uint8_t>(value("cpuTemp", 0).toInt()); }
std::optional<uint8_t> Operate::getGpuTemp() const { const int t = value("gpuTemp", -1).toInt(); if (t < 0) return std::nullopt; return static_cast<uint8_t>(t); }
int Operate::getFan1Speed() const { return value("fan1Speed", 0).toInt(); }
std::optional<int> Operate::getFan2Speed() const { const int v = value("fan2Speed", -1).toInt(); if (v < 0) return std::nullopt; return v; }
QVector<int> Operate::getFan1SpeedSettings() const { return intList("fan1SpeedSettings"); }
QVector<int> Operate::getFan2SpeedSettings() const { return intList("fan2SpeedSettings"); }
QVector<int> Operate::getFan1TempSettings() const { return intList("fan1TempSettings"); }
QVector<int> Operate::getFan2TempSettings() const { return intList("fan2TempSettings"); }
uint8_t Operate::getKeyboardBacklightMode() const { return static_cast<uint8_t>(value("keyboardBacklightMode", 0).toInt()); }
uint8_t Operate::getKeyboardBrightness() const { return static_cast<uint8_t>(value("keyboardBrightness", 0).toInt()); }
bool Operate::getUsbPowerShareState() const { return value("usbPowerShareState", false).toBool(); }
bool Operate::getFnMetaSwapState() const { return value("fnMetaSwapState", false).toBool(); }
bool Operate::getCoolerBoostState() const { return value("coolerBoostState", false).toBool(); }
bool Operate::getTurboMode() const { return Settings::getValue("Settings/TurboMode").toBool(); }
user_mode Operate::getUserMode() const { return static_cast<user_mode>(value("userMode", 4).toInt()); }
fan_mode Operate::getFanMode() const { return static_cast<fan_mode>(value("fanMode", 4).toInt()); }

void Operate::setBatteryThreshold(const uint8_t value) const { if (!iface) return; iface->call("setBatteryThreshold", value); printError(iface->lastError()); }
void Operate::setKeyboardBacklightMode(const uint8_t value) const { if (!iface) return; iface->call("setKeyboardBacklightMode", value != 0); printError(iface->lastError()); }
void Operate::setKeyboardBrightness(const uint8_t value) const { if (!iface) return; iface->call("setKeyboardBrightness", value); printError(iface->lastError()); }

void Operate::setUsbPowerShareState(const bool enabled) const {
    if (!iface) return;
    Settings::setValue(settingsGroup + "UsbPowerShare", enabled);
    iface->call("setUsbPowerShareState", enabled);
    printError(iface->lastError());
}

void Operate::setFnMetaSwapState(const bool enabled) const {
    if (!iface) return;
    Settings::setValue(settingsGroup + "FnMetaSwap", enabled);
    iface->call("setFnMetaSwapState", enabled);
    printError(iface->lastError());
}

void Operate::setCoolerBoostState(const bool enabled) const { if (!iface) return; iface->call("setCoolerBoostState", enabled); printError(iface->lastError()); }

void Operate::setTurboMode(const bool enabled) const {
    if (!iface) return;
    Settings::setValue("Settings/TurboMode", enabled);
    Settings::setValue("Settings/TurboMode", enabled);
    iface->call("setTurboMode", enabled);
    printError(iface->lastError());
}

void Operate::setUserMode(const user_mode userMode) const {
    if (!iface) return;
    if (userMode == user_mode::unknown_mode) return;
    static const char *names[] = {"performance_mode", "balanced_mode", "silent_mode", "eco_silent_mode", "unknown_mode"};
    Settings::setValue(settingsGroup + "UserMode", names[static_cast<int>(userMode)]);
    if (userMode == user_mode::performance_mode) {
        iface->call("setUserMode", Settings::getValue("Settings/TurboMode").toBool() ? 4 : 0);
    } else {
        iface->call("setUserMode", static_cast<int>(userMode));
    }
    printError(iface->lastError());
}

void Operate::setFan1SpeedSettings(const QVector<int> value) const {
    if (value.size() != fanSpeedSettingsCount) return;
    if (!iface) return;
    Settings::setValue(settingsGroup + "fan1SpeedSettings", value);
    const QList<int> list(value.cbegin(), value.cend());
    iface->call("setFan1SpeedSettings", QVariant::fromValue(list));
    printError(iface->lastError());
}

void Operate::setFan2SpeedSettings(const QVector<int> value) const {
    if (value.size() != fanSpeedSettingsCount) return;
    if (!iface) return;
    Settings::setValue(settingsGroup + "fan2SpeedSettings", value);
    const QList<int> list(value.cbegin(), value.cend());
    iface->call("setFan2SpeedSettings", QVariant::fromValue(list));
    printError(iface->lastError());
}

void Operate::setFan1TempSettings(const QVector<int> value) const {
    if (value.size() != fanTempSettingsCount) return;
    if (!iface) return;
    Settings::setValue(settingsGroup + "fan1TempSettings", value);
    const QList<int> list(value.cbegin(), value.cend());
    iface->call("setFan1TempSettings", QVariant::fromValue(list));
    printError(iface->lastError());
}

void Operate::setFan2TempSettings(const QVector<int> value) const {
    if (value.size() != fanTempSettingsCount) return;
    if (!iface) return;
    Settings::setValue(settingsGroup + "fan2TempSettings", value);
    const QList<int> list(value.cbegin(), value.cend());
    iface->call("setFan2TempSettings", QVariant::fromValue(list));
    printError(iface->lastError());
}

void Operate::setFanModeAdvanced(const bool enabled) const {
    const QString mode = Settings::getValue(settingsGroup + "UserMode").toString();
    const int userMode = mode == "performance_mode" ? 0 : mode == "balanced_mode" ? 1 : mode == "silent_mode" ? 2 : mode == "eco_silent_mode" ? 3 : 1;
    Settings::setValue(settingsGroup + "fanModeAdvanced", enabled);
    if (!iface) return;
    iface->call("setFanModeAdvanced", enabled, userMode);
    printError(iface->lastError());
}


void Operate::loadSettings() const {
    if (getUserMode() != user_mode::unknown_mode && Settings::isValueExist(settingsGroup + "UserMode")) {
        const QString value = Settings::getValue(settingsGroup + "UserMode").toString();
        if (value == "balanced_mode")
            setUserMode(user_mode::balanced_mode);
        else if (value == "performance_mode")
            setUserMode(user_mode::performance_mode);
        else if (value == "silent_mode")
            setUserMode(user_mode::silent_mode);
        else if (value == "eco_silent_mode")
            setUserMode(user_mode::eco_silent_mode);
    }

    if (Settings::isValueExist(settingsGroup + "FnMetaSwap"))
        setFnMetaSwapState(Settings::getValue(settingsGroup + "FnMetaSwap").toBool());
    if (Settings::isValueExist(settingsGroup + "UsbPowerShare"))
        setUsbPowerShareState(Settings::getValue(settingsGroup + "UsbPowerShare").toBool());

    if (Settings::isValueExist(settingsGroup + "fan1SpeedSettings"))
        setFan1SpeedSettings(Settings::getValueVector(settingsGroup + "fan1SpeedSettings"));
    if (Settings::isValueExist(settingsGroup + "fan2SpeedSettings"))
        setFan2SpeedSettings(Settings::getValueVector(settingsGroup + "fan2SpeedSettings"));
    if (Settings::isValueExist(settingsGroup + "fan1TempSettings"))
        setFan1TempSettings(Settings::getValueVector(settingsGroup + "fan1TempSettings"));
    if (Settings::isValueExist(settingsGroup + "fan2TempSettings"))
        setFan2TempSettings(Settings::getValueVector(settingsGroup + "fan2TempSettings"));
    if (Settings::isValueExist(settingsGroup + "fanModeAdvanced"))
        setFanModeAdvanced(Settings::getValue(settingsGroup + "fanModeAdvanced").toBool());
}

void Operate::handleWakeEvent() const {
    if (Settings::isValueExist(settingsGroup + "fanModeAdvanced"))
        setFanModeAdvanced(Settings::getValue(settingsGroup + "fanModeAdvanced").toBool());
}

void Operate::printError(const QDBusError &error) const {
    if (error.isValid())
        fprintf(stderr, "Call failed: %s\n", qPrintable(error.message()));
}
