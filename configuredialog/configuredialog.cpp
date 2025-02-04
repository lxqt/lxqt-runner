/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#include "configuredialog.h"
#include "ui_configuredialog.h"

#include <LXQt/Globals>
#include <LXQt/Settings>

#include <QSettings>
#include <QGuiApplication>
#include <QComboBox>
#include <QDebug>
#include <QKeySequence>
#include <QPushButton>
#include <QCloseEvent>
#include <QAction>
#include <QScreen>


/************************************************

 ************************************************/
ConfigureDialog::ConfigureDialog(QSettings *settings, const QString &defaultShortcut, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog),
    mSettings(settings),
    mOldSettings(new LXQt::SettingsCache(settings)),
    mDefaultShortcut(defaultShortcut)
{
    ui->setupUi(this);

    connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &ConfigureDialog::resetSettings);

    // Position .................................
    ui->positionCbx->addItem(tr("Top edge of the screen"), QVariant(ConfigureDialog::PositionTop));
    ui->positionCbx->addItem(tr("Center of the screen"), QVariant(ConfigureDialog::PositionCenter));
    connect(ui->positionCbx, &QComboBox::currentIndexChanged, this, &ConfigureDialog::positionCbxChanged);

    // Monitor ..................................

    ui->monitorCbx->addItem(tr("Focused screen"), QVariant(-1));

    const int monCnt = QGuiApplication::screens().size();
    for (int i = 0; i < monCnt; ++i)
        ui->monitorCbx->addItem(tr("Always on screen %1").arg(i + 1), QVariant(i));

    ui->monitorCbx->setEnabled(monCnt > 1);
    connect(ui->monitorCbx, &QComboBox::currentIndexChanged, this, &ConfigureDialog::monitorCbxChanged);


    // Shortcut .................................
    connect(ui->shortcutEd, &ShortcutSelector::shortcutGrabbed, this, &ConfigureDialog::shortcutChanged);

    connect(ui->shortcutEd->addMenuAction(tr("Reset")), &QAction::triggered, this, &ConfigureDialog::shortcutReset);

    settingsChanged();

    connect(ui->historyUseCb, &QAbstractButton::toggled, this, [this] (bool checked) { mSettings->setValue(QL1S("dialog/history_use"), checked); });
    connect(ui->historyFirstCb, &QAbstractButton::toggled, this, [this] (bool checked) { mSettings->setValue(QL1S("dialog/history_first"), checked); });
    connect(ui->clearCb, &QAbstractButton::toggled, this, [this] (bool checked) { mSettings->setValue(QL1S("dialog/clear_on_running"), checked); });
    connect(ui->marginSB, &QAbstractSpinBox::editingFinished, this, [this] { mSettings->setValue(QL1S("dialog/top_margin"), ui->marginSB->value()); });
    connect(ui->listShownItemsSB, &QAbstractSpinBox::editingFinished, this, [this] { mSettings->setValue(QL1S("dialog/list_shown_items"), ui->listShownItemsSB->value()); });
}


/************************************************

 ************************************************/
void ConfigureDialog::settingsChanged()
{
    if (mSettings->value(QL1S("dialog/show_on_top"), true).toBool())
    {
        ui->positionCbx->setCurrentIndex(0);
        ui->marginSB->setEnabled(true);
    }
    else
    {
        ui->positionCbx->setCurrentIndex(1);
        ui->marginSB->setEnabled(false);
    }

    if (QScreen *s = QApplication::primaryScreen())
        ui->marginSB->setMaximum(s->availableGeometry().height());
    ui->marginSB->setValue(mSettings->value(QL1S("dialog/top_margin"), 0).toInt());

    const auto screens = QGuiApplication::screens();
    if (QGuiApplication::platformName() == QSL("wayland"))
    {
        QString screenName = mSettings->value(QL1S("dialog/screen_name")).toString();
        int waylandMonitor = -1;
        for (int i = 0; i < screens.size(); ++i)
        {
            if (screens.at(i)->name() == screenName)
            {
                waylandMonitor = i;
                break;
            }
        }
        ui->monitorCbx->setCurrentIndex(waylandMonitor + 1);
    }
    else
    {
        int X11Monitor = mSettings->value(QL1S("dialog/monitor"), -1).toInt();
        if (X11Monitor >= screens.size())
        {
            X11Monitor = -1;
        }
        ui->monitorCbx->setCurrentIndex(X11Monitor + 1);
    }

    ui->shortcutEd->setText(mSettings->value(QL1S("dialog/shortcut"), QL1S("Alt+F2")).toString());
    const bool history_use = mSettings->value(QL1S("dialog/history_use"), true).toBool();
    ui->historyUseCb->setChecked(history_use);
    ui->historyFirstCb->setChecked(mSettings->value(QL1S("dialog/history_first"), true).toBool());
    ui->historyFirstCb->setEnabled(history_use);
    ui->clearCb->setChecked(mSettings->value(QL1S("dialog/clear_on_running"), true).toBool());
    ui->listShownItemsSB->setValue(mSettings->value(QL1S("dialog/list_shown_items"), 4).toInt());
}


/************************************************

 ************************************************/
ConfigureDialog::~ConfigureDialog()
{
    delete mOldSettings;
    delete ui;
}


/************************************************

 ************************************************/
void ConfigureDialog::shortcutChanged(const QString &text)
{
    ui->shortcutEd->setText(text);
    mSettings->setValue(QL1S("dialog/shortcut"), text);
}


/************************************************

 ************************************************/
void ConfigureDialog::shortcutReset()
{
    shortcutChanged(mDefaultShortcut);
}


/************************************************

 ************************************************/
void ConfigureDialog::positionCbxChanged(int index)
{
    mSettings->setValue(QL1S("dialog/show_on_top"), index == 0);
    ui->marginSB->setEnabled(index == 0);
}


/************************************************

 ************************************************/
void ConfigureDialog::monitorCbxChanged(int index)
{
    if (QGuiApplication::platformName() == QSL("wayland"))
    {
        const auto screens = QGuiApplication::screens();
        if (index > 0 && index <= screens.size())
        {
            // only for Wayland
            mSettings->setValue(QL1S("dialog/screen_name"), screens.at(index - 1)->name());
        }
        else
        {
            // for both Wayland and X11 (on the focused screen)
            mSettings->remove(QL1S("dialog/screen_name"));
            mSettings->setValue(QL1S("dialog/monitor"), -1);
        }
    }
    else
    {
        if (index == 0)
        {
            // for both Wayland and X11 (on the focused screen)
            mSettings->remove(QL1S("dialog/screen_name"));
        }
        mSettings->setValue(QL1S("dialog/monitor"), index - 1);
    }
}


/************************************************

 ************************************************/
void ConfigureDialog::resetSettings()
{
    mOldSettings->loadToSettings();
    ui->shortcutEd->clear();
    settingsChanged();
}
