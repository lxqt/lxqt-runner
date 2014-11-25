/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *   Petr Vanek <petr@scribus.info>
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

#include "dialog.h"
#include "ui_dialog.h"
#include "widgets.h"
#include "commanditemmodel.h"
#include "configuredialog/configuredialog.h"

#include <LXQt/Settings>
#include <LXQt/HtmlDelegate>
#include <XdgIcon>
#include <LXQt/PowerManager>
#include <LXQt/ScreenSaver>
#include <LXQtGlobalKeys/Action>
#include <LXQtGlobalKeys/Client>
#include <QDebug>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QProcess>
#include <QLineEdit>
#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QMenu>
#include <QWindow>
#include <QScrollBar>

#include <KF5/KWindowSystem/KWindowSystem>

#define DEFAULT_SHORTCUT "Alt+F2"

/************************************************

 ************************************************/
Dialog::Dialog(QWidget *parent) :
    QDialog(parent, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint),
    ui(new Ui::Dialog),
    mSettings(new LxQt::Settings("lxqt-runner", this)),
    mGlobalShortcut(0),
    mLockCascadeChanges(false),
    mConfigureDialog(0)
{
    ui->setupUi(this);
    setWindowTitle("LXQt Runner");
    setAttribute(Qt::WA_TranslucentBackground);

    connect(LxQt::Settings::globalSettings(), SIGNAL(iconThemeChanged()), this, SLOT(update()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(hide()));
    connect(mSettings, SIGNAL(settingsChanged()), this, SLOT(applySettings()));

    ui->commandEd->installEventFilter(this);

    connect(ui->commandEd, SIGNAL(textChanged(QString)), this, SLOT(setFilter(QString)));
    connect(ui->commandEd, SIGNAL(returnPressed()), this, SLOT(runCommand()));

    mCommandItemModel = new CommandItemModel(this);
    ui->commandList->installEventFilter(this);
    ui->commandList->setModel(mCommandItemModel);
    ui->commandList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->commandList, SIGNAL(clicked(QModelIndex)), this, SLOT(runCommand()));
    setFilter("");
    dataChanged();

    ui->commandList->setItemDelegate(new LxQt::HtmlDelegate(QSize(32, 32), ui->commandList));

    // Popup menu ...............................
    QAction *a = new QAction(XdgIcon::fromTheme("configure"), tr("Configure"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(showConfigDialog()));
    addAction(a);

    a = new QAction(XdgIcon::fromTheme("edit-clear-history"), tr("Clear History"), this);
    connect(a, SIGNAL(triggered()), mCommandItemModel, SLOT(clearHistory()));
    addAction(a);

    mPowerManager = new LxQt::PowerManager(this);
    addActions(mPowerManager->availableActions());
    mScreenSaver = new LxQt::ScreenSaver(this);
    addActions(mScreenSaver->availableActions());

    setContextMenuPolicy(Qt::ActionsContextMenu);

    QMenu *menu = new QMenu(this);
    menu->addActions(actions());
    ui->actionButton->setMenu(menu);
    ui->actionButton->setIcon(XdgIcon::fromTheme("configure"));
    // End of popup menu ........................

    applySettings();

    connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), SLOT(realign()));
    connect(QApplication::desktop(), SIGNAL(workAreaResized(int)), SLOT(realign()));
    connect(mGlobalShortcut, SIGNAL(activated()), this, SLOT(showHide()));
    connect(mGlobalShortcut, SIGNAL(shortcutChanged(QString,QString)), this, SLOT(shortcutChanged(QString,QString)));

    resize(mSettings->value("dialog/width", 400).toInt(), size().height());

    // TEST
    connect(mCommandItemModel, SIGNAL(layoutChanged()), this, SLOT(dataChanged()));
}


/************************************************

 ************************************************/
Dialog::~Dialog()
{
    delete ui;
}


/************************************************

 ************************************************/
void Dialog::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}


/************************************************

 ************************************************/
QSize Dialog::sizeHint() const
{
    QSize size = QDialog::sizeHint();
    size.setWidth(this->size().width());
    return size;
}


/************************************************

 ************************************************/
void Dialog::resizeEvent(QResizeEvent *event)
{
    mSettings->setValue("dialog/width", size().width());
}


/************************************************

 ************************************************/
bool Dialog::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (object == ui->commandEd)
            return editKeyPressEvent(keyEvent);

        if (object == ui->commandList)
            return listKeyPressEvent(keyEvent);
    }
    else if (event->type() == QEvent::FocusOut)
    {
        hide();
        return true;
    }

    return QDialog::eventFilter(object, event);
}


/************************************************
 eventFilter for ui->commandEd
 ************************************************/
