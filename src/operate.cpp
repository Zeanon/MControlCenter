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

#include "operate.h"
#include "helper.h"
#include "settings.h"

Helper helper;

const uint8_t cpuTempAddress = 0x68;
const uint8_t gpuTempAddress = 0x80;
const uint8_t batteryChargeAddress = 0x42;

const uint8_t batteryThresholdAddress = 0xD7;

const uint8_t batteryChargingStatusAddress = 0x31;
const uint8_t batteryNotCharging = 0x01;
const uint8_t batteryCharging = 0x03;
const uint8_t batteryDischarging = 0x05;
const uint8_t batteryFullyCharged = 0x09;
const uint8_t batteryFullyCharged_noPower = 0x0D;

const uint8_t keyboardBacklightModeAddress = 0x2C;
const uint8_t keyboardBacklightAutoTurnOffBit = 0x08;

const uint8_t keyboardBacklightAddress = 0xD3;
const uint8_t keyboardBacklight0ff = 0x80;
const uint8_t keyboardBacklightLow = 0x81;
const uint8_t keyboardBacklightMid = 0x82;
const uint8_t keyboardBacklightHigh = 0x83;

const uint8_t usbPowerShareAddress = 0xBF;
const uint8_t usbPowerShareBit = 0x20;

const uint8_t coolerBoostAddress = 0x98;
const uint8_t coolerBoostBit = 0x80;

const uint8_t fnMetaSwapAddress = 0xE8;
const uint8_t fnMetaSwapBit = 0x10;

const uint8_t fan1Address = 0xC9;
const uint8_t fan2Address = 0xCB;

const uint8_t fan1SpeedSettingStartAddress = 0x72;
const uint8_t fan2SpeedSettingStartAddress = 0x8A;
const uint8_t fanSpeedSettingsCount = 7;
const uint8_t fan1TempSettingStartAddress = 0x6A;
const uint8_t fan2TempSettingStartAddress = 0x82;
const uint8_t fanTempSettingsCount = fanSpeedSettingsCount - 1;


// Modes
const uint8_t shiftModeAddress = 0xD2;
const uint8_t shiftModeTurbo = 0xC5;
const uint8_t shiftModePerformance = 0xC4;
const uint8_t shiftModeBalanced = 0xC1;
const uint8_t shiftModeBattery = 0xC2;

const uint8_t fanModeAddress = 0xD4;
const uint8_t fanModeAuto = 0x0D;
const uint8_t fanModeSilent = 0x1D;
const uint8_t fanModeBasic = 0x4D;
const uint8_t fanModeAdvanced = 0x8D;

const QString settingsGroup = "Settings/";

Operate::Operate() = default;

void Operate::closeHelperApp() const {
    helper.quit();
}

bool Operate::isEcModuleLoaded() const {
    return helper.isEcModuleLoaded();
}

bool Operate::loadEcModule() const {
    return helper.loadEcModule();
}

bool Operate::updateEcData() const {
    return helper.updateData();
}

void Operate::updateEcDataAsync() const {
    helper.updateDataAsync();
}

QString Operate::getEcVersion() const {
    return QString(helper.getValues(0xA0, 12));
}

QString Operate::getEcBuild() const {
    std::string build = helper.getValues(0xAC, 16).toStdString();
    if (build.size() < 16)
        return QString::fromStdString(build);
    return QString::fromStdString(build.substr(0, 2) + "/" + build.substr(2, 2) + "/" + build.substr(4, 4) + " " + build.substr(8, 8));
}

QString Operate::getModelName() const {
    return QString(helper.getModelName());
}

bool Operate::isAcpiEc() const {
    return helper.isAcpiEc();
}

bool Operate::isEcSys() const {
    return helper.isEcSys();
}

uint8_t Operate::getBatteryCharge() const {
    return helper.getValue(batteryChargeAddress) - 1;
}

uint8_t Operate::getBatteryThreshold() const {
    return helper.getValue(batteryThresholdAddress) & ~0x80;
}

charging_state Operate::getChargingStatus() const {
    switch (helper.getValue(batteryChargingStatusAddress)) {
        case batteryCharging:
            return charging_state::battery_charging;
        case batteryDischarging:
            return charging_state::battery_discharging;
        case batteryNotCharging:
            return charging_state::battery_not_charging;
        case batteryFullyCharged:
            return charging_state::battery_fully_charged;
        case batteryFullyCharged_noPower:
            return charging_state::battery_fully_charged_no_power;
        default:
            return charging_state::battery_unknown;
    }
}

uint8_t Operate::getCpuTemp() const {
    return helper.getValue(cpuTempAddress);
}

std::optional<uint8_t> Operate::getGpuTemp() const {
    return helper.getOptionalValue(gpuTempAddress);
}

