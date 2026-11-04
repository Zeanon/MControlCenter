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
#include "operate.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusReply>
#include <QVariant>
#include <QFile>

#include <algorithm>

inline constexpr uint8_t fanSpeedSettingsCount = 7;
inline constexpr uint8_t fanTempSettingsCount = 6;

Operate::Operate() {
    QDBusConnection bus = QDBusConnection::systemBus();

    if (!bus.isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus system bus\n");
        return;
    }

    iface = new QDBusInterface(
        SERVICE_NAME,
        "/",
        INTERFACE_NAME,
        bus,
        this
    );

    if (!iface->isValid()) {
        printError(iface->lastError());
        delete iface;
        iface = nullptr;
        return;
    }

    const bool metricsConnected = bus.connect(
        SERVICE_NAME,
        "/",
        INTERFACE_NAME,
        "MetricsChanged",
        this,
        SLOT(onMetricsChanged(QVariantMap))
    );

    const bool configurationConnected = bus.connect(
        SERVICE_NAME,
        "/",
        INTERFACE_NAME,
        "ConfigurationChanged",
        this,
        SLOT(onConfigurationChanged(QVariantMap))
    );

    if (!metricsConnected)
        fprintf(stderr, "Could not connect MetricsChanged signal\n");

    if (!configurationConnected)
        fprintf(stderr, "Could not connect ConfigurationChanged signal\n");
}

void Operate::closeHelperApp() const {
    if (!iface) return;
    iface->call("quit");
    printError(iface->lastError());
}

bool Operate::loadEcModule() const {
    if (!iface) return false;
    const QDBusReply<bool> reply = iface->call("loadEcModule");
    if (reply.isValid())
        return reply.value();
    printError(iface->lastError());
    return false;
}

void Operate::startMonitoring() const {
    if (!iface) return;
    iface->call("startMonitoring");
    printError(iface->lastError());
}

void Operate::stopMonitoring() const {
    if (!iface) return;
    iface->call("stopMonitoring");
    printError(iface->lastError());
}

bool Operate::updateEcData() {
    if (!iface)
        return false;

    const QDBusReply<QVariantMap> reply =
        iface->call("getStatusSnapshot");

    if (!reply.isValid()) {
        printError(iface->lastError());
        return false;
    }

    const QVariantMap snapshot = reply.value();

    if (!snapshot.value("valid").toBool())
        return false;

    // Häufig wechselnde Messwerte
    metricsSnapshot = {
        {"batteryCharge", snapshot.value("batteryCharge")},
        {"chargingStatus", snapshot.value("chargingStatus")},
        {"cpuTemp", snapshot.value("cpuTemp")},
        {"gpuTemp", snapshot.value("gpuTemp")},
        {"fan1Speed", snapshot.value("fan1Speed")},
        {"fan2Speed", snapshot.value("fan2Speed")}
    };

    // Konfiguration und Betriebszustände
    configurationSnapshot = {
        {"batteryThreshold", snapshot.value("batteryThreshold")},
        {"fan1SpeedSettings", snapshot.value("fan1SpeedSettings")},
        {"fan2SpeedSettings", snapshot.value("fan2SpeedSettings")},
        {"fan1TempSettings", snapshot.value("fan1TempSettings")},
        {"fan2TempSettings", snapshot.value("fan2TempSettings")},
        {"keyboardBacklightMode", snapshot.value("keyboardBacklightMode")},
        {"keyboardBrightness", snapshot.value("keyboardBrightness")},
        {"usbPowerShareState", snapshot.value("usbPowerShareState")},
        {"fnMetaSwapState", snapshot.value("fnMetaSwapState")},
        {"coolerBoostState", snapshot.value("coolerBoostState")},
        {"userMode", snapshot.value("userMode")},
        {"fanMode", snapshot.value("fanMode")}
    };

    // Statische Informationen
    staticSnapshot = {
        {"ecVersion", snapshot.value("ecVersion")},
        {"ecBuild", snapshot.value("ecBuild")},
        {"modelName", snapshot.value("modelName")},
        {"ecModuleLoaded", snapshot.value("ecModuleLoaded")},
        {"isAcpiEc", snapshot.value("isAcpiEc")},
        {"isEcSys", snapshot.value("isEcSys")}
    };

    return true;
}

inline QVariant Operate::value(
    const QVariantMap &snapshot,
    const char *key,
    const QVariant &fallback
) const {
    return snapshot.value(QString::fromLatin1(key), fallback);
}

