#ifndef BACKUPTHREAD_H
#define BACKUPTHREAD_H

#include <QObject>
#include <QThread>
#include <QSettings>

class BackUpThread: public QThread {
        Q_OBJECT

    public:
        BackUpThread();
        ~BackUpThread();
        void run();
        void set_arguments(QList<QString> org_dirs, QString new_dir);
        void remove_deleted_files(QString org_dir, QString new_dir);
        int count_files(QString directory);
        void backup_modified_files(QString org_dir, QString new_dir, int *num_of_dirs, int *current_dir);
        void update_progress(int *num_of_dirs, int *current_dir);
        static bool is_running();
    signals:
        void progress_changed(int);
        void task_changed(QString);
    protected:
        static bool running;
        QList<QString> dirs_to_back_up;
        QString dir_to_back_up_to;
        void output(QString string);
};

#endif // BACKUPTHREAD_H
