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
        void set_arguments(QString dropbox_dir, QString archive_dir);
        static bool is_running();

    signals:
        void progress_changed(int);
        void task_changed(QString);

    protected:
        static bool running;
        void output(QString string);
        bool check_filter(QString fileName, QList<QString> &filters);
        void copy_files(QString org_dir, QString new_dir, int *num_of_dirs, int *current_dir);
        void copy_file(QString org, QString copy);
        void update_progress(int *num_of_dirs, int *current_dir);
        int count_files(QString directory);
        QString dropbox;
        QString archive;

};

#endif // BACKUPTHREAD_H