QVector<int> Operate::intList(
    const QVariantMap &snapshot,
    const char *key
) const {
    QVector<int> result;
    const QVariant data = value(snapshot, key);

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

bool Operate::isEcModuleLoaded() const {
    return value(staticSnapshot, "ecModuleLoaded").toBool();
}

bool Operate::isAcpiEc() const {
    return value(staticSnapshot, "isAcpiEc").toBool();
}

bool Operate::isEcSys() const {
    return value(staticSnapshot, "isEcSys").toBool();
}

QString Operate::getEcVersion() const {
    return value(staticSnapshot, "ecVersion").toString();
}

QString Operate::getEcBuild() const {
    return value(staticSnapshot, "ecBuild").toString();
}

QString Operate::getModelName() const {
    return value(staticSnapshot, "modelName").toString();
}

uint8_t Operate::getBatteryCharge() const {
    return static_cast<uint8_t>(
        value(metricsSnapshot, "batteryCharge", 0).toInt()
    );
}

ChargingState Operate::getChargingStatus() const {
    return static_cast<ChargingState>(
        value(metricsSnapshot, "chargingStatus", 5).toInt()
    );
}

uint8_t Operate::getCpuTemp() const {
    return static_cast<uint8_t>(
        value(metricsSnapshot, "cpuTemp", 0).toInt()
    );
}

std::optional<uint8_t> Operate::getGpuTemp() const {
    const int t = value(metricsSnapshot, "gpuTemp", -1).toInt();
    if (t < 0)
        return std::nullopt;
    return static_cast<uint8_t>(t);
}

int Operate::getFan1Speed() const {
    return value(metricsSnapshot, "fan1Speed", 0).toInt();
}

std::optional<int> Operate::getFan2Speed() const {
    const int v = value(metricsSnapshot, "fan2Speed", -1).toInt();
    if (v < 0)
        return std::nullopt;
    return v;
}

uint8_t Operate::getBatteryThreshold() const {
    return static_cast<uint8_t>(
        value(configurationSnapshot, "batteryThreshold", 0).toInt()
    );
}

uint8_t Operate::getKeyboardBacklightMode() const {
    return static_cast<uint8_t>(
        value(configurationSnapshot, "keyboardBacklightMode", 0).toInt()
    );
}

uint8_t Operate::getKeyboardBrightness() const {
    return static_cast<uint8_t>(
        value(configurationSnapshot, "keyboardBrightness", 0).toInt()
    );
}

bool Operate::getUsbPowerShareState() const {
    return value(configurationSnapshot, "usbPowerShareState", false).toBool();
}

bool Operate::getFnMetaSwapState() const {
    return value(configurationSnapshot, "fnMetaSwapState", false).toBool();
}

bool Operate::getCoolerBoostState() const {
    return value(configurationSnapshot, "coolerBoostState", false).toBool();
}

bool Operate::getTurboMode() const {
    return Settings::getValue("Settings/TurboMode").toBool();
}

UserMode Operate::getUserMode() const {
    return static_cast<UserMode>(
        value(configurationSnapshot, "userMode", static_cast<int8_t>(UserMode::Unknown)).toInt()
    );
}

FanMode Operate::getFanMode() const {
    return static_cast<FanMode>(
        value(configurationSnapshot, "fanMode", static_cast<int8_t>(FanMode::Unknown)).toInt()
    );
}

QVector<int> Operate::getFan1SpeedSettings() const {
    return intList(configurationSnapshot, "fan1SpeedSettings");
}

QVector<int> Operate::getFan2SpeedSettings() const {
    return intList(configurationSnapshot, "fan2SpeedSettings");
}

QVector<int> Operate::getFan1TempSettings() const {
    return intList(configurationSnapshot, "fan1TempSettings");
}

QVector<int> Operate::getFan2TempSettings() const {
    return intList(configurationSnapshot, "fan2TempSettings");
}


void Operate::setBatteryThreshold(const uint8_t value) const {
    if (!iface) return;

    iface->call("setBatteryThreshold", value);
    printError(iface->lastError());
}

void Operate::setKeyboardBacklightMode(const uint8_t value) const {
    if (!iface) return;

    iface->call("setKeyboardBacklightMode", value != 0);
    printError(iface->lastError());
}

void Operate::setKeyboardBrightness(const uint8_t value) const {
    if (!iface) return;

    iface->call("setKeyboardBrightness", value);
    printError(iface->lastError());
}

void Operate::setUsbPowerShareState(const bool enabled) const {
    if (!iface) return;

    Settings::setValue("Settings/UsbPowerShare", enabled);
    iface->call("setUsbPowerShareState", enabled);
    printError(iface->lastError());
}

void Operate::setFnMetaSwapState(const bool enabled) const {
    if (!iface) return;

    Settings::setValue("Settings/FnMetaSwap", enabled);
    iface->call("setFnMetaSwapState", enabled);
    printError(iface->lastError());
}

void Operate::setCoolerBoostState(const bool enabled) const {
    if (!iface) return;

    iface->call("setCoolerBoostState", enabled);
    printError(iface->lastError());
}

void Operate::setTurboMode(const bool enabled) const {
    if (!iface) return;

    Settings::setValue("Settings/TurboMode", enabled);
    iface->call("setTurboMode", enabled);
    printError(iface->lastError());
}

void Operate::setUserMode(const UserMode userMode) const {
    if (!iface) return;
    if (userMode == UserMode::Unknown) return;

    Settings::setValue("Settings/UserMode", userMode);
    iface->call("setUserMode", static_cast<int>(userMode));
    printError(iface->lastError());
}

void Operate::setFanSettings(
    const QVector<int> &fan1Speed,
    const QVector<int> &fan2Speed,
    const QVector<int> &fan1Temp,
    const QVector<int> &fan2Temp
) const {
    if (!iface)
        return;

    const auto isValidSettings = [](const QVector<int> &values, const qsizetype expectedSize) {
        return values.size() == expectedSize &&
            std::all_of(values.cbegin(), values.cend(), [](const int value) {
                return value >= 0 && value <= 255;
            });
    };

    if (!isValidSettings(fan1Speed, fanSpeedSettingsCount) ||
        !isValidSettings(fan2Speed, fanSpeedSettingsCount) ||
        !isValidSettings(fan1Temp, fanTempSettingsCount) ||
        !isValidSettings(fan2Temp, fanTempSettingsCount))
        return;

    const QList<int> fan1SpeedList(fan1Speed.cbegin(), fan1Speed.cend());
    const QList<int> fan2SpeedList(fan2Speed.cbegin(), fan2Speed.cend());
    const QList<int> fan1TempList(fan1Temp.cbegin(), fan1Temp.cend());
    const QList<int> fan2TempList(fan2Temp.cbegin(), fan2Temp.cend());

    const QDBusReply<bool> reply = iface->call(
        "setFanSettings",
        QVariant::fromValue(fan1SpeedList),
        QVariant::fromValue(fan2SpeedList),
        QVariant::fromValue(fan1TempList),
        QVariant::fromValue(fan2TempList)
    );

    if (!reply.isValid()) {
        printError(reply.error());
        return;
    }

    if (!reply.value())
        return;

    Settings::setValue("Settings/fan1SpeedSettings", fan1Speed);
    Settings::setValue("Settings/fan2SpeedSettings", fan2Speed);
    Settings::setValue("Settings/fan1TempSettings", fan1Temp);
    Settings::setValue("Settings/fan2TempSettings", fan2Temp);
}

void Operate::setFanModeAdvanced(const bool enabled) const {
    if (!iface) return;

    Settings::setValue("Settings/fanModeAdvanced", enabled);
    iface->call("setFanModeAdvanced", enabled);
    printError(iface->lastError());
}


void Operate::loadSettings() const {
    if (getUserMode() != UserMode::Unknown && Settings::isValueExist("Settings/UserMode")) {
        const UserMode userMode = static_cast<UserMode>(Settings::getValue("Settings/UserMode").toInt());
        if (userMode != UserMode::Unknown) {
            setUserMode(userMode);
        }
    }

    if (Settings::isValueExist("Settings/FnMetaSwap"))
        setFnMetaSwapState(Settings::getValue("Settings/FnMetaSwap").toBool());
    if (Settings::isValueExist("Settings/UsbPowerShare"))
        setUsbPowerShareState(Settings::getValue("Settings/UsbPowerShare").toBool());

    if (Settings::isValueExist("Settings/fanModeAdvanced"))
        setFanModeAdvanced(Settings::getValue("Settings/fanModeAdvanced").toBool());

    QVector<int> fan1Speed = getFan1SpeedSettings();
    QVector<int> fan2Speed = getFan2SpeedSettings();
    QVector<int> fan1Temp = getFan1TempSettings();
    QVector<int> fan2Temp = getFan2TempSettings();

    bool hasFanSettings = false;

    if (Settings::isValueExist("Settings/fan1SpeedSettings")) {
        fan1Speed = Settings::getValueVector("Settings/fan1SpeedSettings");
        hasFanSettings = true;
    }

    if (Settings::isValueExist("Settings/fan2SpeedSettings")) {
        fan2Speed = Settings::getValueVector("Settings/fan2SpeedSettings");
        hasFanSettings = true;
    }

    if (Settings::isValueExist("Settings/fan1TempSettings")) {
        fan1Temp = Settings::getValueVector("Settings/fan1TempSettings");
        hasFanSettings = true;
    }

    if (Settings::isValueExist("Settings/fan2TempSettings")) {
        fan2Temp = Settings::getValueVector("Settings/fan2TempSettings");
        hasFanSettings = true;
    }

    if (hasFanSettings) {
        setFanSettings(fan1Speed, fan2Speed, fan1Temp, fan2Temp);
    }
}

void Operate::handleWakeEvent() const {
    if (Settings::isValueExist("Settings/fanModeAdvanced"))
        setFanModeAdvanced(Settings::getValue("Settings/fanModeAdvanced").toBool());
}

void Operate::printError(const QDBusError &error) const {
    if (error.isValid())
        fprintf(stderr, "Call failed: %s\n", qPrintable(error.message()));
}

void Operate::onMetricsChanged(const QVariantMap &snapshot) {
    metricsSnapshot = snapshot;
    emit metricsUpdated();
}

void Operate::onConfigurationChanged(const QVariantMap &snapshot) {
    configurationSnapshot = snapshot;
    emit configurationUpdated();
}
