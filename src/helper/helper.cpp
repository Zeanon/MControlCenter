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

#include "helper.h"
#include "readwrite.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QProcess>
#include <QTimer>

ReadWrite rw;

void Helper::quit() const {
    QTimer::singleShot(0, QCoreApplication::instance(), &QCoreApplication::quit);
}

QByteArray Helper::getData() const {
    return rw.readFromFile();
}

void Helper::putValue(const int &address, const int &value) const {
    if (value < 0 || value >> 8) {
        fprintf(stderr, "Tried to input invalid value. Address: %d, value: %d\n", address, value);  
    }
    rw.writeToFile(address, value);
}

bool Helper::isEcModuleLoaded() const {
    const bool isAcpiEc = rw.isAcpiEc();
    if (rw.useAcpiEc()) {
        if (!isAcpiEc)
            fprintf(stderr, "%s\n", qPrintable("The acpi_ec kernel module is loaded"));
        return true;
    }
    const bool isEcSys = rw.isEcSys();
    if (rw.useEcSys()) {
        if (!isEcSys)
            fprintf(stderr, "%s\n", qPrintable("The ec_sys kernel module is loaded"));
        return true;
    }
    if (isAcpiEc || isEcSys)
        fprintf(stderr, "%s\n", qPrintable("The acpi_ec and ec_sys kernel modulee are not loaded"));
    return false;
}

bool Helper::isAcpiEc() const {
    return rw.isAcpiEc();
}

bool Helper::isEcSys() const {
    return rw.isEcSys();
}

bool Helper::loadEcModule() const {
    fprintf(stderr, "%s\n", qPrintable("Trying to load the ec_sys kernel module"));
    auto *process = new QProcess();
    process->start("sh", QStringList() << "-c" << "/usr/sbin/modprobe ec_sys write_support=1 2>&1");
    process->waitForFinished(1000);
    if (QByteArray output = process->readAllStandardOutput(); output != "")
        fprintf(stderr, "%s", qPrintable(output));
    if (isEcModuleLoaded())
        return true;
    return false;
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QObject obj;
    auto *helper = new Helper(&obj);
    QObject::connect(&a, &QCoreApplication::aboutToQuit, helper, &Helper::aboutToQuit);
    helper->setProperty("value", "initial value");
    QDBusConnection::systemBus().registerObject("/", &obj);

    if (!QDBusConnection::systemBus().registerService(SERVICE_NAME)) {
        fprintf(stderr, "%s\n", qPrintable(QDBusConnection::systemBus().lastError().message()));
        exit(1);
    }

    return QCoreApplication::exec();
}