int Operate::getFan1Speed() const {
    // Read 2 bytes (big-endian)
    const uint8_t value0 = helper.getValue(fan1Address);
    const uint8_t value1 = helper.getValue(fan1Address - 1);
    const uint8_t value = (value1 << 8) | value0;
    if (value > 0)
        return 480000 / value;
    return value;
}

std::optional<int> Operate::getFan2Speed() const {
    // Read 2 bytes (big-endian)
    const auto value0 = helper.getOptionalValue(fan2Address);
    const auto value1 = helper.getOptionalValue(fan2Address - 1);
    if (!value0.has_value() || !value1.has_value())
        return std::nullopt;
    const uint8_t value = (value1.value() << 8) | value0.value();
    if (value > 0)
        return 480000 / value;
    return value;
}

QVector<int> Operate::getFan1SpeedSettings() const {
    QVector<int> a;
    for (uint8_t i = 0; i < fanSpeedSettingsCount; i++) {
        a.push_back(helper.getValue(fan1SpeedSettingStartAddress + i));
    }
    return a;
}

QVector<int> Operate::getFan2SpeedSettings() const {
    QVector<int> a;
    for (uint8_t i = 0; i < fanSpeedSettingsCount; i++) {
        a.push_back(helper.getValue(fan2SpeedSettingStartAddress + i));
    }
    return a;
}

QVector<int> Operate::getFan1TempSettings() const {
    QVector<int> a;
    for (uint8_t i = 0; i < fanTempSettingsCount; i++) {
        a.push_back(helper.getValue(fan1TempSettingStartAddress + i));
    }
    return a;
}

QVector<int> Operate::getFan2TempSettings() const {
    QVector<int> a;
    for (uint8_t i = 0; i < fanTempSettingsCount; i++) {
        a.push_back(helper.getValue(fan2TempSettingStartAddress + i));
    }
    return a;
}

uint8_t Operate::getKeyboardBacklightMode() const {
    return (helper.getValue(keyboardBacklightModeAddress) & keyboardBacklightAutoTurnOffBit) == keyboardBacklightAutoTurnOffBit ? 1 : 0;
}

uint8_t Operate::getKeyboardBrightness() const {
    switch (helper.getValue(keyboardBacklightAddress)) {
        case keyboardBacklight0ff:
            return 0;
        case keyboardBacklightLow:
            return 1;
        case keyboardBacklightMid:
            return 2;
        case keyboardBacklightHigh:
            return 3;
        default:
            return 0;
    }
}

bool Operate::getUsbPowerShareState() const {
    return helper.getValue(usbPowerShareAddress) & usbPowerShareBit;
}

bool Operate::getFnMetaSwapState() const {
    return helper.getValue(fnMetaSwapAddress) & fnMetaSwapBit;
}

bool Operate::getCoolerBoostState() const {
    return helper.getValue(coolerBoostAddress) & coolerBoostBit;
}

bool Operate::getTurboMode() const {
    return Settings::getValue("Settings/TurboMode").toBool();
}

user_mode Operate::getUserMode() const {
    switch (helper.getValue(shiftModeAddress)) {
        case shiftModeTurbo:
        case shiftModePerformance:
            return user_mode::performance_mode;
        case shiftModeBalanced:
            if (helper.getValue(fanModeAddress) == fanModeSilent)
                return user_mode::silent_mode;
            return user_mode::balanced_mode;
        case shiftModeBattery:
            return user_mode::eco_silent_mode;
        default:
            return user_mode::unknown_mode;
    }
}

fan_mode Operate::getFanMode() const {
    switch (helper.getValue(fanModeAddress)) {
        case fanModeAuto:
            return fan_mode::auto_fan_mode;
        case fanModeSilent:
            return fan_mode::silent_fan_mode;
        case fanModeBasic:
            return fan_mode::basic_fan_mode;
        case fanModeAdvanced:
            return fan_mode::advanced_fan_mode;
        default:
            return fan_mode::unknown_fan_mode;
    }
}

static inline void setEcBit(const uint8_t address, const uint8_t bit, const bool enabled) {
    uint8_t value = helper.getValue(address);
    helper.putValue(address, enabled ? value | bit : value & ~bit);
}

void Operate::setBatteryThreshold(const uint8_t value) const {
    if (value != getBatteryThreshold())
        helper.putValue(batteryThresholdAddress, value | 0x80);
}

void Operate::setKeyboardBacklightMode(const uint8_t value) const {
    setEcBit(keyboardBacklightModeAddress, keyboardBacklightAutoTurnOffBit, value);
}

void Operate::setKeyboardBrightness(const uint8_t value) const {
    switch (value) {
        case 3:
            helper.putValue(keyboardBacklightAddress, keyboardBacklightHigh);
            break;
        case 2:
            helper.putValue(keyboardBacklightAddress, keyboardBacklightMid);
            break;
        case 1:
            helper.putValue(keyboardBacklightAddress, keyboardBacklightLow);
            break;
        case 0:
        default:
            helper.putValue(keyboardBacklightAddress, keyboardBacklight0ff);
            break;
    }
}

