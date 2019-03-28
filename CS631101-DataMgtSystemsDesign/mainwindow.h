#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QMessageBox"
#include "QtSql/QSqlDatabase"
#include "QSql"
#include "QSqlQuery"
#include "qdebug.h"
#include <QSqlError>
#include "QDateTime"
#include "qtimer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_PB_signin_clicked();

    void on_PB_register_clicked();

    void on_PB_exit_clicked();

    void Sign_In_Init();

    void Check_15_days();

    void on_CB_bank_account_activated(int index);

    void on_CB_transfer_money_activated(int index);

    void on_CB_account_activated(int index);

    void on_PB_link_bank_account_link_clicked();

    void on_CB_history_type_activated(int index);

    void on_PB_request_money_add_clicked();

    void on_PB_verify_bank_account_verify_clicked();

    void on_PB_send_money_send_clicked();

    void update_front_page();

    void on_CB_unlink_bank_account_id_activated(const QString &arg1);

    void on_PB_unlink_bank_account_unlink_clicked();

    void on_CB_verify_bank_account_id_activated(const QString &arg1);

    void on_CB_set_primary_account_id_activated(const QString &arg1);

    void on_PB_set_primary_account_set_clicked();

    void on_PB_verify_account_email_clicked();

    void on_PB_verify_account_phone_clicked();

    void on_CB_cancel_payment_stid_activated(const QString &arg1);

    void on_PB_cancel_payment_cancel_clicked();

    void on_CB_take_money_bank_id_activated(const QString &arg1);

    void on_PB_take_money_take_clicked();

    void on_PB_link_identifier_link_clicked();

    void on_TW_request_money_cellClicked(int row, int column);

    void on_PB_request_money_request_clicked();

    void on_TW_requested_money_cellClicked(int row, int column);

    void on_PB_request_money_split_clicked();

    void on_PB_request_money_amount_clicked();

    void on_PB_history_search_clicked();

    void on_PB_signout_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QString userssn;
};

#endif // MAINWINDOW_H
