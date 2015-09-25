#include "backupthread.h"

#include <QThread>
#include <QDirIterator>
#include <QDebug>
#include <QDateTime>
#include <QListWidgetItem>
#include <QSettings>

bool BackUpThread::running = false;

BackUpThread::BackUpThread() { }

BackUpThread::~BackUpThread() { }

void BackUpThread::run() {
    if(running) {
        output("Currently backing up. Please wait.");
        return;
    }
    running = true;
    output("Starting back-up.");

    int total_files = 0;
    int processed_files = 0;

    foreach(QString dir, dirs_to_back_up) {
        total_files += count_files(dir);
    }

    foreach(QString org_dir, dirs_to_back_up) {
        QString new_dir = QDir(dir_to_back_up_to).filePath(QDir(org_dir).dirName());
        if(!QDir(new_dir).exists()) {
            QDir().mkdir(new_dir);
        }

        backup_modified_files(org_dir, new_dir, &total_files, &processed_files);
        remove_deleted_files(org_dir, new_dir);
    }
    output("Back-up complete: "+QDateTime::currentDateTime().toString());
    running = false;
}

void BackUpThread::set_arguments(QList<QString> org_dirs, QString new_dir) {
    this->dirs_to_back_up = org_dirs;
    this->dir_to_back_up_to = new_dir;
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

void BackUpThread::backup_modified_files(QString org_dir, QString new_dir, int *total_files, int *processed_files) {
    // Back-Up files in current directory
    QDirIterator current_dir_files(org_dir, QDir::Files | QDir::NoDotAndDotDot);
    while (current_dir_files.hasNext()) {
        QString next_file = current_dir_files.next();
        QString next_file_copy = QString::fromUtf16(next_file.utf16());
        next_file_copy.replace(org_dir, new_dir);
        if (!QFile::exists(next_file_copy)) {
            QFile::copy(next_file, next_file_copy);
            output("Creating back-up of: "+next_file);
        } else if (QFileInfo(next_file).lastModified() > QFileInfo(next_file_copy).lastModified()) {
            QFile::remove(next_file_copy);
            QFile::copy(next_file, next_file_copy);
            output("Updating back-up of: "+next_file);
        }
        (*processed_files)++;
        update_progress(total_files, processed_files);
    }
    // Back-Up files in subdirectories
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
            QString next_file_copy = QString::fromUtf16(next_file.utf16());
            next_file_copy.replace(org_dir, new_dir);
            if (!QFile::exists(next_file_copy)) {
                QFile::copy(next_file, next_file_copy);
                output("Creating back-up of: "+next_file);
            } else if (QFileInfo(next_file).lastModified() > QFileInfo(next_file_copy).lastModified()) {
                QFile::remove(next_file_copy);
                QFile::copy(next_file, next_file_copy);
                output("Updating back-up of: "+next_file);
            }
            (*processed_files)++;
            update_progress(total_files, processed_files);
        }
    }
}

void BackUpThread::remove_deleted_files(QString org_dir, QString new_dir) {
    QDirIterator current_copy_dir_files(new_dir, QDir::Files | QDir::NoDotAndDotDot);
    while (current_copy_dir_files.hasNext()) {
        QString next_file = current_copy_dir_files.next();
        QString next_file_org = QString::fromUtf16(next_file.utf16());
        next_file_org.replace(new_dir, org_dir);
        if (!QFile::exists(next_file_org)) {
            QFile::remove(next_file);
            output("Removing: "+next_file);
        }
    }
    QDirIterator copy_dirs(new_dir, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (copy_dirs.hasNext()) {
        QString next = copy_dirs.next();
        QString next_org = QString::fromUtf16(next.utf16());
        next_org.replace(new_dir, org_dir);
        if(!QDir(next_org).exists()) {
            QDir(next).removeRecursively();
        }
        QDirIterator copy_files(next, QDir::Files | QDir::NoDotAndDotDot);
        while (copy_files.hasNext()) {
            QString next_file = copy_files.next();
            QString next_file_org = QString::fromUtf16(next_file.utf16());
            next_file_org.replace(new_dir, org_dir);
            if (!QFile::exists(next_file_org)) {
                QFile::remove(next_file);
                output("Removing: "+next_file);
            }
        }
    }
}

void BackUpThread::update_progress(int *total_files, int *processed_files) {
    float current_percent = (float) (*processed_files)/(*total_files)*100;
    // qDebug() << *total_files << *processed_files;
    progress_changed((int) current_percent);
}

void BackUpThread::output(QString string) {
    qDebug() << string;
    task_changed(string);
}

bool BackUpThread::is_running() {
    return BackUpThread::running;
}

