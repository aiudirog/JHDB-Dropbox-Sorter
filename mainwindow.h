#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QListWidgetItem>
#include <QSettings>
#include <QProgressBar>
#include <QCloseEvent>


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
        void set_progress(int progress);

        QSettings settings;
        void run_back_up();
        void get_progress_changed(int prog);

    private slots:
        void on_move_to_dropbox_button_clicked();
        void on_dropbox_folder_browse_clicked();
        void on_dir_to_copy_browse_clicked();

    signals:
        void progress_changed(int);

    private:
        Ui::MainWindow *ui;
        void write_state_settings();
        void read_state_settings();
        void load_settings();
        void save_settings();

    protected:
        void closeEvent(QCloseEvent *);

};

#endif // MAINWINDOW_H
