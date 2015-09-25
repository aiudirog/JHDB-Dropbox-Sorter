#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "backupthread.h"
#include <QFileDialog>
#include <QListWidgetItem>
#include <QDebug>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QWidgetAction>
#include <QProgressBar>
#include <QCloseEvent>
#include <QTimer>
#include <QDateTime>
#include <QSysInfo>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    this->quit = false;
    ui->setupUi(this);
    read_state_settings();
    create_tray_icon();
    load_settings();
    menu_icon_style();

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(on_timer()));
    timer->start(60000);

    ui->current_task->setText("Last Back-Up: "+this->settings.value("MainWindow/last_back_up").toString());
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::menu_icon_style() {
    QStyle *style = QApplication::style();
    // ui->actionClose->setIcon(style->standardIcon(QStyle::SP_ArrowDown));
    ui->actionQuit->setIcon(style->standardIcon(QStyle::SP_DialogCloseButton));
}

void MainWindow::on_timer() {
    // Don't run if user doesn't want to back-up on a schedule
    if (!this->settings.value("MainWindow/back_up_on_schedule").toBool()) {
        // qDebug() << "User does not want to back up on schedule.";
        return;
    }

    // Check if back-up is currently in progress
    if (BackUpThread::is_running()) {
        // qDebug() << "Back up is already running, aborting.";
        return;
    }

    // Check if enough time has passed
    QDateTime last_back_up = MainWindow::get_last_back_up_time();
    if (last_back_up == QDateTime()) {
        last_back_up = QDateTime::fromMSecsSinceEpoch(0);
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 time_span = last_back_up.daysTo(now);
    int days = this->settings.value("MainWindow/days_spinBox").toInt();

    if (!(time_span >= days)) {
        // qDebug() << last_back_up << now << time_span << days;
        // qDebug() << "Not enough days have passed.";
        return;
    }

    QTime time = QTime::fromString(this->settings.value("MainWindow/time_timeEdit").toString(),"h:m ap");
    if (!(QTime::currentTime() >= time)) {
        // qDebug() << "Not time yet.";
        return;
    }

    // All checks passed, let's back up
    this->run_back_up();
}

void MainWindow::load_settings() {
    int size = this->settings.beginReadArray("MainWindow/backed_up_dirs_list");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        ui->backed_up_dirs_list->addItem(QString(settings.value("directory").toString()));
    }
    this->settings.endArray();

    ui->back_up_to_line_edit->setText(this->settings.value("MainWindow/back_up_to_line_edit").toString());
    ui->back_up_on_schedule->setChecked(this->settings.value("MainWindow/back_up_on_schedule").toBool());
    ui->days_spinBox->setValue(this->settings.value("MainWindow/days_spinBox").toInt());
    ui->time_timeEdit->setTime(QTime::fromString(this->settings.value("MainWindow/time_timeEdit").toString(),"h:m ap"));
}

void MainWindow::create_tray_icon() {
    QSystemTrayIcon *m_tray_icon = new QSystemTrayIcon(QIcon(":/Icons/Icons/File Cabinet/1434274458_Card_file.png"), this);

    connect(m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(when_show_hide(QSystemTrayIcon::ActivationReason)));
    connect(this, SIGNAL(show_tray_icon_msg(QString, QString)), m_tray_icon, SLOT(showMessage(QString, QString)));

    QAction *quit_action = new QAction("Quit", m_tray_icon);
    connect(quit_action, SIGNAL(triggered()), this, SLOT(quit_action()));

    QStyle *style = QApplication::style();
    quit_action->setIcon(style->standardIcon(QStyle::SP_DialogCloseButton));

    QAction *hide_action = new QAction("Settings", m_tray_icon);
    connect(hide_action, SIGNAL(triggered()), this, SLOT(when_show_hide()));

    QMenu *tray_icon_menu = new QMenu;
    tray_icon_menu->addAction(hide_action);
    tray_icon_menu->addAction(quit_action);

    m_tray_progress = new QProgressBar();
    connect(this, SIGNAL(progress_changed(int)), m_tray_progress, SLOT(setValue(int)));

    QWidgetAction *m_tray_progress_action = new QWidgetAction(m_tray_icon);
    m_tray_progress_action->setDefaultWidget(m_tray_progress);

    tray_icon_menu->addAction(m_tray_progress_action);

    m_tray_icon->setContextMenu(tray_icon_menu);

    m_tray_icon->show();
}

void MainWindow::set_progress(int progress) {
    this->progress_changed(progress);
}

void MainWindow::get_progress_changed(int prog) {
    this->set_progress(prog);
}

void MainWindow::when_show_hide(QSystemTrayIcon::ActivationReason reason) {
    if (reason) {
        if( reason != QSystemTrayIcon::DoubleClick )
            return;
    }

    if( isVisible() ) {
        hide();
    } else {
        show();
        raise();
        setFocus();
    }
}

void MainWindow::when_show_hide() {
    if( isVisible() ) {
        hide();
    } else {
        show();
        raise();
        setFocus();
    }

    display_hide_message();
}

void MainWindow::on_actionClose_triggered() {
    this->when_show_hide();
}

