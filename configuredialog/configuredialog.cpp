/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
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

#include <LXQt/Settings>

#include <QSettings>
#include <QDesktopWidget>
#include <QComboBox>
#include <QDebug>
#include <QKeySequence>
#include <QPushButton>
#include <QCloseEvent>
#include <QAction>



const QKeySequence ConfigureDialog::DEFAULT_SHORTCUT{Qt::ALT + Qt::Key_F2};
/************************************************

 ************************************************/
ConfigureDialog::ConfigureDialog(QSettings *settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog),
    mSettings(settings),
    mOldSettings(new LXQt::SettingsCache(settings))
{
    ui->setupUi(this);

    connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(resetSettings()));

    // Position .................................
    ui->positionCbx->addItem(tr("Top edge of the screen"), QVariant(ConfigureDialog::PositionTop));
    ui->positionCbx->addItem(tr("Center of the screen"), QVariant(ConfigureDialog::PositionCenter));
    connect(ui->positionCbx, SIGNAL(currentIndexChanged(int)), this, SLOT(positionCbxChanged(int)));

    // Monitor ..................................
    QDesktopWidget *desktop = qApp->desktop();

    ui->monitorCbx->addItem(tr("Focused screen"), QVariant(-1));

    int monCnt = desktop->screenCount();
    for (int i = 0; i < monCnt; ++i)
        ui->monitorCbx->addItem(tr("Always on screen %1").arg(i + 1), QVariant(i));

    ui->monitorCbx->setEnabled(monCnt > 1);
    connect(ui->monitorCbx, SIGNAL(currentIndexChanged(int)), this, SLOT(monitorCbxChanged(int)));


    // Shortcut .................................
    connect(ui->shortcutEd, &QKeySequenceEdit::editingFinished, this, &ConfigureDialog::shortcutChanged);

    //TODO:?
    //connect(ui->shortcutEd->addMenuAction(tr("Reset")), SIGNAL(triggered()), this, SLOT(shortcutReset()));

    settingsChanged();

    connect(ui->historyCb, &QAbstractButton::toggled, [this] (bool checked) { mSettings->setValue("dialog/history_first", checked); });
}


/************************************************

 ************************************************/
void ConfigureDialog::settingsChanged()
{
    if (mSettings->value("dialog/show_on_top", true).toBool())
        ui->positionCbx->setCurrentIndex(0);
    else
        ui->positionCbx->setCurrentIndex(1);

    ui->monitorCbx->setCurrentIndex(mSettings->value("dialog/monitor", -1).toInt() + 1);
    ui->shortcutEd->setKeySequence(mSettings->value("dialog/shortcut", DEFAULT_SHORTCUT).toString());
    ui->historyCb->setChecked(mSettings->value("dialog/history_first", true).toBool());
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
void ConfigureDialog::shortcutChanged()
{
    QKeySequence shortcut = ui->shortcutEd->keySequence();
    if (shortcut.isEmpty())
        shortcut = mSettings->value("dialog/shortcut", DEFAULT_SHORTCUT).toString();
    else
    {
        // use only the first key (+modifiers)
        if (shortcut.count() > 1)
            shortcut = shortcut[0];

        const int modifiers = shortcut[0] & Qt::MODIFIER_MASK;
        const int key = shortcut[0] & ~Qt::MODIFIER_MASK;
        // do not allow plain printable keys
        if (modifiers
                && !(modifiers == Qt::SHIFT && key > Qt::Key_Space && key <= Qt::Key_AsciiTilde))
        {
            mSettings->setValue("dialog/shortcut", shortcut);
        } else
        {
            shortcut = mSettings->value("dialog/shortcut", DEFAULT_SHORTCUT).toString();
        }
    }
    ui->shortcutEd->setKeySequence(shortcut);
    ui->shortcutEd->clearFocus();
}


/************************************************

 ************************************************/
void ConfigureDialog::shortcutReset()
{
    ui->shortcutEd->setKeySequence(DEFAULT_SHORTCUT);
    shortcutChanged();
}


/************************************************

 ************************************************/
void ConfigureDialog::positionCbxChanged(int index)
{
    mSettings->setValue("dialog/show_on_top", index == 0);
}


/************************************************

 ************************************************/
void ConfigureDialog::monitorCbxChanged(int index)
{
    mSettings->setValue("dialog/monitor", index - 1);
}


/************************************************

 ************************************************/
void ConfigureDialog::resetSettings()
{
    mOldSettings->loadToSettings();
    ui->shortcutEd->clear();
    settingsChanged();
}
