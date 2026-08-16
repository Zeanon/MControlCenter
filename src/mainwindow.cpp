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

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "operate.h"
#include "settings.h"
#include <QTimer>
#include <QMessageBox>

Operate operate;
PowerMonitor powerMonitor;

bool isActive = false;
bool isUpdateDataError = false;

QTimer *realtimeUpdateTimer = new QTimer;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    connect(ui->advancedFanControlCheckBox, &QCheckBox::toggled, this, &MainWindow::setFanModeAdvanced);

    connect(ui->fan1Speed1Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan1Speed1SpinBox->setValue(ui->fan1Speed1Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed1SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan1Speed1Slider->setValue(ui->fan1Speed1SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed2Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan1Speed2SpinBox->setValue(ui->fan1Speed2Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed2SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan1Speed2Slider->setValue(ui->fan1Speed2SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed3Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan1Speed3SpinBox->setValue(ui->fan1Speed3Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed3SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan1Speed3Slider->setValue(ui->fan1Speed3SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed4Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan1Speed4SpinBox->setValue(ui->fan1Speed4Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed4SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan1Speed4Slider->setValue(ui->fan1Speed4SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed5Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan1Speed5SpinBox->setValue(ui->fan1Speed5Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed5SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan1Speed5Slider->setValue(ui->fan1Speed5SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed6Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan1Speed6SpinBox->setValue(ui->fan1Speed6Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed6SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan1Speed6Slider->setValue(ui->fan1Speed6SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed7Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan1Speed7SpinBox->setValue(ui->fan1Speed7Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed7SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan1Speed7Slider->setValue(ui->fan1Speed7SpinBox->value());
        checkFanSettingsChanged();
    });

    connect(ui->fan1Speed2TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan1Speed1TempLabel->setText(QString("< %1 °C").arg(ui->fan1Speed2TempSpinBox->value()));
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed3TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed4TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed5TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed6TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        checkFanSettingsChanged();
    });
    connect(ui->fan1Speed7TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        checkFanSettingsChanged();
    });

    connect(ui->fan2Speed1Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan2Speed1SpinBox->setValue(ui->fan2Speed1Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed1SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan2Speed1Slider->setValue(ui->fan2Speed1SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed2Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan2Speed2SpinBox->setValue(ui->fan2Speed2Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed2SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan2Speed2Slider->setValue(ui->fan2Speed2SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed3Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan2Speed3SpinBox->setValue(ui->fan2Speed3Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed3SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan2Speed3Slider->setValue(ui->fan2Speed3SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed4Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan2Speed4SpinBox->setValue(ui->fan2Speed4Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed4SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan2Speed4Slider->setValue(ui->fan2Speed4SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed5Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan2Speed5SpinBox->setValue(ui->fan2Speed5Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed5SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan2Speed5Slider->setValue(ui->fan2Speed5SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed6Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan2Speed6SpinBox->setValue(ui->fan2Speed6Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed6SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan2Speed6Slider->setValue(ui->fan2Speed6SpinBox->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed7Slider, &QSlider::valueChanged, this, [this]() {
        ui->fan2Speed7SpinBox->setValue(ui->fan2Speed7Slider->value());
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed7SpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan2Speed7Slider->setValue(ui->fan2Speed7SpinBox->value());
        checkFanSettingsChanged();
    });

    connect(ui->fan2Speed2TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        ui->fan2Speed1TempLabel->setText(QString("< %1 °C").arg(ui->fan2Speed2TempSpinBox->value()));
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed3TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed4TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed5TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed6TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        checkFanSettingsChanged();
    });
    connect(ui->fan2Speed7TempSpinBox, &QSpinBox::textChanged, this, [this]() {
        checkFanSettingsChanged();
    });

    connect(ui->fanSpeedResetButton, &QPushButton::clicked, this, &MainWindow::updateFanSpeedSettings);
    connect(ui->fanSpeedApplyButton, &QPushButton::clicked, this, &MainWindow::setFanSpeedSettings);
    connect(&powerMonitor, &PowerMonitor::currentChargerState, this, &MainWindow::on_ChargerStateChange);
    connect(&powerMonitor, &PowerMonitor::currentPowerProfile, this, &MainWindow::on_PowerProfileChange);

    connect(qApp, &QGuiApplication::saveStateRequest, this, &MainWindow::saveStateRequest);

    MainWindow::setWindowIcon(QIcon(":/images/AppIcon"));
    if (Settings::isValueExist("MainWindow/Width") && Settings::isValueExist("MainWindow/Height"))
        MainWindow::resize(Settings::getValue("MainWindow/Width").toInt(), Settings::getValue("MainWindow/Height").toInt());

    createTrayIcon();

    ui->tabWidget->tabBar()->setExpanding(true);
    // Disable debug tab
    ui->tabWidget->setTabVisible(5, false);
    setTabsEnabled(false);

    if (!operate.isEcModuleLoaded() && !operate.loadEcModule())
        QMessageBox::critical(nullptr, this->windowTitle(), tr("The ec_sys module couldn't be detected, it might be required to control the fans."));

    if(operate.updateEcData())
        updateData();

    connect(realtimeUpdateTimer, &QTimer::timeout, this, &MainWindow::realtimeUpdate);
    setUpdateInterval(1000);

    // Timer to detect sleep and reapply Advanced Mode Fan if necessary
    connect(&timerSleepWatcher, &QTimer::timeout, this, &MainWindow::timerSleepTimeout);
    timerSleepWatcher.setInterval(10 * 1000);
    timerSleepWatcher.start();

    ui->QtVersionValue->setText(QT_VERSION_STR);
    ui->versionValueLabel->setText(MControlCenter_VERSION);
    loadSettings();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::loadSettings() const {
    ui->autoAcDcProfilesGroupBox->setChecked(Settings::getValue("Settings/autoAcDcProfilesState").toBool());
    if (!Settings::isValueExist("Settings/UserModeOnBattery")) {
        Settings::setValue("Settings/UserModeOnBattery", ui->userModeOnBatteryComboBox->currentIndex());
    }
    ui->userModeOnBatteryComboBox->setCurrentIndex(Settings::getValue("Settings/UserModeOnBattery").toInt());
    if (!Settings::isValueExist("Settings/UserModeOnCharger")) {
        Settings::setValue("Settings/UserModeOnCharger", ui->userModeOnChargerComboBox->currentIndex());
    }
    ui->userModeOnChargerComboBox->setCurrentIndex(Settings::getValue("Settings/UserModeOnCharger").toInt());
    ui->autoPPDCheckBox->setChecked(Settings::getValue("Settings/autoPPDstate").toBool());

    // init the custom battery threshold
    if (!Settings::isValueExist("Settings/CustomBatteryThreshold")) {
        Settings::setValue("Settings/CustomBatteryThreshold", 50);
    }
    ui->customBatteryThresholdSpinBox->setValue(Settings::getValue("Settings/CustomBatteryThreshold").toInt());
    setCustomBatteryTrayLabel(ui->customBatteryThresholdSpinBox->value());
    if (!Settings::isValueExist("Settings/BatteryThresholdSetting")) {
        switch (operate.getBatteryThreshold()) {
            case 0:
            case 100:
                Settings::setValue("Settings/BatteryThresholdSetting", 0);
                break;
            case 80:
                Settings::setValue("Settings/BatteryThresholdSetting", 1);
                break;
            case 60:
                Settings::setValue("Settings/BatteryThresholdSetting", 2);
                break;
            default:
                Settings::setValue("Settings/BatteryThresholdSetting", 3);
                break;
        }
    }

    if (!Settings::isValueExist("Settings/TurboMode")) {
        Settings::setValue("Settings/TurboMode", false);
    }

    updateBatteryThreshold();
}

void MainWindow::setUpdateDataError(const bool error) {
    isUpdateDataError = error;
}

void MainWindow::setTabsEnabled(const bool enabled) const {
    ui->overviewTab->setEnabled(enabled);
    ui->modeFormWidget->setEnabled(enabled);
    ui->batteryTab->setEnabled(enabled);
    ui->fanControlTab->setEnabled(enabled);
    ui->keyboardTab->setEnabled(enabled);
    ui->debugTab->setEnabled(enabled);

    if (modeTrayMenu)
        modeTrayMenu->setEnabled(enabled);
    if (fanTrayMenu)
        fanTrayMenu->setEnabled(enabled);
    if (batteryTrayMenu)
        batteryTrayMenu->setEnabled(enabled);
    if (keyboardTrayMenu)
        keyboardTrayMenu->setEnabled(enabled);
    if (miscTrayMenu)
        miscTrayMenu->setEnabled(enabled);

}

void MainWindow::startRealtimeUpdate() const {
    realtimeUpdateTimer->start();
}

void MainWindow::stopRealtimeUpdate() const {
    realtimeUpdateTimer->stop();
}

void MainWindow::setUpdateInterval(const int msec) const {
    realtimeUpdateTimer->setInterval(msec);
}

void MainWindow::realtimeUpdate() const {
    operate.updateEcDataAsync();
    updateData();
}

void MainWindow::updateData() const {
    if (!isUpdateDataError && operate.isEcModuleLoaded()) {
        if (!isActive) {
            setTabsEnabled(true);
            loadConfigs();
            isActive = true;
        }
        updateBatteryCharge();
        updatePenCharge();
        updateChargingStatus();
        updateCpuTemp();
        updateGpuTemp();
        updateFanMode();
        updateFan1Speed();
        updateFan2Speed();
        updateKeyboardBrightness();
        updateKeyboardBacklightMode();
        updateTurboMode();

        if(operate.isAcpiEc()) {
            ui->EcBackendStatusLabel->setText(tr("acpi_ec"));
        } else if (operate.isEcSys()) {
            ui->EcBackendStatusLabel->setText(tr("ec_sys"));
        }
    } else {
        setTabsEnabled(false);
        ui->EcBackendStatusLabel->setText(tr("Failed to load both acpi_ec/ec_sys"));
        isActive = false;
    }
}

void MainWindow::loadConfigs() const {
    ui->ecVersionValueLabel->setText(QString::fromStdString(operate.getEcVersion()));
    ui->ecBuildValueLabel->setText(QString::fromStdString(operate.getEcBuild()));

    operate.loadSettings();
    updateUserMode();
    updateCoolerBoostState();

    updateBatteryThreshold();

    updateFanSpeedSettings();

    updateKeyboardBacklightMode();

    updateKeyboardBrightness();

    updateUsbPowerShareState();

    updateTurboMode();

    updateFnMetaSwapState();
}

QString MainWindow::intToQString(const int value) const {
    if (value < 0)
        return "-";
    return QString::number(value);
}

void MainWindow::updateBatteryCharge() const {
    ui->batteryChargeValueLabel->setText(intToQString(operate.getBatteryCharge()) + " %");
}

void MainWindow::updatePenCharge() const {
    uint8_t penChargeValue = 0;
    QFile presenceFile("/sys/class/power_supply/hid-0018:04F3:4516.0002-battery-7/present");
    QFile capacityFile("/sys/class/power_supply/hid-0018:04F3:4516.0002-battery-7/capacity");
    if (!presenceFile.open(QIODevice::ReadOnly) || presenceFile.readAll().toInt() == 0) {
        goto hide;
    }
    if (capacityFile.open(QIODevice::ReadOnly)) {
        penChargeValue = capacityFile.readAll().toInt();
    }
    if (penChargeValue > 0) {
        ui->penChargeLabel->setVisible(true);
        ui->penChargeValueLabel->setVisible(true);
        ui->penChargeValueLabel->setText(intToQString(penChargeValue) + " %");
        return;
    }
hide:
    ui->penChargeLabel->setVisible(false);
    ui->penChargeValueLabel->setVisible(false);
}

void MainWindow::updateBatteryThreshold() const {
    if (operate.updateEcData()) {
        uint8_t batteryThreshold = operate.getBatteryThreshold();

        switch (Settings::getValue("Settings/BatteryThresholdSetting").toInt()) {
            case 0:
                ui->bestMobilityRadioButton->click();
                batteryThreshold = 100;
                bestMobilityAction->setChecked(true);
                break;
            case 1:
                ui->balancedBatteryRadioButton->click();
                balancedBatteryAction->setChecked(true);
                break;
            case 2:
                ui->bestBatteryRadioButton->click();
                bestBatteryAction->setChecked(true);
                break;
            default:
                ui->customBatteryThresholdRadioButton->click();
                ui->customBatteryApplyButton->setEnabled(
                        batteryThreshold != ui->customBatteryThresholdSpinBox->value());
                setCustomBatteryTrayLabel(batteryThreshold);
                customBatteryAction->setChecked(true);
                break;
        }

        ui->batteryThresholdValueLabel->setText(QString::number(batteryThreshold) + " %");

        // ui->customBatteryThresholdSpinBox->setValue(batteryThreshold);
    }
}

void MainWindow::updateChargingStatus() const {
    QString chargingStatus;
    switch (operate.getChargingStatus()) {
        case charging_state::battery_not_charging:
            chargingStatus = tr("Not charging");
            break;
        case charging_state::battery_charging:
            chargingStatus = tr("Charging");
            break;
        case charging_state::battery_fully_charged:
            chargingStatus = tr("Fully charged");
            break;
        case charging_state::battery_discharging:
        case charging_state::battery_fully_charged_no_power:
            chargingStatus = tr("Discharging");
            break;
        default:
            chargingStatus = tr("Unknown");
            break;
    }
    ui->chargingStatusValueLabel->setText(chargingStatus);
}

void MainWindow::updateCpuTemp() const {
    ui->cpuTempValueLabel->setText(intToQString(operate.getCpuTemp()) + " °C");
}

void MainWindow::updateGpuTemp() const {
    const std::optional<uint8_t> temp = operate.getGpuTemp();
    if (!temp.has_value() || temp.value() <= 0) {
        ui->gpuTempValueLabel->setVisible(false);
        ui->gpuTempLabel->setVisible(false);
    } else {
        ui->gpuTempValueLabel->setVisible(true);
        ui->gpuTempLabel->setVisible(true);
        ui->gpuTempValueLabel->setText(intToQString(temp.value()) + " °C");
    }
}

void MainWindow::updateFan1Speed() const {
    const int speed = operate.getFan1Speed();
    if (speed > 0)
        ui->fan1ValueLabel->setText(intToQString(speed) + " " + tr("rpm"));
    else
        ui->fan1ValueLabel->setText(tr("OFF"));
}

void MainWindow::updateFan2Speed() const {
    const std::optional<int> speed = operate.getFan2Speed();
    if (speed.has_value()) {
        ui->fan2ValueLabel->setVisible(true);
        ui->fan2Label->setVisible(true);
        if (speed.value() > 0)
            ui->fan2ValueLabel->setText(intToQString(speed.value()) + " " + tr("rpm"));
        else
            ui->fan2ValueLabel->setText(tr("OFF"));
    } else {
        ui->fan2ValueLabel->setVisible(false);
        ui->fan2Label->setVisible(false);
        ui->fanControlTabWidget->setTabEnabled(1, false);
    }
}

void MainWindow::updateKeyboardBacklightMode() const {
    if (operate.updateEcData()) {
        const uint8_t index = operate.getKeyboardBacklightMode();
        ui->keyboardBacklightModeComboBox->setCurrentIndex(index);
        if (index == 0) {
            keyboardAlwaysOn->setChecked(true);
        } else if (index == 1) {
            keyboardAutoOff->setChecked(true);
        }
    }
}

void MainWindow::updateKeyboardBrightness() const {
    ui->keyboardBrightnessSlider->setSliderPosition(operate.getKeyboardBrightness());
}

void MainWindow::updateUsbPowerShareState() const {
    ui->usbPowerShareCheckBox->setChecked(operate.getUsbPowerShareState());
}

void MainWindow::updateTurboMode() const {
    const bool state = operate.getTurboMode();
    ui->turboModeCheckBox->setChecked(state);
    turboMode->setChecked(state);

    const QString text = state ? "Full Performance" : "Performance";
    ui->performanceModeRadioButton->setText(text);
    performanceMode->setText(text);
    ui->userModeOnChargerComboBox->setItemText(0, text);
    ui->userModeOnBatteryComboBox->setItemText(0, text);
}

void MainWindow::updateFnMetaSwapState() const {
    if (operate.updateEcData()) {
        const bool state = operate.getFnMetaSwapState();
        ui->fnMetaSwapCheckBox->setChecked(state);
        fnMetaSwapTrayAction->setChecked(state);
    }
}

void MainWindow::updateCoolerBoostState() const {
    if (operate.updateEcData()) {
        const bool state = operate.getCoolerBoostState();
        ui->coolerBoostCheckBox->setChecked(state);
        coolerBoostAction->setChecked(state);
    }
}

void MainWindow::updateUserMode() const {
    if (operate.updateEcData()) {
        switch (operate.getUserMode()) {
            case user_mode::balanced_mode:
                ui->balancedModeRadioButton->click();
                balancedMode->setChecked(true);
                break;
            case user_mode::performance_mode:
                ui->performanceModeRadioButton->click();
                performanceMode->setChecked(true);
                break;
            case user_mode::silent_mode:
                ui->silentModeRadioButton->click();
                silentMode->setChecked(true);
                break;
            case user_mode::eco_silent_mode:
                ui->ecoSilentModeRadioButton->click();
                ecoSilentMode->setChecked(true);
                break;
            case user_mode::unknown_mode:
                //fall to default, happens on some models after booting
            default:
                ui->ecoSilentModeRadioButton->setChecked(false);
                ui->silentModeRadioButton->setChecked(false);
                ui->balancedModeRadioButton->setChecked(false);
                ui->performanceModeRadioButton->setChecked(false);
                break;
        }
    }
}

static inline const char* getFanModeString() {
    if (operate.getCoolerBoostState())
        return "Turbotastic";
    switch (operate.getFanMode()) {
        case fan_mode::auto_fan_mode:
            return "Auto";
        case fan_mode::silent_fan_mode:
            return "Silent";
        case fan_mode::basic_fan_mode:
            return "Basic";
        case fan_mode::advanced_fan_mode:
            return "Advanced";
        default:
            return "Unknown";
    }
}

void MainWindow::updateFanMode() const {
    ui->fanModeValueLabel->setText(tr(getFanModeString()));
}

void MainWindow::updateFanSpeedSettings() const {
    ui->advancedFanControlCheckBox->setChecked(operate.getFanMode() == fan_mode::advanced_fan_mode);

    const QVector fan1SpeedSettings = operate.getFan1SpeedSettings();
    const QVector fan1TempSettings = operate.getFan1TempSettings();
    const QVector fan2SpeedSettings = operate.getFan2SpeedSettings();
    const QVector fan2TempSettings = operate.getFan2TempSettings();

    ui->fan1Speed1Slider->setValue(fan1SpeedSettings[0]);
    ui->fan1Speed2Slider->setValue(fan1SpeedSettings[1]);
    ui->fan1Speed3Slider->setValue(fan1SpeedSettings[2]);
    ui->fan1Speed4Slider->setValue(fan1SpeedSettings[3]);
    ui->fan1Speed5Slider->setValue(fan1SpeedSettings[4]);
    ui->fan1Speed6Slider->setValue(fan1SpeedSettings[5]);
    ui->fan1Speed7Slider->setValue(fan1SpeedSettings[6]);

    ui->fan1Speed1TempLabel->setText(QString("< %1 °C").arg(fan1TempSettings[0]));
    ui->fan1Speed2TempSpinBox->setValue(fan1TempSettings[0]);
    ui->fan1Speed3TempSpinBox->setValue(fan1TempSettings[1]);
    ui->fan1Speed4TempSpinBox->setValue(fan1TempSettings[2]);
    ui->fan1Speed5TempSpinBox->setValue(fan1TempSettings[3]);
    ui->fan1Speed6TempSpinBox->setValue(fan1TempSettings[4]);
    ui->fan1Speed7TempSpinBox->setValue(fan1TempSettings[5]);

    ui->fan2Speed1Slider->setValue(fan2SpeedSettings[0]);
    ui->fan2Speed2Slider->setValue(fan2SpeedSettings[1]);
    ui->fan2Speed3Slider->setValue(fan2SpeedSettings[2]);
    ui->fan2Speed4Slider->setValue(fan2SpeedSettings[3]);
    ui->fan2Speed5Slider->setValue(fan2SpeedSettings[4]);
    ui->fan2Speed6Slider->setValue(fan2SpeedSettings[5]);
    ui->fan2Speed7Slider->setValue(fan2SpeedSettings[6]);

    ui->fan2Speed1TempLabel->setText(QString("< %1 °C").arg(fan2TempSettings[0]));
    ui->fan2Speed2TempSpinBox->setValue(fan2TempSettings[0]);
    ui->fan2Speed3TempSpinBox->setValue(fan2TempSettings[1]);
    ui->fan2Speed4TempSpinBox->setValue(fan2TempSettings[2]);
    ui->fan2Speed5TempSpinBox->setValue(fan2TempSettings[3]);
    ui->fan2Speed6TempSpinBox->setValue(fan2TempSettings[4]);
    ui->fan2Speed7TempSpinBox->setValue(fan2TempSettings[5]);

    checkFanSettingsChanged();
}

void MainWindow::setBestMobility() const {
    operate.setBatteryThreshold(100);
    Settings::setValue("Settings/BatteryThresholdSetting", 0);
    updateBatteryThreshold();
}

void MainWindow::setBalancedBattery() const {
    operate.setBatteryThreshold(80);
    Settings::setValue("Settings/BatteryThresholdSetting", 1);
    updateBatteryThreshold();
}

void MainWindow::setBestBattery() const {
    operate.setBatteryThreshold(60);
    Settings::setValue("Settings/BatteryThresholdSetting", 2);
    updateBatteryThreshold();
}

void MainWindow::setCustomBattery() const {
    operate.setBatteryThreshold(Settings::getValue("Settings/CustomBatteryThreshold").toInt());
    Settings::setValue("Settings/BatteryThresholdSetting", 3);
    updateBatteryThreshold();
}

void MainWindow::setCustomBatteryTrayLabel(const uint8_t value) const {
    customBatteryAction->setText(ui->customBatteryThresholdRadioButton->text() + " (" + intToQString(value) + "%)");
}

void MainWindow::setPerformanceMode() const {
    operate.setUserMode(user_mode::performance_mode);
    updateUserMode();
}

void MainWindow::setBalancedMode() const {
    operate.setUserMode(user_mode::balanced_mode);
    updateUserMode();
}

void MainWindow::setSilentMode() const {
    operate.setUserMode(user_mode::silent_mode);
    updateUserMode();
}

void MainWindow::setEcoSilentMode() const {
    operate.setUserMode(user_mode::eco_silent_mode);
    updateUserMode();
}

void MainWindow::setCoolerBoostState(const bool enabled) const {
    operate.setCoolerBoostState(enabled);
    updateCoolerBoostState();
}

void MainWindow::setKeyboardAlwaysOn() const {
    operate.setKeyboardBacklightMode(0);
    updateKeyboardBacklightMode();
}

void MainWindow::setKeyboardAutoOff() const {
    operate.setKeyboardBacklightMode(1);
    updateKeyboardBacklightMode();
}

QVector<int> MainWindow::getFan1SpeedValues() const {
    QVector<int> fan1SpeedSettings;

    fan1SpeedSettings.push_back(ui->fan1Speed1Slider->value());
    fan1SpeedSettings.push_back(ui->fan1Speed2Slider->value());
    fan1SpeedSettings.push_back(ui->fan1Speed3Slider->value());
    fan1SpeedSettings.push_back(ui->fan1Speed4Slider->value());
    fan1SpeedSettings.push_back(ui->fan1Speed5Slider->value());
    fan1SpeedSettings.push_back(ui->fan1Speed6Slider->value());
    fan1SpeedSettings.push_back(ui->fan1Speed7Slider->value());

    return fan1SpeedSettings;
}

QVector<int> MainWindow::getFan2SpeedValues() const {
    QVector<int> fan2SpeedSettings;

    fan2SpeedSettings.push_back(ui->fan2Speed1Slider->value());
    fan2SpeedSettings.push_back(ui->fan2Speed2Slider->value());
    fan2SpeedSettings.push_back(ui->fan2Speed3Slider->value());
    fan2SpeedSettings.push_back(ui->fan2Speed4Slider->value());
    fan2SpeedSettings.push_back(ui->fan2Speed5Slider->value());
    fan2SpeedSettings.push_back(ui->fan2Speed6Slider->value());
    fan2SpeedSettings.push_back(ui->fan2Speed7Slider->value());

    return fan2SpeedSettings;
}

QVector<int> MainWindow::getFan1TempValues() const {
    QVector<int> fan1TempSettings;

    fan1TempSettings.push_back(ui->fan1Speed2TempSpinBox->value());
    fan1TempSettings.push_back(ui->fan1Speed3TempSpinBox->value());
    fan1TempSettings.push_back(ui->fan1Speed4TempSpinBox->value());
    fan1TempSettings.push_back(ui->fan1Speed5TempSpinBox->value());
    fan1TempSettings.push_back(ui->fan1Speed6TempSpinBox->value());
    fan1TempSettings.push_back(ui->fan1Speed7TempSpinBox->value());

    return fan1TempSettings;
}

QVector<int> MainWindow::getFan2TempValues() const {
    QVector<int> fan2TempSettings;

    fan2TempSettings.push_back(ui->fan2Speed2TempSpinBox->value());
    fan2TempSettings.push_back(ui->fan2Speed3TempSpinBox->value());
    fan2TempSettings.push_back(ui->fan2Speed4TempSpinBox->value());
    fan2TempSettings.push_back(ui->fan2Speed5TempSpinBox->value());
    fan2TempSettings.push_back(ui->fan2Speed6TempSpinBox->value());
    fan2TempSettings.push_back(ui->fan2Speed7TempSpinBox->value());

    return fan2TempSettings;
}

void MainWindow::setFanSpeedSettings() const {
    operate.setFan1SpeedSettings(getFan1SpeedValues());
    operate.setFan1TempSettings(getFan1TempValues());
    operate.setFan2SpeedSettings(getFan2SpeedValues());
    operate.setFan2TempSettings(getFan2TempValues());
    if (operate.updateEcData())
        checkFanSettingsChanged();
}

void MainWindow::setFanModeAdvanced(const bool enabled) const {
    operate.setFanModeAdvanced(enabled);
    ui->fanControlTabWidget->setEnabled(enabled);
    ui->fanSpeedResetButton->setEnabled(enabled);
    ui->fanSpeedApplyButton->setEnabled(enabled);
}

void MainWindow::checkFanSettingsChanged() const {
    const bool fanSettingChanged = (getFan1SpeedValues() != operate.getFan1SpeedSettings() ||
                                    getFan2SpeedValues() != operate.getFan2SpeedSettings() ||
                                    getFan1TempValues() != operate.getFan1TempSettings() ||
                                    getFan2TempValues() != operate.getFan2TempSettings());
    ui->fanSpeedApplyButton->setEnabled(fanSettingChanged);
    ui->fanSpeedResetButton->setEnabled(fanSettingChanged);
}

void MainWindow::showEvent(QShowEvent *event) {
    startRealtimeUpdate();
    activateWindow();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (trayIcon && trayIcon->isVisible()) {
        stopRealtimeUpdate();
        MainWindow::hide();
        event->ignore();
        return;
    }
    quitApp();
}

void MainWindow::quitApp() const {
    operate.closeHelperApp();
    Settings::setValue("MainWindow/Width", MainWindow::width());
    Settings::setValue("MainWindow/Height", MainWindow::height());
    (void) QCoreApplication::quit();
}

void MainWindow::timerSleepTimeout() {
    const qint64 timeNow = QDateTime::currentMSecsSinceEpoch();
    if (timeLastWatcherInterval == 0) {
        timeLastWatcherInterval = timeNow;
        return;
    }
    const qint64 msecsSinceTimeout = timeNow - timeLastWatcherInterval;
    timeLastWatcherInterval = timeNow;
    if (msecsSinceTimeout > timerSleepWatcher.interval() + 5000) {
        // Went to sleep for at least 5 seconds
        operate.handleWakeEvent();
    }
}

void MainWindow::setModeFromSelection(const PowerProfile profile) const {
    switch (profile) {
        case PowerProfile::Performance:
            setPerformanceMode();
            break;
        case PowerProfile::Balanced:
            setBalancedMode();
            break;
        case PowerProfile::Silent:
            setSilentMode();
            break;
        case PowerProfile::PowerSaver:
            setEcoSilentMode();
            break;
        case PowerProfile::Unknown:
        default:;
    }
}

void MainWindow::on_ChargerStateChange(const bool isCharging) const {
    if (ui->autoAcDcProfilesGroupBox->isChecked()) {
        const uint8_t SelectedModeOnBattery = Settings::getValue("Settings/UserModeOnBattery").toInt();
        const uint8_t SelectedModeOnCharger = Settings::getValue("Settings/UserModeOnCharger").toInt();

        const PowerProfile batteryProfile = static_cast<PowerProfile>(SelectedModeOnBattery);
        const PowerProfile chargerProfile = static_cast<PowerProfile>(SelectedModeOnCharger);

        if (isCharging) {
            setModeFromSelection(chargerProfile);
        } else {
            setModeFromSelection(batteryProfile);
        }
    } else {
        ui->autoPPDCheckBox->setEnabled(true);
    }
}

void MainWindow::on_PowerProfileChange(const PowerProfile profile) const {
    if (ui->autoPPDCheckBox->isChecked()) {
        switch (profile) {
        case PowerProfile::Performance:
            setPerformanceMode();
            ui->performanceModeRadioButton->setChecked(true);
            break;
        case PowerProfile::Balanced:
            setBalancedMode();
            ui->balancedModeRadioButton->setChecked(true);
            break;
        case PowerProfile::PowerSaver:
            setEcoSilentMode();
            ui->ecoSilentModeRadioButton->setChecked(true);
            break;
        case PowerProfile::Unknown:
            default:;
        }
    }
}

void MainWindow::on_bestMobilityRadioButton_toggled(const bool checked) const {
    if (checked)
        setBestMobility();
}

void MainWindow::on_balancedBatteryRadioButton_toggled(const bool checked) const {
    if (checked)
        setBalancedBattery();
}

void MainWindow::on_bestBatteryRadioButton_toggled(const bool checked) const {
    if (checked)
        setBestBattery();
}

void MainWindow::on_customBatteryThresholdRadioButton_toggled(const bool checked) const {
    if (checked) {
        ui->customBatteryThresholdSpinBox->setEnabled(true);
        setCustomBattery();
    } else {
        ui->customBatteryThresholdSpinBox->setDisabled(true);
        ui->customBatteryApplyButton->setDisabled(true);
    }
}

void MainWindow::on_customBatteryThresholdSpinBox_valueChanged(const int arg1) const {
    ui->customBatteryApplyButton->setEnabled(
            ui->customBatteryThresholdRadioButton->isChecked() && operate.getBatteryThreshold() != arg1);
}

void MainWindow::on_customBatteryApplyButton_clicked() const {
    Settings::setValue("Settings/CustomBatteryThreshold", ui->customBatteryThresholdSpinBox->value());
    setCustomBattery();
}

void MainWindow::on_clearSettingsButton_clicked() const {
    if (QMessageBox::Yes == QMessageBox(QMessageBox::NoIcon, this->windowTitle(), tr("Do you really want to clear your settings?"), QMessageBox::Yes|QMessageBox::No).exec()) {
        clearSettings();
    }
}

void MainWindow::clearSettings() const {
    Settings::clear();
    loadSettings();
}

void MainWindow::on_ReadValueButton_clicked() const {
    const QString text = ui->addressEdit->displayText();
    uint8_t value = operate.getValue(text.toInt());
    ui->ValueSpinBox->setValue(value);
}

void MainWindow::on_WriteValueButton_clicked() const {
    const QString text = ui->addressEdit->displayText();
    uint8_t address = text.toInt();
    operate.setValue(address, ui->ValueSpinBox->value());
}

void MainWindow::on_usbPowerShareCheckBox_clicked(const bool checked) const {
    ui->usbPowerShareCheckBox->setChecked(checked);
    usbPowerShareTrayAction->setChecked(checked);
    operate.setUsbPowerShareState(checked);
}

void MainWindow::on_turboModeCheckBox_clicked(const bool checked) const {
    operate.setTurboMode(checked);
    updateTurboMode();
}

void MainWindow::on_fnMetaSwapCheckBox_clicked(const bool checked) const {
    operate.setFnMetaSwapState(checked);
    updateFnMetaSwapState();
}

void MainWindow::on_coolerBoostCheckBox_clicked(const bool checked) const {
    if (operate.getCoolerBoostState() != checked)
        setCoolerBoostState(checked);
}

void MainWindow::on_keyboardBrightnessSlider_valueChanged(const int value) const {
    operate.setKeyboardBrightness(value);
    if (operate.updateEcData()) {
        updateKeyboardBrightness();
    }
}

void MainWindow::on_keyboardBacklightModeComboBox_currentIndexChanged(const int index) const {
    if (index == 0) {
        setKeyboardAlwaysOn();
    } else if (index == 1) {
        setKeyboardAutoOff();
    }
}

void MainWindow::on_userModeOnBatteryComboBox_currentIndexChanged(const int index) const {
    Settings::setValue("Settings/UserModeOnBattery", index);
    powerMonitor.queryChargerState();
}

void MainWindow::on_userModeOnChargerComboBox_currentIndexChanged(const int index) const {
    Settings::setValue("Settings/UserModeOnCharger", index);
    powerMonitor.queryChargerState();
}

void MainWindow::on_autoAcDcProfilesGroupBox_toggled(const bool checked) const {
    if(checked) {
        if (!powerMonitor.connectToUpower()) {
            QMessageBox::critical(nullptr, this->windowTitle(), tr("Couldn't connect to UPower to get charger status.\n"
                                                                   "Make sure that UPower is installed and running then restart the system."));
            ui->autoAcDcProfilesGroupBox->setChecked(false);
            ui->autoAcDcProfilesGroupBox->setDisabled(true);
            return;
        }

        ui->autoAcDcProfilesGroupBox->setChecked(true);
        autoAcDcMode->setChecked(true);

        powerMonitor.disconnectFromPowerProfiles();
        ui->autoPPDCheckBox->setChecked(false);
        ui->autoPPDCheckBox->setDisabled(true);
        autoPPDMode->setChecked(false);
        autoPPDMode->setVisible(false);
        powerMonitor.queryChargerState();
    } else {
        ui->autoAcDcProfilesGroupBox->setChecked(false);
        autoAcDcMode->setChecked(false);

        ui->autoPPDCheckBox->setEnabled(true);
        autoPPDMode->setVisible(true);
        powerMonitor.disconnectFromUpower();
    }

    Settings::setValue("Settings/autoAcDcProfilesState", checked);
}

void MainWindow::on_autoPPDCheckBox_toggled(const bool checked) const {
    if (checked) {
        if (!powerMonitor.connectToPowerProfiles()) {
            QMessageBox::critical(nullptr, this->windowTitle(), tr("Couldn't connect to Power Profiles Daemon.\n"
                                                                   "Make sure that either Power Profiles Daemon or TuneD is installed and restart the system."));
            ui->autoPPDCheckBox->setChecked(false);
            return;
        }
        ui->autoPPDCheckBox->setChecked(true);
        autoPPDMode->setChecked(true);

        powerMonitor.disconnectFromUpower();
        ui->performanceModeRadioButton->setDisabled(true);
        ui->balancedModeRadioButton->setDisabled(true);
        ui->silentModeRadioButton->setDisabled(true);
        ui->ecoSilentModeRadioButton->setDisabled(true);

        ui->autoAcDcProfilesGroupBox->setChecked(false);
        ui->autoAcDcProfilesGroupBox->setDisabled(true);
        autoAcDcMode->setChecked(false);
        autoAcDcMode->setVisible(false);
        if (modeTrayActions)
            modeTrayActions->setVisible(false);
        powerMonitor.queryPowerProfile();
    } else {
        ui->autoPPDCheckBox->setChecked(false);
        autoPPDMode->setChecked(false);

        ui->performanceModeRadioButton->setEnabled(true);
        ui->balancedModeRadioButton->setEnabled(true);
        ui->silentModeRadioButton->setEnabled(true);
        ui->ecoSilentModeRadioButton->setEnabled(true);

        ui->autoAcDcProfilesGroupBox->setEnabled(true);
        autoAcDcMode->setVisible(true);
        if (modeTrayActions)
            modeTrayActions->setVisible(true);
    }
    Settings::setValue("Settings/autoPPDstate", checked);
}

void MainWindow::on_performanceModeRadioButton_toggled(const bool checked) const {
    if (checked)
        setPerformanceMode();
}

void MainWindow::on_balancedModeRadioButton_toggled(const bool checked) const {
    if (checked)
        setBalancedMode();
}

void MainWindow::on_silentModeRadioButton_toggled(const bool checked) const {
    if (checked)
        setSilentMode();
}

void MainWindow::on_ecoSilentModeRadioButton_toggled(const bool checked) const {
    if (checked)
        setEcoSilentMode();
}

void MainWindow::iconActivated(const QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            if (!MainWindow::isHidden()) {
                MainWindow::close();
            } else {
                MainWindow::showNormal();
            }
            break;
        case QSystemTrayIcon::MiddleClick:
            break;
        default:;
    }
}

void MainWindow::createTrayIcon() {
    createActions();

    modeTrayMenu = new QMenu(tr("Mode    "));
    modeTrayActions = new QActionGroup(this);
    modeTrayActions->setExclusive(true);

    modeTrayActions->addAction(performanceMode);
    modeTrayActions->addAction(balancedMode);
    modeTrayActions->addAction(silentMode);
    modeTrayActions->addAction(ecoSilentMode);

    modeTrayMenu->addAction(autoPPDMode);
    modeTrayMenu->addAction(autoAcDcMode);
    modeTrayMenu->addSeparator();
    modeTrayMenu->addAction(turboMode);
    modeTrayMenu->addSeparator();
    modeTrayMenu->addAction(performanceMode);
    modeTrayMenu->addAction(balancedMode);
    modeTrayMenu->addAction(silentMode);
    modeTrayMenu->addAction(ecoSilentMode);

    fanTrayMenu = new QMenu(tr("Cooling    "));
    fanTrayMenu->addAction(coolerBoostAction);


    batteryTrayMenu = new QMenu(tr("Battery    "));
    batteryTrayActions = new QActionGroup(this);
    batteryTrayActions->setExclusive(true);

    batteryTrayMenu->addAction(bestMobilityAction);
    batteryTrayMenu->addAction(balancedBatteryAction);
    batteryTrayMenu->addAction(bestBatteryAction);
    batteryTrayMenu->addAction(customBatteryAction);

    batteryTrayActions->addAction(bestMobilityAction);
    batteryTrayActions->addAction(balancedBatteryAction);
    batteryTrayActions->addAction(bestBatteryAction);
    batteryTrayActions->addAction(customBatteryAction);


    keyboardTrayMenu = new QMenu(tr("Keyboard    "));
    keyboardTrayActions = new QActionGroup(this);
    keyboardTrayActions->setExclusive(true);

    keyboardTrayMenu->addAction(keyboardAlwaysOn);
    keyboardTrayMenu->addAction(keyboardAutoOff);

    keyboardTrayActions->addAction(keyboardAlwaysOn);
    keyboardTrayActions->addAction(keyboardAutoOff);


    miscTrayMenu = new QMenu(tr("Misc    "));
    miscTrayMenu->addAction(usbPowerShareTrayAction);
    miscTrayMenu->addAction(fnMetaSwapTrayAction);


    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addMenu(modeTrayMenu);
    trayIconMenu->addMenu(fanTrayMenu);
    trayIconMenu->addMenu(batteryTrayMenu);
    trayIconMenu->addMenu(keyboardTrayMenu);
    trayIconMenu->addMenu(miscTrayMenu);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    const auto icon = QIcon(":/images/AppIcon");
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("MControlCenter"); //TODO

    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);
}

void MainWindow::createActions() {
    restoreAction = new QAction(tr("Show    "), this);
    connect(restoreAction, &QAction::triggered, this, &MainWindow::showNormal);

    autoPPDMode = new QAction(tr("System Profile"), this);
    autoPPDMode->setCheckable(true);

    autoAcDcMode = new QAction(tr("AC|DC based"), this);
    autoAcDcMode->setCheckable(true);

    turboMode = new QAction(ui->turboModeCheckBox->text(), this);
    turboMode->setCheckable(true);

    performanceMode = new QAction(ui->performanceModeRadioButton->text(), this);
    performanceMode->setCheckable(true);

    balancedMode = new QAction(ui->balancedModeRadioButton->text(), this);
    balancedMode->setCheckable(true);

    silentMode = new QAction(ui->silentModeRadioButton->text(), this);
    silentMode->setCheckable(true);

    ecoSilentMode = new QAction(ui->ecoSilentModeRadioButton->text(), this);
    ecoSilentMode->setCheckable(true);

    connect(autoPPDMode, &QAction::triggered, this, &MainWindow::on_autoPPDCheckBox_toggled);
    connect(autoAcDcMode, &QAction::triggered, this, &MainWindow::on_autoAcDcProfilesGroupBox_toggled);
    connect(turboMode, &QAction::triggered, this, &MainWindow::on_turboModeCheckBox_clicked);
    connect(performanceMode, &QAction::triggered, this, &MainWindow::setPerformanceMode);
    connect(balancedMode, &QAction::triggered, this, &MainWindow::setBalancedMode);
    connect(silentMode, &QAction::triggered, this, &MainWindow::setSilentMode);
    connect(ecoSilentMode, &QAction::triggered, this, &MainWindow::setEcoSilentMode);

    coolerBoostAction = new QAction(ui->coolerBoostCheckBox->text(), this);
    coolerBoostAction->setCheckable(true);

    connect(coolerBoostAction, &QAction::triggered, this, &MainWindow::setCoolerBoostState);


    bestMobilityAction = new QAction(ui->bestMobilityRadioButton->text() + " (100%)", this);
    bestMobilityAction->setCheckable(true);

    balancedBatteryAction = new QAction(ui->balancedBatteryRadioButton->text() + " (80%)", this);
    balancedBatteryAction->setCheckable(true);

    bestBatteryAction = new QAction(ui->bestBatteryRadioButton->text() + " (60%)", this);
    bestBatteryAction->setCheckable(true);

    customBatteryAction = new QAction(ui->customBatteryThresholdRadioButton->text(), this);
    customBatteryAction->setCheckable(true);

    connect(bestMobilityAction, &QAction::triggered, this, &MainWindow::setBestMobility);
    connect(balancedBatteryAction, &QAction::triggered, this, &MainWindow::setBalancedBattery);
    connect(bestBatteryAction, &QAction::triggered, this, &MainWindow::setBestBattery);
    connect(customBatteryAction, &QAction::triggered, this, &MainWindow::setCustomBattery);


    keyboardAlwaysOn = new QAction(tr("Always on"), this);
    keyboardAlwaysOn->setCheckable(true);

    keyboardAutoOff = new QAction(tr("Auto off"), this);
    keyboardAutoOff->setCheckable(true);

    connect(keyboardAlwaysOn, &QAction::triggered, this, &MainWindow::setKeyboardAlwaysOn);
    connect(keyboardAutoOff, &QAction::triggered, this, &MainWindow::setKeyboardAutoOff);


    usbPowerShareTrayAction = new QAction(tr("USB Power Share"), this);
    usbPowerShareTrayAction->setCheckable(true);

    fnMetaSwapTrayAction = new QAction(tr("FN ⇄ Meta"), this);
    fnMetaSwapTrayAction->setCheckable(true);

    connect(usbPowerShareTrayAction, &QAction::triggered, this, &MainWindow::on_usbPowerShareCheckBox_clicked);
    connect(fnMetaSwapTrayAction, &QAction::triggered, this, &MainWindow::on_fnMetaSwapCheckBox_clicked);


    quitAction = new QAction(tr("Quit    "), this);
    connect(quitAction, &QAction::triggered, this, &MainWindow::quitApp);
}

void MainWindow::saveStateRequest(QSessionManager &sessionManager) const {
    sessionManager.setRestartHint(QSessionManager::RestartNever);
}