void MainWindow::quit_action() {
    this->quit = true;
    this->close();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (this->quit) {
        write_state_settings();
        event->accept();
    } else {
        this->when_show_hide();
        event->ignore();
    }
}

void MainWindow::on_backed_up_dirs_browse_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "/home",
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->backed_up_dirs_line_edit->setText(dir);
    ui->backed_up_dirs_list->addItem(dir);
}

void MainWindow::on_backed_up_dirs_list_itemClicked(QListWidgetItem *item) {
    ui->backed_up_dirs_line_edit->setText(item->text());
}

void MainWindow::on_remove_selected_directory_button_clicked() {
    QList<QListWidgetItem*> items = ui->backed_up_dirs_list->selectedItems();
    foreach(QListWidgetItem *item, items) {
        ui->backed_up_dirs_list->removeItemWidget(item);
        delete item;
    }
}

void MainWindow::on_back_up_to_browse_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "/home",
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->back_up_to_line_edit->setText(dir);
}


void MainWindow::on_back_up_on_schedule_toggled(bool checked) {
    ui->back_up_every_label->setEnabled(checked);
    ui->days_spinBox->setEnabled(checked);
    ui->days_at_label->setEnabled(checked);
    ui->time_timeEdit->setEnabled(checked);
}

void MainWindow::on_save_settings_button_clicked() {
    qDebug() << "Settings stored at:" << this->settings.fileName();
    QList<QListWidgetItem *> items =
        ui->backed_up_dirs_list->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard);
    QList<QString> texts;
    foreach(QListWidgetItem *item, items) texts.append(item->text());

    this->settings.beginWriteArray("MainWindow/backed_up_dirs_list");
    for (int i = 0; i < texts.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("directory", texts.at(i));
    }
    this->settings.endArray();
    this->settings.setValue("MainWindow/back_up_to_line_edit", ui->back_up_to_line_edit->text());
    this->settings.setValue("MainWindow/back_up_on_schedule", ui->back_up_on_schedule->isChecked());
    this->settings.setValue("MainWindow/days_spinBox", ui->days_spinBox->value());
    this->settings.setValue("MainWindow/time_timeEdit", ui->time_timeEdit->time().toString("h:m ap"));
}

void MainWindow::on_back_up_now_clicked()
{
    this->on_save_settings_button_clicked();
    this->run_back_up();
}

void MainWindow::run_back_up() {
    QList<QString> dirs_to_back_up;
    QString dir_to_back_up_to;

    int size = this->settings.beginReadArray("MainWindow/backed_up_dirs_list");
    for (int i = 0; i < size; ++i) {
        this->settings.setArrayIndex(i);
        dirs_to_back_up.append(QString(this->settings.value("directory").toString()));
    }
    this->settings.endArray();

    // Don't run if we have no directories to back-up.
    if (dirs_to_back_up.isEmpty()) return;

    dir_to_back_up_to = this->settings.value("MainWindow/back_up_to_line_edit").toString();

    // Don't run if we don't have a directory to back up to.
    if (dir_to_back_up_to.isEmpty()) return;
    if (!QDir(dir_to_back_up_to).exists()) {
        QDir().mkdir(dir_to_back_up_to);
    }
    BackUpThread *back_up_thread = new BackUpThread();
    connect(back_up_thread,SIGNAL(progress_changed(int)),this,SLOT(get_progress_changed(int)));
    connect(back_up_thread,SIGNAL(task_changed(QString)),ui->current_task,SLOT(setText(QString)));
    back_up_thread->set_arguments(dirs_to_back_up, dir_to_back_up_to);
    back_up_thread->start();

    this->set_last_back_up_time();

}

void MainWindow::set_last_back_up_time() {
    this->settings.setValue("MainWindow/last_back_up", QDateTime::currentDateTime().toString());
}

QDateTime MainWindow::get_last_back_up_time() {
    QString last_time = this->settings.value("MainWindow/last_back_up").toString();
    return QDateTime::fromString(last_time);
}

void MainWindow::write_state_settings() {
    this->settings.beginGroup("MainWindow");
    this->settings.setValue("size", size());
    this->settings.setValue("pos", pos());
    this->settings.setValue("visible", isVisible());
    this->settings.endGroup();
}

void MainWindow::read_state_settings() {
    this->settings.beginGroup("MainWindow");
    resize(this->settings.value("size", QSize(650, 450)).toSize());
    move(this->settings.value("pos", QPoint(100, 100)).toPoint());
    if (this->settings.value("visible", true).toBool()) {
        show();
        raise();
        setFocus();
    } else {
        hide();
    }
    this->settings.endGroup();
}

void MainWindow::on_actionQuit_triggered()
{
    quit_action();
}

void MainWindow::display_hide_message() {
    if (!this->settings.value("MainWindow/done_show_hide").toBool()) {
        if (QSystemTrayIcon::supportsMessages() && QSysInfo::productType() == "windows") {
            show_tray_icon_msg("Back-up Utility is still running", "Back-up Utility is still running and has been "
                                                                   "minimized to the system tray. To completely close "
                                                                   "this program, right click on this icon and select "
                                                                   "'Quit' or from the Settings window, select "
                                                                   "'File->Quit'");

            this->settings.setValue("MainWindow/done_show_hide", true);
        }

    } else {
        return;
    }
}
