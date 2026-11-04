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


#include <QtDBus/QDBusInterface>
#include <QtCore/QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QByteArray>
#include <QVector>

#include <optional>
#include <cstdint>
#include <string>

enum class ChargingState : int8_t {
    Unknown = -1,
    Charging,
    Discharging,
    NotCharging,
    FullyCharged,
    battery_fully_charged_no_power,
};

enum class UserMode : int8_t {
    Unknown = -1,
    Eco,
    Silent,
    Balanced,
    Performance,
    Turbo,
};

enum class FanMode : int8_t {
    Unknown = -1,
    Auto,
    Silent,
    Basic,
    Advanced,
};

enum class BatteryThreshold : int8_t {
    Unknown = -1,
    Mobility,
    Balanced,
    BestBattery,
    Custom,
};

class Operate : public QObject {
    Q_OBJECT

public:
    Operate();
    void closeHelperApp() const;
    [[nodiscard]] bool loadEcModule() const;
    void startMonitoring() const;
    void stopMonitoring() const;
    [[nodiscard]] bool updateEcData();
    [[nodiscard]] bool isEcModuleLoaded() const;
    [[nodiscard]] bool isAcpiEc() const;
    [[nodiscard]] bool isEcSys() const;
    [[nodiscard]] QString getEcVersion() const;
    [[nodiscard]] QString getEcBuild() const;
    [[nodiscard]] QString getModelName() const;
    [[nodiscard]] uint8_t getBatteryCharge() const;
    [[nodiscard]] uint8_t getBatteryThreshold() const;
    [[nodiscard]] ChargingState getChargingStatus() const;
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
    [[nodiscard]] UserMode getUserMode() const;
    [[nodiscard]] FanMode getFanMode() const;

    void setBatteryThreshold(uint8_t value) const;
    void setKeyboardBacklightMode(uint8_t value) const;
    void setKeyboardBrightness(uint8_t value) const;
    void setUsbPowerShareState(bool enabled) const;
    void setFnMetaSwapState(bool enabled) const;
    void setCoolerBoostState(bool enabled) const;
    void setTurboMode(bool enabled) const;
    void setUserMode(UserMode userMode) const;
    void setFanSettings(
        const QVector<int> &fan1Speed,
        const QVector<int> &fan2Speed,
        const QVector<int> &fan1Temp,
        const QVector<int> &fan2Temp
    ) const;
    void setFanModeAdvanced(bool enabled) const;


    void loadSettings() const;
    void handleWakeEvent() const;

private:
    QDBusInterface *iface = nullptr;

    QVariantMap metricsSnapshot;
    QVariantMap configurationSnapshot;
    QVariantMap staticSnapshot;

    QVariant value(
        const QVariantMap &snapshot,
        const char *key,
        const QVariant &fallback = {}
    ) const;
    QVector<int> intList(const QVariantMap &snapshot, const char *key) const;
    void printError(const QDBusError &error) const;

private slots:
    void onMetricsChanged(const QVariantMap &snapshot);
    void onConfigurationChanged(const QVariantMap &snapshot);

signals:
    void metricsUpdated();
    void configurationUpdated();
};

#endif // OPERATE_H