bool Dialog::editKeyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_N:
        if (event->modifiers().testFlag(Qt::ControlModifier))
        {
            QKeyEvent ev(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
            editKeyPressEvent(&ev);
            return true;
        }
            return false;

    case Qt::Key_P:
        if (event->modifiers().testFlag(Qt::ControlModifier))
        {
            QKeyEvent ev(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
            editKeyPressEvent(&ev);
            return true;
        }
            return false;

    case Qt::Key_Up:
    case Qt::Key_PageUp:
        if (ui->commandEd->text().isEmpty() &&
            ui->commandList->isVisible() &&
            ui->commandList->currentIndex().row() == 0
           )
        {
            setFilter("", false);
            return true;
        }
        qApp->sendEvent(ui->commandList, event);
        return true;

    case Qt::Key_Down:
    case Qt::Key_PageDown:
        if (ui->commandEd->text().isEmpty() &&
            ui->commandList->isHidden()
           )
        {
            setFilter("", true);
            return true;
        }

        qApp->sendEvent(ui->commandList, event);
        return true;

    case Qt::Key_Tab:
        const CommandProviderItem *command = mCommandItemModel->command(ui->commandList->currentIndex());
        if (command)
            ui->commandEd->setText(command->title());
        return true;
    }

    return QDialog::eventFilter(ui->commandList, event);
}


/************************************************
 eventFilter for ui->commandList
 ************************************************/
bool Dialog::listKeyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Up:
    case Qt::Key_PageUp:
        if (ui->commandList->currentIndex().row() == 0)
        {
            ui->commandEd->setFocus();
            return true;
        }
        break;

    case Qt::Key_Left:
    case Qt::Key_Right:
        ui->commandEd->setFocus();
        qApp->sendEvent(ui->commandEd, event);
        return true;

    default:
        // Alphabetical or number key ...........
        if (!event->text().isEmpty())
        {
            ui->commandEd->setFocus();
            qApp->sendEvent(ui->commandEd, event);
            return true;
        }
    }

    return QDialog::eventFilter(ui->commandEd, event);
}


/************************************************

 ************************************************/
void Dialog::showHide()
{
    if (isVisible() && isActiveWindow())
    {
        hide();
    }
    else
    {
        realign();
        show();
        KWindowSystem::forceActiveWindow(windowHandle()->winId());
        ui->commandEd->setFocus();
    }
}


/************************************************

 ************************************************/
void Dialog::realign()
{
    QRect desktop;

    int screen = mMonitor;
    if (mMonitor < 0 || mMonitor > QApplication::desktop()->screenCount() - 1)
        screen = QApplication::desktop()->screenNumber(QCursor::pos());

    desktop = QApplication::desktop()->availableGeometry(screen).intersected(KWindowSystem::workArea(screen));

    QRect rect = this->geometry();
    rect.moveCenter(desktop.center());

    if (mShowOnTop)
        rect.moveTop(desktop.top());
    else
        rect.moveTop(desktop.center().y() - ui->panel->sizeHint().height());

    setGeometry(rect);
}


/************************************************

 ************************************************/
void Dialog::applySettings()
{
    if (mLockCascadeChanges)
        return;

    // Shortcut .................................
    QString shortcut = mSettings->value("dialog/shortcut", DEFAULT_SHORTCUT).toString();
    if (shortcut.isEmpty())
        shortcut = DEFAULT_SHORTCUT;

    if (!mGlobalShortcut)
        mGlobalShortcut = GlobalKeyShortcut::Client::instance()->addAction(shortcut, "/runner/show_hide_dialog", tr("Show/hide runner dialog"), this);
    else if (mGlobalShortcut->shortcut() != shortcut)
        mGlobalShortcut->changeShortcut(shortcut);

    mShowOnTop = mSettings->value("dialog/show_on_top", true).toBool();

    mMonitor = mSettings->value("dialog/monitor", -1).toInt();

    realign();
    mSettings->sync();
}


/************************************************

 ************************************************/
void Dialog::shortcutChanged(const QString &/*oldShortcut*/, const QString &newShortcut)
{
    if (!newShortcut.isEmpty())
    {
        mLockCascadeChanges = true;

        mSettings->setValue("dialog/shortcut", newShortcut);
        mSettings->sync();

        mLockCascadeChanges = false;
    }
}


/************************************************

 ************************************************/
void Dialog::setFilter(const QString &text, bool onlyHistory)
{
    qDebug() << "Ind i setFilter...";
    if (mCommandItemModel->isOutDated())
        mCommandItemModel->rebuild();

    QString trimmedText = text.simplified();
    mCommandItemModel->setCommand(trimmedText);
    mCommandItemModel->showOnlyHistory(onlyHistory);
    mCommandItemModel->setFilterWildcard(trimmedText);
    mCommandItemModel->sort(0);
}

/************************************************

 ************************************************/
void Dialog::dataChanged()
{
    if (mCommandItemModel->rowCount())
    {
       ui->commandList->setCurrentIndex(mCommandItemModel->appropriateItem(mCommandItemModel->command()));
        ui->commandList->scrollTo(ui->commandList->currentIndex());
        ui->commandList->show();
    }
    else
    {
        ui->commandList->hide();
    }

    adjustSize();

}


/************************************************

 ************************************************/
void Dialog::runCommand()
{
    bool res =false;
    const CommandProviderItem *command = mCommandItemModel->command(ui->commandList->currentIndex());

    if (command)
        res = command->run();

    if (res)
    {
        hide();
        ui->commandEd->clear();
    }

}


/************************************************

 ************************************************/
void Dialog::showConfigDialog()
{
    if (!mConfigureDialog)
        mConfigureDialog = new ConfigureDialog(mSettings, DEFAULT_SHORTCUT, this);
    mConfigureDialog->exec();
}

#undef DEFAULT_SHORTCUT
