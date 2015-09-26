#include "backupthread.h"

#include <QThread>
#include <QDirIterator>
#include <QDebug>
#include <QDateTime>
#include <QListWidgetItem>
#include <QSettings>
#include <QList>

bool BackUpThread::running = false;

BackUpThread::BackUpThread() { }

BackUpThread::~BackUpThread() { }

void BackUpThread::run() {
    if(running) {
        output("Currently copying. Please wait.");
        return;
    }
    running = true;
    output("Scanning directory...");

    int total_files = count_files(archive);
    int processed_files = 0;

    copy_files(dropbox, archive, &total_files, &processed_files);
    output("Copying complete: "+QDateTime::currentDateTime().toString());
    running = false;
}

void BackUpThread::set_arguments(QString dropbox_dir, QString archive_dir) {
    this->dropbox = dropbox_dir;
    this->archive = archive_dir;
}

int BackUpThread::count_files(QString directory) {
    int num_of_files = 0;
    // First count the files that are in the current directory
    QDirIterator current_dir_files(directory, QDir::Files | QDir::NoDotAndDotDot);
    while (current_dir_files.hasNext()) {
        current_dir_files.next();
        num_of_files++;
    }
    // Now count the files for all subdirectories
    QDirIterator count_dirs(directory, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (count_dirs.hasNext()) {
        QDirIterator files(count_dirs.next(), QDir::Files | QDir::NoDotAndDotDot);
        while (files.hasNext()) {
            files.next();
            num_of_files++;
        }
    }
    return num_of_files;
}

void BackUpThread::copy_files(QString new_dir, QString org_dir, int *total_files, int *processed_files) {
    QList<QString> filters;
    filters << "RAW" << ".mp4" << ".wav" << "HR" << ".psd";

    // Copy files in current directory
    QDirIterator current_dir_files(org_dir, QDir::Files | QDir::NoDotAndDotDot);
    while (current_dir_files.hasNext()) {
        QString next_file = current_dir_files.next();
        if (!check_filter(next_file, filters)) {
            QString next_file_copy = QString::fromUtf16(next_file.utf16());
            next_file_copy.replace(org_dir, new_dir);
            copy_file(next_file, next_file_copy);
        }
        (*processed_files)++;
        update_progress(total_files, processed_files);
    }
    // Copy files in subdirectories
    QDirIterator dirs(org_dir, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (dirs.hasNext()) {
        QString next = dirs.next();
        QString next_copy = QString::fromUtf16(next.utf16());
        next_copy.replace(org_dir, new_dir);
        if(!QDir(next_copy).exists()) {
            QDir().mkdir(next_copy);
        }
        QDirIterator files(next, QDir::Files | QDir::NoDotAndDotDot);
        while (files.hasNext()) {
            QString next_file = files.next();
            if (!check_filter(next_file, filters)) {
                QString next_file_copy = QString::fromUtf16(next_file.utf16());
                next_file_copy.replace(org_dir, new_dir);
                copy_file(next_file, next_file_copy);
            }
            (*processed_files)++;
            update_progress(total_files, processed_files);
        }
    }
}

bool BackUpThread::check_filter(QString fileName, QList<QString> &filters) {
    QString filter;
    foreach (filter, filters) {
        if (fileName.contains(filter, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

void BackUpThread::copy_file(QString org, QString copy) {
    if (!QFile::exists(copy)) {
        QFile::copy(org, copy);
        output("Creating "+copy);
    } else if (QFileInfo(org).lastModified() > QFileInfo(copy).lastModified()) {
        QFile::remove(copy);
        QFile::copy(org, copy);
        output("Updating "+copy);
    }
}

void BackUpThread::update_progress(int *total_files, int *processed_files) {
    float current_percent = (float) (*processed_files)/(*total_files)*100;
    //qDebug() << *total_files << *processed_files;
    progress_changed((int) current_percent);
}

void BackUpThread::output(QString string) {
    qDebug() << string;
    task_changed(string);
}

bool BackUpThread::is_running() {
    return BackUpThread::running;
}

