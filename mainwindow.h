#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QListWidgetItem>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QProgressBar>
#include <QCloseEvent>
#include <QTimer>
#include <QDateTime>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();
        void menu_icon_style();
        void load_settings();
        void set_progress(int progress);
        bool quit;
        QSettings settings;
        void run_back_up();
        void set_last_back_up_time();
        QDateTime get_last_back_up_time();

    private slots:
        void on_timer();

        void when_show_hide(QSystemTrayIcon::ActivationReason reason);

        void when_show_hide();

        void on_actionClose_triggered();

        void quit_action();

        void on_backed_up_dirs_browse_clicked();

        void on_backed_up_dirs_list_itemClicked(QListWidgetItem *item);

        void on_remove_selected_directory_button_clicked();

        void on_back_up_to_browse_clicked();

        void on_back_up_on_schedule_toggled(bool checked);

        void on_save_settings_button_clicked();

        void on_back_up_now_clicked();

        void get_progress_changed(int prog);

        void on_actionQuit_triggered();

    signals:
        void progress_changed(int);

        void show_tray_icon_msg(QString, QString);


    private:
        Ui::MainWindow *ui;
        void create_tray_icon();
        QProgressBar *m_tray_progress;
        QTimer *timer;
        void write_state_settings();
        void read_state_settings();
        void display_hide_message();

    protected:
        void closeEvent(QCloseEvent *);

};

#endif // MAINWINDOW_H
