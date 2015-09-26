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


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    read_state_settings();
    load_settings();
    menu_icon_style();
    this->show();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::menu_icon_style() {
    QStyle *style = QApplication::style();
    ui->actionQuit->setIcon(style->standardIcon(QStyle::SP_DialogCloseButton));
}

void MainWindow::run_back_up() {
    QString dropbox_dir;
    QString archive_dir;

    dropbox_dir = this->settings.value("MainWindow/dropbox_folder_lineedit").toString();

    // Don't run if we don't have a Dropbox folder.
    if (dropbox_dir.isEmpty()) return;

    archive_dir = ui->dir_to_copy_lineedit->text();

    // Don't run if we don't have a directory to copy.
    if (archive_dir.isEmpty()) return;

    BackUpThread *back_up_thread = new BackUpThread();
    connect(back_up_thread,SIGNAL(progress_changed(int)),this,SLOT(get_progress_changed(int)));
    connect(back_up_thread,SIGNAL(task_changed(QString)),ui->current_task,SLOT(setText(QString)));
    back_up_thread->set_arguments(dropbox_dir, archive_dir);
    back_up_thread->start();

}



/////////////////////////////////////////////////////////////////////////////////////////////////
/// PROGRESS MANAGEMENT                                         /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::set_progress(int progress) {
    this->progress_changed(progress);
}

void MainWindow::get_progress_changed(int prog) {
    this->set_progress(prog);
}



/////////////////////////////////////////////////////////////////////////////////////////////////
/// ACTIONS                                                     /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::on_move_to_dropbox_button_clicked()
{
    this->save_settings();
    this->run_back_up();
}

void MainWindow::on_dropbox_folder_browse_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "/home",
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->dropbox_folder_lineedit->setText(dir);
    this->save_settings();
}

void MainWindow::on_dir_to_copy_browse_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "/home",
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->dir_to_copy_lineedit->setText(dir);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    this->write_state_settings();
}



/////////////////////////////////////////////////////////////////////////////////////////////////
/// SETTINGS                                                    /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::save_settings() {
    qDebug() << "Settings stored at:" << this->settings.fileName();
    this->settings.setValue("MainWindow/dropbox_folder_lineedit", ui->dropbox_folder_lineedit->text());
}

void MainWindow::load_settings() {
    ui->dropbox_folder_lineedit->setText(this->settings.value("MainWindow/dropbox_folder_lineedit").toString());
}

void MainWindow::write_state_settings() {
    this->settings.beginGroup("MainWindow");
    this->settings.setValue("size", size());
    this->settings.setValue("pos", pos());
    this->settings.endGroup();
}

void MainWindow::read_state_settings() {
    this->settings.beginGroup("MainWindow");
    resize(this->settings.value("size", QSize(650, 450)).toSize());
    move(this->settings.value("pos", QPoint(100, 100)).toPoint());
    this->settings.endGroup();
}



/////////////////////////////////////////////////////////////////////////////////////////////////
