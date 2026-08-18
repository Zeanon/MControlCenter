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

#ifndef OPERATE_H
#define OPERATE_H


#include <string>
#include <QVector>

enum class charging_state {
    battery_charging,
    battery_discharging,
    battery_not_charging,
    battery_fully_charged,
    battery_fully_charged_no_power,
    battery_unknown
};

enum class shift_mode {
    eco_mode,
    comfort_mode,
    sport_mode,
    turbo_mode,
    unknown_mode
};

enum class user_mode {
    performance_mode,
    balanced_mode,
    silent_mode,
    eco_silent_mode,
    unknown_mode
};

enum class fan_mode {
    auto_fan_mode,
    silent_fan_mode,
    basic_fan_mode,
    advanced_fan_mode,
    unknown_fan_mode
};

class Operate {
public:
    Operate();
    void closeHelperApp() const;
    [[nodiscard]] bool isEcModuleLoaded() const;
    [[nodiscard]] bool loadEcModule() const;
    [[nodiscard]] bool updateEcData() const;
    void updateEcDataAsync() const;
    [[nodiscard]] QString getEcVersion() const;
    [[nodiscard]] QString getEcBuild() const;
    [[nodiscard]] QString getModelName() const;
    [[nodiscard]] bool isAcpiEc() const;
    [[nodiscard]] bool isEcSys() const;
    [[nodiscard]] uint8_t getBatteryCharge() const;
    [[nodiscard]] uint8_t getBatteryThreshold() const;
    [[nodiscard]] charging_state getChargingStatus() const;
    [[nodiscard]] uint8_t getCpuTemp() const;
    [[nodiscard]] std::optional<uint8_t> getGpuTemp() const;
    [[nodiscard]] int getFan1Speed() const;
    [[nodiscard]] std::optional<int> getFan2Speed() const;
    [[nodiscard]] QVector<int> getFan1SpeedSettings() const;
    [[nodiscard]] QVector<int> getFan2SpeedSettings() const;
    [[nodiscard]] QVector<int> getFan1TempSettings() const;
    [[nodiscard]] QVector<int> getFan2TempSettings() const;

    [[nodiscard]] uint8_t getKeyboardBacklightMode() const;
    [[nodiscard]] uint8_t getKeyboardBrightness() const;
    [[nodiscard]] bool getUsbPowerShareState() const;
    [[nodiscard]] bool getFnMetaSwapState() const;
    [[nodiscard]] bool getCoolerBoostState() const;
    [[nodiscard]] bool getTurboMode() const;
    [[nodiscard]] user_mode getUserMode() const;
    [[nodiscard]] fan_mode getFanMode() const;

    void setBatteryThreshold(uint8_t value) const;
    void setKeyboardBacklightMode(uint8_t value) const;
    void setKeyboardBrightness(uint8_t value) const;
    void setUsbPowerShareState(bool enabled) const;
    void setFnMetaSwapState(bool enabled) const;
    void setCoolerBoostState(bool enabled) const;
    void setTurboMode(bool enabled) const;
    void setUserMode(user_mode userMode) const;
    void setFan1SpeedSettings(QVector<int> value) const;
    void setFan2SpeedSettings(QVector<int> value) const;
    void setFan1TempSettings(QVector<int> value) const;
    void setFan2TempSettings(QVector<int> value) const;
    void setFanMode(uint8_t value) const;
    void setFanModeAdvanced(bool enabled) const;

    [[nodiscard]] uint8_t getValue(uint8_t address) const;
    void setValue(uint8_t address, uint8_t value) const;

    void loadSettings() const;
    void handleWakeEvent() const;
private:
    void setUserModeValues(const uint8_t shiftMode, const uint8_t fanMode, const QString userMode) const;
};

#endif // OPERATE_H