void Operate::setUsbPowerShareState(const bool enabled) const {
    Settings::setValue(settingsGroup + "UsbPowerShare", enabled);
    setEcBit(usbPowerShareAddress, usbPowerShareBit, enabled);
}

void Operate::setFnMetaSwapState(const bool enabled) const {
    Settings::setValue(settingsGroup + "FnMetaSwap", enabled);
    setEcBit(fnMetaSwapAddress, fnMetaSwapBit, enabled);
}

void Operate::setCoolerBoostState(const bool enabled) const {
    setEcBit(coolerBoostAddress, coolerBoostBit, enabled);
}

void Operate::setTurboMode(const bool enabled) const {
    Settings::setValue("Settings/TurboMode", enabled);
    if (enabled && helper.getValue(shiftModeAddress) == shiftModePerformance) {
        helper.putValue(shiftModeAddress, shiftModeTurbo);
    } else if (!enabled && helper.getValue(shiftModeAddress) == shiftModeTurbo) {
        helper.putValue(shiftModeAddress, shiftModePerformance);
    }
}

static inline uint8_t getPerformanceMode() {
    return Settings::getValue("Settings/TurboMode").toBool() ? shiftModeTurbo : shiftModePerformance;
}

void Operate::setUserModeValues(const uint8_t shiftMode, const uint8_t fanMode, const QString userMode) const {
    helper.putValue(shiftModeAddress, shiftMode);
    setFanMode(fanMode);
    Settings::setValue(settingsGroup + "UserMode", userMode);
}

void Operate::setUserMode(const user_mode userMode) const {
    switch (userMode) {
        case user_mode::performance_mode:
            setUserModeValues(getPerformanceMode(), fanModeAuto, "performance_mode");
            break;
        case user_mode::balanced_mode:
            setUserModeValues(shiftModeBalanced, fanModeAuto, "balanced_mode");
            break;
        case user_mode::silent_mode:
            setUserModeValues(shiftModeBalanced, fanModeSilent, "silent_mode");
            break;
        case user_mode::eco_silent_mode:
            setUserModeValues(shiftModeBattery, fanModeSilent, "eco_silent_mode");
            break;
        default:
            break;
    }
}

void Operate::setFan1SpeedSettings(const QVector<int> value) const {
    if (value.size() != fanSpeedSettingsCount)
        return;
    for (uint8_t i = 0; i < value.size(); i++) {
        helper.putValue(fan1SpeedSettingStartAddress + i, value[i]);
    }
    Settings::setValue(settingsGroup + QString("fan1SpeedSettings"), value);
}

void Operate::setFan2SpeedSettings(const QVector<int> value) const {
    if (value.size() != fanSpeedSettingsCount)
        return;
    for (uint8_t i = 0; i < value.size(); i++) {
        helper.putValue(fan2SpeedSettingStartAddress + i, value[i]);
    }
    Settings::setValue(settingsGroup + QString("fan2SpeedSettings"), value);
}

void Operate::setFan1TempSettings(const QVector<int> value) const {
    if (value.size() != fanTempSettingsCount)
        return;
    for (uint8_t i = 0; i < value.size(); i++) {
        helper.putValue(fan1TempSettingStartAddress + i, value[i]);
    }
    Settings::setValue(settingsGroup + QString("fan1TempSettings"), value);
}

void Operate::setFan2TempSettings(const QVector<int> value) const {
    if (value.size() != fanTempSettingsCount)
        return;
    for (uint8_t i = 0; i < value.size(); i++) {
        helper.putValue(fan2TempSettingStartAddress + i, value[i]);
    }
    Settings::setValue(settingsGroup + QString("fan2TempSettings"), value);
}

void Operate::setFanMode(const uint8_t value) const {
    if (helper.getValue(fanModeAddress) == fanModeAdvanced)
        return;
    helper.putValue(fanModeAddress, value);
}

void Operate::setFanModeAdvanced(const bool enabled) const {
    if (enabled) {
        helper.putValue(fanModeAddress, fanModeAdvanced);
    } else {
        const QString value = Settings::getValue(settingsGroup + "UserMode").toString();
        if (value == "silent_mode" || value == "eco_silent_mode")
            helper.putValue(fanModeAddress, fanModeSilent);
        else
            helper.putValue(fanModeAddress, fanModeAuto);
    }
    Settings::setValue(settingsGroup + "fanModeAdvanced", enabled);
}

uint8_t Operate::getValue(const uint8_t address) const {
    helper.updateData();
    return helper.getValue(address);
}

void Operate::setValue(const uint8_t address, const uint8_t value) const {
    helper.putValue(address, value);
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
