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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "powermonitor.h"
#include <QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QtWidgets>
#include <QDBusPendingCallWatcher>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void updateData() const;
    static void setUpdateDataError(const bool error);

private:
    Ui::MainWindow *ui;

    void setTabsEnabled(const bool enabled) const;
    void startRealtimeUpdate() const;
    void stopRealtimeUpdate() const;
    void setUpdateInterval(const int msec) const;
    void setModeFromSelection(const PowerProfile profile) const;
    void realtimeUpdate() const;
    void loadConfigs() const;
    void loadSettings() const;

    [[nodiscard]] QString intToQString(const int value) const;
    void updateBatteryCharge() const;
    void updatePenCharge() const;
    void updateBatteryThreshold() const;
    void updateChargingStatus() const;
    void updateCpuTemp() const;
    void updateGpuTemp() const;
    void updateFan1Speed() const;
    void updateFan2Speed() const;
    void updateKeyboardBacklightMode() const;
    void updateKeyboardBrightness() const;
    void updateUsbPowerShareState() const;
    void updateTurboMode() const;
    void updateFnMetaSwapState() const;
    void updateCoolerBoostState() const;
    void updateUserMode() const;

    void updateFanMode() const;
    void updateFanSpeedSettings() const;

    void setBestMobility() const;
    void setBalancedBattery() const;
    void setBestBattery() const;
    void setCustomBattery() const;
    void setCustomBatteryTrayLabel(const uint8_t value) const;

    void setPerformanceMode() const;
    void setBalancedMode() const;
    void setSilentMode() const;
    void setEcoSilentMode() const;

    void setCoolerBoostState(const bool enabled) const;

    void setKeyboardAlwaysOn() const;
    void setKeyboardAutoOff() const;

    QVector<int> getFan1SpeedValues() const;
    QVector<int> getFan2SpeedValues() const;
    QVector<int> getFan1TempValues() const;
    QVector<int> getFan2TempValues() const;
    void setFanSpeedSettings() const;
    void setFanModeAdvanced(const bool enabled) const;
    void checkFanSettingsChanged() const;

    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);
    void quitApp() const;

    QTimer timerSleepWatcher;
    qint64 timeLastWatcherInterval = 0;
    void timerSleepTimeout();

    void createTrayIcon();
    void createActions();
    void iconActivated(const QSystemTrayIcon::ActivationReason reason);
    void saveStateRequest(QSessionManager &sessionManager) const;

    QSystemTrayIcon *trayIcon = nullptr;;
    QMenu *trayIconMenu = nullptr;
    QMenu *modeTrayMenu = nullptr;
    QMenu *fanTrayMenu = nullptr;
    QMenu *batteryTrayMenu = nullptr;
    QMenu *keyboardTrayMenu = nullptr;
    QMenu *miscTrayMenu = nullptr;

    QAction *coolerBoostAction = nullptr;

    QActionGroup *modeTrayActions = nullptr;
    QAction *autoPPDMode = nullptr;
    QAction *autoAcDcMode = nullptr;
    QAction *turboMode = nullptr;
    QAction *performanceMode = nullptr;
    QAction *balancedMode = nullptr;
    QAction *silentMode = nullptr;
    QAction *ecoSilentMode = nullptr;

    QActionGroup *batteryTrayActions = nullptr;
    QAction *bestMobilityAction = nullptr;
    QAction *balancedBatteryAction = nullptr;
    QAction *bestBatteryAction = nullptr;
    QAction *customBatteryAction = nullptr;

    QActionGroup *keyboardTrayActions = nullptr;
    QAction *keyboardAlwaysOn = nullptr;
    QAction *keyboardAutoOff = nullptr;

    QAction *usbPowerShareTrayAction = nullptr;
    QAction *fnMetaSwapTrayAction = nullptr;

    QAction *restoreAction = nullptr;
    QAction *quitAction = nullptr;

private slots:
    void on_ChargerStateChange(const bool isCharging) const;
    void on_PowerProfileChange(const PowerProfile profile) const;

    void on_bestMobilityRadioButton_toggled(const bool checked) const;
    void on_balancedBatteryRadioButton_toggled(const bool checked) const;
    void on_bestBatteryRadioButton_toggled(const bool checked) const;
    void on_customBatteryThresholdRadioButton_toggled(const bool checked) const;
    void on_customBatteryThresholdSpinBox_valueChanged(const int arg1) const;
    void on_customBatteryApplyButton_clicked() const;
    void on_clearSettingsButton_clicked() const;
    void on_ReadValueButton_clicked() const;

    void on_WriteValueButton_clicked() const;

    void on_usbPowerShareCheckBox_clicked(const bool checked) const;
    void on_turboModeCheckBox_clicked(const bool checked) const;

    void on_fnMetaSwapCheckBox_clicked(const bool checked) const;

    void on_coolerBoostCheckBox_clicked(const bool checked) const;

    void on_keyboardBrightnessSlider_valueChanged(const int value) const;

    void on_keyboardBacklightModeComboBox_currentIndexChanged(const int index) const;

    void on_userModeOnBatteryComboBox_currentIndexChanged(const int index) const;
    void on_userModeOnChargerComboBox_currentIndexChanged(const int index) const;
    void on_autoPPDCheckBox_toggled(const bool checked) const;
    void on_autoAcDcProfilesGroupBox_toggled(const bool active) const;

    void on_performanceModeRadioButton_toggled(const bool checked) const;
    void on_balancedModeRadioButton_toggled(const bool checked) const;
    void on_silentModeRadioButton_toggled(const bool checked) const;
    void on_ecoSilentModeRadioButton_toggled(const bool checked) const;

    void clearSettings() const;
};
#endif // MAINWINDOW_H
