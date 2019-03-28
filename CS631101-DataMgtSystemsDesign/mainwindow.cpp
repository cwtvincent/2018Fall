#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QMessageBox mb;
    QTimer *timer = new QTimer(this);
    ui->setupUi(this);
    ui->Main_TB->setCurrentIndex(0);

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setDatabaseName("tijn");
    db.setUserName("root");
    db.setPassword("Abcd1234");

    if(!db.open()){
        mb.setText("Database failed to open.");
        mb.exec();
        qDebug() << db.lastError();
    }


    connect(timer, SIGNAL(timeout()), this, SLOT(Check_15_days()));
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Check_15_days()
{
    QSqlQuery query(db), query2(db);
    QString datetime;
    QMessageBox mb;

    datetime = "";
    datetime += QDate::currentDate().addDays(-15).toString(Qt::ISODate);
    datetime += " ";
    datetime += QTime::currentTime().toString();
    query.prepare("SELECT STid, Amount, SSN "
                  "FROM SEND_TRANSACTION "
                  "WHERE DateTime < :dt and Cancelled = 'n' and Identifier NOT IN (SELECT Identifier "
                  "                                                                FROM ELECTRONIC_ADDRESS)");
    query.bindValue(":dt", datetime);
    if(!query.exec()){
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    else if(query.next()){
        do{
            query2.prepare("UPDATE SEND_TRANSACTION "
                           "SET Cancelled = 'y' "
                           "WHERE STid = :stid ");
            query2.bindValue(":stid", query.value(0).toString());
            if(!query2.exec()){
                mb.setText(query2.lastError().text());
                mb.exec();
                qDebug() << query2.lastError();
            }
            query2.clear();

            query2.prepare("UPDATE USER_ACCOUNT "
                          "SET Balance = Balance + :amou "
                          "WHERE SSN = :ssn ");
            query2.bindValue(":amou", query.value(1).toDouble());
            query2.bindValue(":ssn", query.value(2).toString());
            if(!query2.exec()){
                mb.setText(query2.lastError().text());
                mb.exec();
                qDebug() << query2.lastError();
            }
            query2.clear();

        }while(query.next());
    }
}

void MainWindow::on_PB_exit_clicked()
{
    MainWindow::close();
}

void MainWindow::update_front_page()
{
    QSqlQuery query(db);
    QMessageBox mb;
    query.prepare("SELECT * "
                  "FROM   USER_ACCOUNT "
                  "WHERE  ssn = :ssn");
    query.bindValue(":ssn", userssn);
    if(!query.exec()){
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    else{
        query.next();
        ui->LE_front_page_ssn->setText(query.value(0).toString());
        ui->LE_front_page_name->setText(query.value(1).toString());
        ui->LE_front_page_balance->setText(query.value(2).toString());
        ui->LE_front_page_primary_bank_id->setText(query.value(3).toString());
        ui->LE_front_page_primary_bank_num->setText(query.value(4).toString());
    }
}

void MainWindow::on_PB_signin_clicked()
{
    QMessageBox mb;
    QString str;
    bool invalid = false;

    //USERNAME
        str = ui->LE_s_username->text();
        if(str.size() < 8 || str.size() > 14){invalid = true;}
    //PASSWORD
        str = ui->LE_s_password->text();
        if(str.size() < 8 || str.size() > 14){invalid = true;}

    if(invalid){
        mb.setText("Invalid username or password.\nPlease try again.");
        mb.exec();
    }
    else{}
    QSqlQuery query(db);
    query.prepare("SELECT * "
                  "FROM   REGISTER_ACCOUNT "
                  "WHERE  USERNAME = :un");
    query.bindValue(":un", ui->LE_s_username->text());
    if(!query.exec()){
        invalid = true;
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    if(query.next()){
        do{
            if(query.value(0).toString() == ui->LE_s_username->text() &&
               query.value(1).toString() == ui->LE_s_password->text()){
                userssn = query.value(2).toString();
                ui->Main_TB->setCurrentIndex(1);
                ui->Sub_TB->setCurrentIndex(0);
                update_front_page();
                ui->LE_s_username->setText("");
                ui->LE_s_password->setText("");
            }
            else{
                mb.setText("Wrong password.\nPlease try again.");
                mb.exec();
            }
        }while(query.next());
    }
    else{
        mb.setText("Wrong username.\nPlease try again.");
        mb.exec();
    }
    query.clear();
}

void MainWindow::on_PB_register_clicked()
{
    QString str, mbtext = "", datetime = "";
    QMessageBox mb;
    QSqlQuery query(db);
    int i, deleteindex = 0, amount = 0;
    bool invalid = false, tmp = false;

//USERNAME
    str = ui->LE_r_username->text();
    if(str.size() < 8 || str.size() > 14){invalid = true; mbtext += "Invalid username.\n";}
    for(i = 0; i != str.size(); ++i){}
//PASSWORD
    str = ui->LE_r_password->text();
    if(str.size() < 8 || str.size() > 14){invalid = true; mbtext += "Invalid password.\n";}
    for(i = 0; i != str.size(); ++i){}
//SSN
    str = ui->LE_ssn->text();
    if(str.size() != 9) tmp = true;
    for(i = 0; i != str.size(); ++i)
        if(str[i].toLatin1()<48 || str[i].toLatin1()>57) tmp = true;
    if(tmp){invalid = true; mbtext += "Invalid ssn.\n"; tmp = false;}

//NAME
    str = ui->LE_fname->text();
    if(str.size() < 1 || str.size() > 14) tmp = true;
    for(i = 0; i != str.size(); ++i)
        if((str[i].toLatin1()<65 || str[i].toLatin1()>90) && (str[i].toLatin1()<97 || str[i].toLatin1()>122)) tmp = true;
    if(tmp) {invalid = true; mbtext += "Invalid first name.\n"; tmp = false;}

//PHONE
    str = ui->LE_phone->text();
    if(str.size() != 10) tmp = true;
    for(i = 0; i != str.size(); ++i)
        if(str[i].toLatin1()<48 || str[i].toLatin1()>57) tmp = true;
    if(tmp){invalid = true; mbtext += "Invalid phone number.\n"; tmp = false;}

//EMAIL
    str = ui->LE_email->text();
    if(str.size() < 3 || str.size() > 14){invalid = true; mbtext += "Invalid email.";}
//
    if(!invalid){
        query.clear();
        query.prepare("INSERT INTO REGISTER_ACCOUNT "
                      "VALUES (:un, :pw, :ssn)");
        query.bindValue(":un", ui->LE_r_username->text());
        query.bindValue(":pw", ui->LE_r_password->text());
        query.bindValue(":ssn", ui->LE_ssn->text());
        if(!query.exec()){
            invalid = true;
            mbtext += QString("The username is alreally exist")+'\n';
            qDebug() << query.lastError();
        }
        else{
            query.clear();

            query.prepare("INSERT INTO USER_ACCOUNT (SSN, Name) "
                       "VALUES (:ssn, :name)");
            query.bindValue(":ssn", ui->LE_ssn->text());
            query.bindValue(":name", ui->LE_fname->text());
            if(!query.exec()){
                deleteindex = 2;
                invalid = true;
                mbtext += QString("The ssn is alreally exist")+'\n';
                qDebug() << query.lastError();
            }
            else{
                query.clear();

                query.prepare("INSERT INTO ELECTRONIC_ADDRESS "
                           "VALUES ('p', :ssn, :iden, 'n')");
                query.bindValue(":ssn", ui->LE_ssn->text());
                query.bindValue(":iden", ui->LE_phone->text());
                if(!query.exec()){
                    deleteindex = 3;
                    invalid = true;
                    mbtext += QString("The phone number is alreally exist")+'\n';
                    qDebug() << query.lastError();
                }
                else{
                    query.clear();

                    query.prepare("INSERT INTO ELECTRONIC_ADDRESS "
                               "VALUES ('e', :ssn, :iden, 'n')");
                    query.bindValue(":ssn", ui->LE_ssn->text());
                    query.bindValue(":iden", ui->LE_email->text());
                    if(!query.exec()){
                        deleteindex = 4;
                        invalid = true;
                        mbtext += QString("The email is alreally exist")+'\n';
                        qDebug() << query.lastError();
                    }
                    query.clear();
                }
            }
        }
    }

    if(invalid){
        mb.setText(mbtext);
        mb.exec();
        if(deleteindex > 1){
            query.prepare("DELETE FROM REGISTER_ACCOUNT "
                          "WHERE SSN=:ssn");
            query.bindValue(":ssn", ui->LE_ssn->text());
            if(!query.exec()){
                qDebug() << query.lastError();
                mb.setText(query.lastError().text());
                mb.exec();
            }
            query.clear();
        }
        if(deleteindex > 2){
            query.prepare("DELETE FROM USER_ACCOUNT "
                          "WHERE SSN=:ssn");
            query.bindValue(":ssn", ui->LE_ssn->text());
            if(!query.exec()){
                qDebug() << query.lastError();
                mb.setText(query.lastError().text());
                mb.exec();
            }
            query.clear();
        }
        if(deleteindex > 3){
            query.prepare("DELETE FROM ELECTRONIC_ADDRESS "
                          "WHERE Identifier=:iden");
            query.bindValue(":iden", ui->LE_phone->text());
            if(!query.exec()){
                qDebug() << query.lastError();
                mb.setText(query.lastError().text());
                mb.exec();
            }
            query.clear();
        }
    }
    else{
        //CHECK IF SOMEONE ALREADY SEND YOU MONEY IN 15 DAYS.
        datetime = "";
        datetime += QDate::currentDate().addDays(-15).toString(Qt::ISODate);
        datetime += " ";
        datetime += QTime::currentTime().toString();
        //EMAIL
        query.clear();
        query.prepare("SELECT Amount "
                      "FROM   SEND_TRANSACTION "
                      "WHERE  Identifier = :iden and Cancelled = 'n' and DateTime > :dt");
        query.bindValue(":iden", ui->LE_email->text());
        query.bindValue(":dt", datetime);
        if(!query.exec()){
            qDebug() << query.lastError();
            mb.setText(query.lastError().text());
            mb.exec();
        }
        else if(query.next()){
            do{
                amount += query.value(0).toDouble();
            }while(query.next());
        }
        query.clear();
        //PHONE
        query.prepare("SELECT Amount "
                      "FROM SEND_TRANSACTION "
                      "WHERE Identifier = :iden and Cancelled = 'n' and DateTime > :dt");
        query.bindValue(":iden", ui->LE_phone->text());
        query.bindValue(":dt", datetime);
        if(!query.exec()){
            qDebug() << query.lastError();
            mb.setText(query.lastError().text());
            mb.exec();
        }
        else if(query.next()){
            do{
                amount += query.value(0).toDouble();
            }while(query.next());
        }
        query.clear();
        //+MONEY
        query.prepare("UPDATE USER_ACCOUNT "
                      "SET Balance = Balance + :bal "
                      "WHERE SSN = :ssn");
        query.bindValue(":bal", amount);
        query.bindValue(":ssn", ui->LE_ssn->text());
        if(!query.exec()){
            qDebug() << query.lastError();
            mb.setText(query.lastError().text());
            mb.exec();
        }
        query.clear();

        mb.setText("Registration succeeded !\nNow you can sign in !");
        ui->LE_r_password->setText("");
        ui->LE_r_username->setText("");
        ui->LE_email->setText("");
        ui->LE_fname->setText("");
        ui->LE_phone->setText("");
        ui->LE_ssn->setText("");
        mb.exec();
    }

}

void MainWindow::Sign_In_Init()
{
    ui->Sub_TB->setCurrentIndex(0);
}

void MainWindow::on_CB_bank_account_activated(int index)
{
    QSqlQuery query(db);
    QMessageBox mb;
    QStringList header;
    int count;
    // 1 Bank Account List, 2 Link Bank Account, 3 Unlink Bank Account, 4 Verify Bank Account,
    // 5 Primary Account Setting, 6 Take Money
    if(index != 0){
        ui->CB_account->setCurrentIndex(0);
        ui->CB_transfer_money->setCurrentIndex(0);
    }
    if(index == 1){
        ui->Sub_TB->setCurrentIndex(4);
        ui->TW_bank_account_list->clear();
        ui->TW_bank_account_list->setRowCount(0);
        ui->TW_bank_account_list->setColumnCount(3);
        header << tr("Bank ID") << tr("Bank Num") << tr("Verified");
        ui->TW_bank_account_list->setHorizontalHeaderLabels(header);
        query.prepare("SELECT * "
                      "FROM   HAS_ADDITIONAL "
                      "WHERE  SSN = :ssn");
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            do{
                count = ui->TW_bank_account_list->rowCount();
                ui->TW_bank_account_list->setRowCount(count+1);
                ui->TW_bank_account_list->setItem(count, 0, new QTableWidgetItem(query.value(1).toString()));
                ui->TW_bank_account_list->setItem(count, 1, new QTableWidgetItem(query.value(2).toString()));
                ui->TW_bank_account_list->setItem(count, 2, new QTableWidgetItem(query.value(3).toString()));
            }while(query.next());
        }
    }
    else if(index == 2){
        ui->Sub_TB->setCurrentIndex(5);
    }
    else if(index == 3){
        ui->Sub_TB->setCurrentIndex(6);
        ui->CB_unlink_bank_account_id->clear();
        query.prepare("SELECT DISTINCT BankID "
                      "FROM   HAS_ADDITIONAL "
                      "WHERE  SSN = :ssn");
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            do{
                ui->CB_unlink_bank_account_id->addItem(query.value(0).toString());
            }while(query.next());

            ui->CB_unlink_bank_account_num->clear();
            query.clear();
            query.prepare("SELECT DISTINCT BANumber "
                          "FROM   HAS_ADDITIONAL "
                          "WHERE  SSN = :ssn and BankID = :bid");
            query.bindValue(":ssn", userssn);
            query.bindValue(":bid", ui->CB_unlink_bank_account_id->currentText());
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
            if(query.next()){
                do{
                    ui->CB_unlink_bank_account_num->addItem(query.value(0).toString());
                }while(query.next());
            }
        }
    }
    else if(index == 4){
        ui->Sub_TB->setCurrentIndex(7);
        ui->CB_verify_bank_account_id->clear();
        query.prepare("SELECT DISTINCT BankID "
                      "FROM   HAS_ADDITIONAL "
                      "WHERE  SSN = :ssn");
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            do{
                ui->CB_verify_bank_account_id->addItem(query.value(0).toString());
            }while(query.next());

            ui->CB_verify_bank_account_num->clear();
            query.clear();
            query.prepare("SELECT DISTINCT BANumber "
                          "FROM   HAS_ADDITIONAL "
                          "WHERE  SSN = :ssn and BankID = :bid");
            query.bindValue(":ssn", userssn);
            query.bindValue(":bid", ui->CB_verify_bank_account_id->currentText());
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
            if(query.next()){
                do{
                    ui->CB_verify_bank_account_num->addItem(query.value(0).toString());
                }while(query.next());
            }
        }
    }
    else if(index == 5){
        ui->Sub_TB->setCurrentIndex(10);
        ui->CB_set_primary_account_id->clear();
        query.prepare("SELECT DISTINCT BankID "
                      "FROM   HAS_ADDITIONAL "
                      "WHERE  SSN = :ssn");
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            do{
                ui->CB_set_primary_account_id->addItem(query.value(0).toString());
            }while(query.next());

            ui->CB_set_primary_account_num->clear();
            query.clear();
            query.prepare("SELECT DISTINCT BANumber "
                          "FROM   HAS_ADDITIONAL "
                          "WHERE  SSN = :ssn and BankID = :bid");
            query.bindValue(":ssn", userssn);
            query.bindValue(":bid", ui->CB_set_primary_account_id->currentText());
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
            else if(query.next()){
                do{
                    ui->CB_set_primary_account_num->addItem(query.value(0).toString());
                }while(query.next());
            }
        }
    }
    else if(index == 6){
        ui->Sub_TB->setCurrentIndex(12);
        ui->CB_take_money_bank_id->clear();
        query.prepare("SELECT DISTINCT BankID "
                      "FROM   HAS_ADDITIONAL "
                      "WHERE  SSN = :ssn and Verified = 'y'");
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            do{
                ui->CB_take_money_bank_id->addItem(query.value(0).toString());
            }while(query.next());

            ui->CB_take_money_bank_num->clear();
            query.clear();
            query.prepare("SELECT DISTINCT BANumber "
                          "FROM   HAS_ADDITIONAL "
                          "WHERE  SSN = :ssn and BankID = :bid and Verified = 'y'");
            query.bindValue(":ssn", userssn);
            query.bindValue(":bid", ui->CB_take_money_bank_id->currentText());
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
            else if(query.next()){
                do{
                    ui->CB_take_money_bank_num->addItem(query.value(0).toString());
                }while(query.next());
            }
        }
    }
}

void MainWindow::on_CB_transfer_money_activated(int index)
{
    QSqlQuery query(db);
    QMessageBox mb;
    QString date10min = "";
    // 1 Send, 2 Request, 3 Cancel Payment
    if(index != 0){
        ui->CB_account->setCurrentIndex(0);
        ui->CB_bank_account->setCurrentIndex(0);
    }
    if(index == 1){
        ui->Sub_TB->setCurrentIndex(8);

    }
    else if(index == 2){
        ui->Sub_TB->setCurrentIndex(9);
    }
    else if(index == 3){
        ui->Sub_TB->setCurrentIndex(11);
        ui->CB_cancel_payment_stid->clear();
        ui->CB_cancel_payment_stid->addItem("Select STid ID :");
        date10min += QDate::currentDate().toString(Qt::ISODate);
        date10min += " ";
        date10min += QTime::currentTime().addSecs(-600).toString();
        query.prepare("SELECT STid "
                      "FROM SEND_TRANSACTION "
                      "WHERE DateTime > :dt and SSN = :ssn and Cancelled = 'n'");
        query.bindValue(":dt",  date10min);
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        else if(query.next()){
            do{
                ui->CB_cancel_payment_stid->addItem(query.value(0).toString());
            }while(query.next());
        }
    }
}

void MainWindow::on_CB_account_activated(int index)
{
    // 1 Account Page, 2 History, 3 Verify Account, 4 Requested Money, 5 Link Identifier
    QSqlQuery query(db);
    QMessageBox mb;
    int currentrow;
    QString years = "", months = "", yeare = "", monthe = "";
    QString datetime = "";
    QStringList header;
    int j, jend;

    if(index != 0){
        ui->CB_bank_account->setCurrentIndex(0);
        ui->CB_transfer_money->setCurrentIndex(0);
    }
    if(index == 1){
        ui->Sub_TB->setCurrentIndex(0);
        update_front_page();
    }
    else if(index == 2){
        ui->Sub_TB->setCurrentIndex(1);
        ui->CB_history_month->clear();
        ui->CB_history_month->addItem("All");
        ui->TW_history->clear();
        //SET MIN DATE
        query.prepare("SELECT MIN(DateTime) "
                      "FROM   SEND_TRANSACTION ");
        query.bindValue(":un", ui->LE_s_username->text());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            years = query.value(0).toString().mid(0,4);
            months = query.value(0).toString().mid(5,2);
        }
        query.clear();
        query.prepare("SELECT MIN(DateTime) "
                      "FROM   REQUEST_TRANSACTION ");
        query.bindValue(":un", ui->LE_s_username->text());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            if(query.value(0).toString().mid(0,4).toInt() < years.toInt()){
                years = query.value(0).toString().mid(0,4);
                months = query.value(0).toString().mid(5,2);
            }
            else if(query.value(0).toString().mid(0,4).toInt() == years.toInt()){
                if(query.value(0).toString().mid(5,2).toInt() < months.toInt()){
                    months = query.value(0).toString().mid(5,2);
                }
            }

        }
        query.clear();

        //SET MAX DATE
        query.prepare("SELECT MAX(DateTime) "
                      "FROM   SEND_TRANSACTION ");
        query.bindValue(":un", ui->LE_s_username->text());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            yeare = query.value(0).toString().mid(0,4);
            monthe = query.value(0).toString().mid(5,2);
        }
        query.clear();
        query.prepare("SELECT MAX(DateTime) "
                      "FROM   REQUEST_TRANSACTION ");
        query.bindValue(":un", ui->LE_s_username->text());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            if(query.value(0).toString().mid(0,4).toInt() > yeare.toInt()){
                yeare = query.value(0).toString().mid(0,4);
                monthe = query.value(0).toString().mid(5,2);
            }
            else if(query.value(0).toString().mid(0,4).toInt() == yeare.toInt()){
                if(query.value(0).toString().mid(5,2).toInt() > monthe.toInt()){
                    monthe = query.value(0).toString().mid(5,2);
                }
            }
        }
        query.clear();

        for(int i = years.toInt(); i != yeare.toInt()+1; ++i){
            if(i == years.toInt()) j = months.toInt();
            else j = 1;

            if(i == yeare.toInt()) jend = monthe.toInt()+1;
            else jend = 13;

            for(; j != jend; ++j){
                datetime = "";
                datetime += QString::number(i);
                datetime += "-";
                if(j < 10){
                    datetime += "0";
                }
                datetime += QString::number(j);

                ui->CB_history_month->addItem(datetime);
            }
        }
    }
    else if(index == 3){
        ui->Sub_TB->setCurrentIndex(2);
        ui->CB_verify_account_email->clear();
        ui->CB_verify_account_email->addItem("Unverified Email:");
        ui->CB_verify_account_phone->clear();
        ui->CB_verify_account_phone->addItem("Unverified Phone:");
        query.prepare("SELECT Type, Identifier "
                      "FROM   ELECTRONIC_ADDRESS "
                      "WHERE  SSN = :ssn and Verified = 'n'");
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            do{
                if(query.value(0).toString() == "e"){
                    ui->CB_verify_account_email->addItem(query.value(1).toString());
                }
                else{
                    ui->CB_verify_account_phone->addItem(query.value(1).toString());
                }
            } while(query.next());
        }
    }
    else if(index == 4){
        ui->Sub_TB->setCurrentIndex(3);
        ui->TW_requested_money->clear();
        ui->TW_requested_money->setColumnCount(8);
        header << tr("RTid") << tr("Amount") << tr("Date/Time") << tr("Memo") << tr("Name") << tr("Percentage") << tr("Accept") << tr("Deny");
        ui->TW_requested_money->setHorizontalHeaderLabels(header);
        ui->TW_requested_money->setRowCount(0);
        query.prepare("SELECT RT.RTid, RT.Amount, RT.DateTime, RT.Memo, UA.Name, F.Percentage "
                      "FROM   USER_ACCOUNT AS UA, ELECTRONIC_ADDRESS AS EA, REQUEST_TRANSACTION AS RT, FROM_ AS F "
                      "WHERE  EA.SSN = :ssn and EA.Identifier = F.Identifier and F.RTid = RT.RTid and UA.SSN = RT.SSN and F.Status = 'p'");
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            do{
                currentrow = ui->TW_requested_money->rowCount();
                ui->TW_requested_money->setRowCount(currentrow+1);
                ui->TW_requested_money->setItem(currentrow, 0, new QTableWidgetItem(query.value(0).toString()));
                ui->TW_requested_money->setItem(currentrow, 1, new QTableWidgetItem(query.value(1).toString()));
                ui->TW_requested_money->setItem(currentrow, 2, new QTableWidgetItem(query.value(2).toString()));
                ui->TW_requested_money->setItem(currentrow, 3, new QTableWidgetItem(query.value(3).toString()));
                ui->TW_requested_money->setItem(currentrow, 4, new QTableWidgetItem(query.value(4).toString()));
                ui->TW_requested_money->setItem(currentrow, 5, new QTableWidgetItem(query.value(5).toString()));
                ui->TW_requested_money->setItem(currentrow, 6, new QTableWidgetItem("Accept"));
                ui->TW_requested_money->setItem(currentrow, 7, new QTableWidgetItem("Deny"));
            }while(query.next());
        }
    }
    else if(index == 5){
        ui->Sub_TB->setCurrentIndex(13);
    }
}

void MainWindow::on_PB_link_bank_account_link_clicked()
{
    QMessageBox mb;
    QSqlQuery query(db);
    int i;
    bool invalid = false;

    if(ui->LE_link_bank_account_bank_id->text().size() == 9 &&
       ui->LE_link_bank_account_bank_number->text().size() == 9){
        for(i = 0; i != 9; ++i){
            if(ui->LE_link_bank_account_bank_id->text()[i] < '0' &&
               ui->LE_link_bank_account_bank_id->text()[i] > '9'){
                invalid = true;
            }
            if(ui->LE_link_bank_account_bank_number->text()[i] < '0' &&
               ui->LE_link_bank_account_bank_number->text()[i] > '9'){
                invalid = true;
            }
        }
    }
    else{
        invalid = true;
    }

    if(invalid){
        mb.setText("Invalid Bank ID or Bank number!");
        mb.exec();
    }
    else{
        //FIND BANK ACCOUNT
        query.prepare("SELECT * "
                      "FROM   BANK_ACCOUNT "
                      "WHERE  BankID = :bid and BANumber = :bn");
        query.bindValue(":bid", ui->LE_link_bank_account_bank_id->text());
        query.bindValue(":bn",  ui->LE_link_bank_account_bank_number->text());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        else if(!query.next()){
            query.clear();
            query.prepare("INSERT INTO BANK_ACCOUNT "
                          "VALUES (:bid, :bn)");
            query.bindValue(":bid", ui->LE_link_bank_account_bank_id->text());
            query.bindValue(":bn",  ui->LE_link_bank_account_bank_number->text());
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
        }

        query.clear();
        //FIND HAS_ADDITIONAL
        query.prepare("SELECT * "
                      "FROM   HAS_ADDITIONAL "
                      "WHERE  SSN = :ssn and BankID = :bid and BANumber = :bn");
        query.bindValue(":ssn", userssn);
        query.bindValue(":bid", ui->LE_link_bank_account_bank_id->text());
        query.bindValue(":bn",  ui->LE_link_bank_account_bank_number->text());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        else if(query.next()){
            mb.setText("You already linked this Bank Account.");
            mb.exec();
            ui->LE_link_bank_account_bank_id->setText("");
            ui->LE_link_bank_account_bank_number->setText("");
        }
        else{
            query.clear();
            query.prepare("INSERT INTO HAS_ADDITIONAL "
                          "VALUES (:ssn, :bid, :bn, 'n')");
            query.bindValue(":ssn", userssn);
            query.bindValue(":bid", ui->LE_link_bank_account_bank_id->text());
            query.bindValue(":bn",  ui->LE_link_bank_account_bank_number->text());
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
            mb.setText("Bank Account Linked success!!");
            mb.exec();
            ui->LE_link_bank_account_bank_id->setText("");
            ui->LE_link_bank_account_bank_number->setText("");
        }
    }
}

void MainWindow::on_CB_history_type_activated(int index)
{/*
    QMessageBox mb;
    QSqlQuery query(db);
    QStringList header;
    int currentrow;
    ui->TW_history->clear();
    ui->TW_history->setRowCount(0);

    if(index == 1){//send
        ui->TW_history->setColumnCount(7);
        header << tr("STID") << tr("Amount") << tr("DateTime") << tr("Memo")
               << tr("Cancelled") << tr("U_SSN") << tr("Identifier");
        ui->TW_history->setHorizontalHeaderLabels(header);

        query.prepare("SELECT * "
                      "FROM SEND_TRANSACTION "
                      "WHERE SSN = :ssn "
                      "ORDER BY DateTime DESC");
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        else if(query.next()){
            do{
                currentrow = ui->TW_history->rowCount();
                ui->TW_history->setRowCount(currentrow+1);
                ui->TW_history->setItem(currentrow, 0, new QTableWidgetItem(query.value(0).toString()));
                ui->TW_history->setItem(currentrow, 1, new QTableWidgetItem(query.value(1).toString()));
                ui->TW_history->setItem(currentrow, 2, new QTableWidgetItem(query.value(2).toString()));
                ui->TW_history->setItem(currentrow, 3, new QTableWidgetItem(query.value(3).toString()));
                ui->TW_history->setItem(currentrow, 4, new QTableWidgetItem(query.value(4).toString()));
                ui->TW_history->setItem(currentrow, 5, new QTableWidgetItem(query.value(5).toString()));
                ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem(query.value(6).toString()));
            }while(query.next());
        }
    }
    else if (index == 2){//request
        ui->TW_history->setColumnCount(8);
        header << tr("RTID") << tr("Amount") << tr("DateTime") << tr("Identifier") << tr("Name")
               << tr("Percenage") << tr("Status") << tr("Memo");
        ui->TW_history->setHorizontalHeaderLabels(header);
        query.prepare("SELECT RT.RTid, RT.Amount, RT.DateTime, F.Identifier, UA.Name, F.Percentage, F.Status, RT.Memo "
                      "FROM ELECTRONIC_ADDRESS AS EA, FROM_ AS F, REQUEST_TRANSACTION AS RT, USER_ACCOUNT AS UA "
                      "WHERE RT.SSN = :ssn and RT.RTid = F.RTid and F.Identifier = EA.Identifier and EA.SSN = UA.SSN "
                      "ORDER BY DateTime DESC");
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        else if(query.next()){
            do{
                currentrow = ui->TW_history->rowCount();
                ui->TW_history->setRowCount(currentrow+1);
                ui->TW_history->setItem(currentrow, 0, new QTableWidgetItem(query.value(0).toString()));
                ui->TW_history->setItem(currentrow, 1, new QTableWidgetItem(query.value(1).toString()));
                ui->TW_history->setItem(currentrow, 2, new QTableWidgetItem(query.value(2).toString()));
                ui->TW_history->setItem(currentrow, 3, new QTableWidgetItem(query.value(3).toString()));
                ui->TW_history->setItem(currentrow, 4, new QTableWidgetItem(query.value(4).toString()));
                ui->TW_history->setItem(currentrow, 5, new QTableWidgetItem(query.value(5).toString()));
                if(query.value(6).toString() == "p")
                    ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Pending"));
                if(query.value(6).toString() == "a")
                    ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Accept"));
                if(query.value(6).toString() == "d")
                    ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Deny"));
                ui->TW_history->setItem(currentrow, 7, new QTableWidgetItem(query.value(7).toString()));
            }while(query.next());
        }
    }*/
}

void MainWindow::on_PB_request_money_add_clicked()
{
    QMessageBox mb;
    QSqlQuery query(db);
    QString name;
    int count = ui->TW_request_money->rowCount();
    bool invalid = false;

    //CHECK BANK_ACCOUNT
    query.prepare("SELECT Name "
                  "FROM ELECTRONIC_ADDRESS AS EA, USER_ACCOUNT AS UA "
                  "WHERE Identifier = :iden and EA.SSN = UA.SSN and EA.SSN <> :ssn");
    query.bindValue(":iden", ui->LE_request_money_identifier->text());
    query.bindValue(":ssn", userssn);
    if(!query.exec()){
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    else if(!query.next()){
        invalid = true;
        mb.setText("The identifier is not register yet or cannot input your account!");
        mb.exec();
    }
    else{
        name = query.value(0).toString();
    }

    //DUPLICATE ACCOUNT
    for(int i = 0; i != count; ++i){
        if(ui->TW_request_money->item(i, 0)->text() == ui->LE_request_money_identifier->text()){
            invalid = true;
            mb.setText("Already have this account.");
            mb.exec();
            break;
        }
    }

    if(ui->LE_request_money_identifier->text() != "" && !invalid){
        ui->TW_request_money->setRowCount(count+1);
        ui->TW_request_money->setItem(count, 0, new QTableWidgetItem(ui->LE_request_money_identifier->text()));
        ui->TW_request_money->setItem(count, 1, new QTableWidgetItem(name));
        ui->TW_request_money->setItem(count, 2, new QTableWidgetItem(""));
        ui->TW_request_money->setItem(count, 4, new QTableWidgetItem("Delete"));
    }
    ui->LE_request_money_identifier->setText("");
}

void MainWindow::on_PB_verify_bank_account_verify_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;

    if(ui->LE_verify_bank_account_code->text().toDouble() > 0){
        //UPDATE HAS_ADDITIONAL -> Verified
        query.prepare("UPDATE HAS_ADDITIONAL "
                      "SET Verified = 'y' "
                      "WHERE SSN = :ssn and BankID = :bid and BANumber = :bn");
        query.bindValue(":ssn", userssn);
        query.bindValue(":bid", ui->CB_verify_bank_account_id->currentText());
        query.bindValue(":bn",  ui->CB_verify_bank_account_num->currentText());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        //UPDATE USER_ACCOUN -> Balance
        query.prepare("UPDATE USER_ACCOUNT "
                      "SET Balance = Balance + :bal "
                      "WHERE SSN = :ssn");
        query.bindValue(":ssn", userssn);
        query.bindValue(":bal", ui->LE_verify_bank_account_code->text().toDouble());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        //UPDATE USER_ACCOUN -> Primary Bank Account Verified
        query.prepare("UPDATE USER_ACCOUNT "
                      "SET PBAVerified = 'y' "
                      "WHERE SSN = :ssn and BankID = :bid and BANumber = :bn");
        query.bindValue(":ssn", userssn);
        query.bindValue(":bid", ui->CB_verify_bank_account_id->currentText());
        query.bindValue(":bn",  ui->CB_verify_bank_account_id->currentText());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        mb.setText("Deposit success!");
        mb.exec();
        //update: bank account verify(HAS_ADDITIONAL, USER_ACCOUNT).
        //update: delete CB_verify_bank account id and number
    }
    else{
        mb.setText("Wrong verified code!");
        mb.exec();
    }
}

void MainWindow::on_PB_send_money_send_clicked()
{
    QMessageBox mb;
    QSqlQuery query(db);
    QString STid = "", sendssn = "", datetime = "";
    bool weeklypayment = true;
    double userbalance = 0, amount = 0;

    //UNIQUE RANDOM STid
    do{
        STid = "";
        for(int i = 0; i != 9; ++i){
            STid += QString::number(std::rand()%10);
        }
        query.prepare("SELECT STid "
                      "FROM   SEND_TRANSACTION "
                      "WHERE  STid = :stid");
        query.bindValue(":stid", STid);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
    }while(query.next());

    //WEEKLY PAYMENT
    datetime = "";
    datetime += QDate::currentDate().addDays(-7).toString(Qt::ISODate);
    datetime += " ";
    datetime += QTime::currentTime().toString();
    query.prepare("SELECT Amount "
                  "FROM   SEND_TRANSACTION "
                  "WHERE  SSN = :ssn and Cancelled = 'n' and DateTime > :dt");
    query.bindValue(":ssn", userssn);
    query.bindValue(":dt", datetime);
    if(!query.exec()){
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    else if(query.next()){
        do{
            amount += query.value(0).toDouble();
        }while(query.next());
        if(amount + ui->LE_send_money_amount->text().toDouble() > 299.99){
            weeklypayment = false;
            mb.setText("Exceed weekly rolling limit!");
            mb.exec();
        }
    }
    else if(ui->LE_send_money_amount->text().toDouble() > 299.99){
            weeklypayment = false;
            mb.setText("Exceed weekly rolling limit!");
            mb.exec();
    }

    if(weeklypayment){
        if(ui->LE_send_money_amount->text().toDouble() > 0 &&
                ui->LE_send_money_identifier->text() != ""){
            //UPDATE:wrong phone num or email >> 15 days
            //UPDATE:299.99 per week
            datetime = "";
            datetime += QDate::currentDate().toString(Qt::ISODate);
            datetime += " ";
            datetime += QTime::currentTime().toString();
            query.clear();
            query.prepare("INSERT INTO SEND_TRANSACTION "  //(STid, Amount, DateTime, Memo, SSN, Identifier) "
                          "VALUES (:stid, :amount, :dt, :memo, 'n', :ssn, :iden)");
            query.bindValue(":stid",   STid);
            query.bindValue(":amount", ui->LE_send_money_amount->text().toDouble());
            query.bindValue(":dt",     datetime);
            query.bindValue(":memo",   ui->LE_send_money_memo->text());
            query.bindValue(":ssn",    userssn);
            query.bindValue(":iden",   ui->LE_send_money_identifier->text());
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
            else{
                //FIND USER CURRENT BALANCE
                query.clear();
                query.prepare("SELECT Balance "
                              "FROM USER_ACCOUNT "
                              "WHERE SSN = :ssn");
                query.bindValue(":ssn",    userssn);
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    userbalance = query.value(0).toDouble();
                }
                if(userbalance < ui->LE_send_money_amount->text().toDouble()){
                    mb.setText("We will fund the entire payment from your primary bank.");
                    mb.exec();
                }
                else{
                    //- USER MONEY
                    query.clear();
                    query.prepare("UPDATE USER_ACCOUNT "
                                  "SET Balance = Balance - :amount "
                                  "WHERE SSN = :ssn");
                    query.bindValue(":amount", ui->LE_send_money_amount->text());
                    query.bindValue(":ssn", userssn);
                    if(!query.exec()){
                        mb.setText(query.lastError().text());
                        mb.exec();
                        qDebug() << query.lastError();
                    }
                }

                //CHECK IF ELECTRONIC_ADDRESS EXIST
                query.clear();
                query.prepare("SELECT SSN "
                              "FROM ELECTRONIC_ADDRESS "
                              "WHERE Identifier = :iden");
                query.bindValue(":iden", ui->LE_send_money_identifier->text());
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    sendssn = query.value(0).toString();
                }
                if(sendssn.length() == 9){
                    //+ RECEIVER MONEY
                    query.clear();
                    query.prepare("UPDATE USER_ACCOUNT "
                                  "SET Balance = Balance + :amount "
                                  "WHERE SSN = :ssn");
                    query.bindValue(":amount", ui->LE_send_money_amount->text());
                    query.bindValue(":ssn",    sendssn);
                    if(!query.exec()){
                        mb.setText(query.lastError().text());
                        mb.exec();
                        qDebug() << query.lastError();
                    }
                    else if(query.next()){
                        sendssn = query.value(0).toString();
                    }
                }
                else{//UPDATE: UNREGISTER IDENTIFIER, NEED UPDATE MONEY

                }

                ui->LE_send_money_amount->setText("");
                ui->LE_send_money_identifier->setText("");
                ui->LE_send_money_memo->setText("");
                mb.setText("Send money successed!");
                mb.exec();
            }
        }
        else{
            mb.setText("Send money deny!");
            mb.exec();
        }
    }
}

void MainWindow::on_CB_unlink_bank_account_id_activated(const QString &arg1)
{
    QSqlQuery query(db);
    QMessageBox mb;
    ui->CB_unlink_bank_account_num->clear();
    if(arg1.length() == 9){
        query.prepare("SELECT BANumber "
                      "FROM   HAS_ADDITIONAL "
                      "WHERE  SSN = :ssn and BankID = :bid");
        query.bindValue(":ssn", userssn);
        query.bindValue(":bid", arg1);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            do{
                ui->CB_unlink_bank_account_num->addItem(query.value(0).toString());
            }while(query.next());
        }
    }
}

void MainWindow::on_PB_unlink_bank_account_unlink_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;

    query.prepare("DELETE FROM HAS_ADDITIONAL "
                  "WHERE SSN = :ssn and BankID = :bid and BANumber = :bn");
    query.bindValue(":ssn", userssn);
    query.bindValue(":bid", ui->CB_unlink_bank_account_id->currentText());
    query.bindValue(":bn",  ui->CB_unlink_bank_account_num->currentText());
    if(!query.exec()){
        mb.setText("Invalid Bank ID or Bank Number!");
        mb.exec();
        qDebug() << query.lastError();
    }
    query.clear();

    //on_CB_bank_account_activated(3)
    ui->CB_unlink_bank_account_id->clear();
    query.prepare("SELECT DISTINCT BankID "
                  "FROM   HAS_ADDITIONAL "
                  "WHERE  SSN = :ssn");
    query.bindValue(":ssn", userssn);
    if(!query.exec()){
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    if(query.next()){
        do{
            ui->CB_unlink_bank_account_id->addItem(query.value(0).toString());
        }while(query.next());

        ui->CB_unlink_bank_account_num->clear();
        query.clear();
        query.prepare("SELECT DISTINCT BANumber "
                      "FROM   HAS_ADDITIONAL "
                      "WHERE  SSN = :ssn and BankID = :bid");
        query.bindValue(":ssn", userssn);
        query.bindValue(":bid", ui->CB_unlink_bank_account_id->currentText());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            do{
                ui->CB_unlink_bank_account_num->addItem(query.value(0).toString());
            }while(query.next());
        }
    }
}

void MainWindow::on_CB_verify_bank_account_id_activated(const QString &arg1)
{
    QSqlQuery query(db);
    QMessageBox mb;
    ui->CB_verify_bank_account_num->clear();
    if(arg1.length() == 9){
        query.prepare("SELECT BANumber "
                      "FROM   HAS_ADDITIONAL "
                      "WHERE  SSN = :ssn and BankID = :bid");
        query.bindValue(":ssn", userssn);
        query.bindValue(":bid", arg1);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            do{
                ui->CB_verify_bank_account_num->addItem(query.value(0).toString());
            }while(query.next());
        }
    }
}

void MainWindow::on_CB_set_primary_account_id_activated(const QString &arg1)
{
    QSqlQuery query(db);
    QMessageBox mb;
    ui->CB_set_primary_account_num->clear();
    if(arg1.length() == 9){
        query.prepare("SELECT BANumber "
                      "FROM   HAS_ADDITIONAL "
                      "WHERE  SSN = :ssn and BankID = :bid");
        query.bindValue(":ssn", userssn);
        query.bindValue(":bid", arg1);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        if(query.next()){
            do{
                ui->CB_set_primary_account_num->addItem(query.value(0).toString());
            }while(query.next());
        }
    }
}

void MainWindow::on_PB_set_primary_account_set_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;
    QString PBAVerified;

    query.prepare("SELECT Verified "
                  "FROM HAS_ADDITIONAL "
                  "WHERE SSN = :ssn and BankID = :bid and BANumber = :bn");
    query.bindValue(":ssn", userssn);
    query.bindValue(":bid", ui->CB_set_primary_account_id->currentText());
    query.bindValue(":bn",  ui->CB_set_primary_account_num->currentText());
    if(!query.exec()){
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    else if(query.next()){
        PBAVerified = query.value(0).toString();
    }
    else{
        mb.setText("Null value in Verified.");
        mb.exec();
    }
    query.clear();

    query.prepare("UPDATE USER_ACCOUNT "
                  "SET BankID = :bid, BANumber = :bn, PBAVerified = :pbav "
                  "WHERE SSN = :ssn");
    query.bindValue(":ssn", userssn);
    query.bindValue(":bid", ui->CB_set_primary_account_id->currentText());
    query.bindValue(":bn",  ui->CB_set_primary_account_num->currentText());
    query.bindValue(":pbav",  PBAVerified);
    if(!query.exec()){
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    else{

        mb.setText("Setting Primary Account Succeed!");
        mb.exec();
    }
}

void MainWindow::on_PB_verify_account_email_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;

    if(ui->CB_verify_account_email->currentText() != "Unverified Email:"){
        if(ui->LE_verify_account_email_code->text() == "1234"){
            query.prepare("UPDATE ELECTRONIC_ADDRESS "
                          "SET Verified = 'y' "
                          "WHERE SSN = :ssn and Identifier = :iden");
            query.bindValue(":ssn", userssn);
            query.bindValue(":iden", ui->CB_verify_account_email->currentText());
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
            mb.setText("Email verified.");
            mb.exec();
        }
        else{
            mb.setText("Invalid code.");
            mb.exec();
        }
    }
    ui->CB_verify_account_email->clear();
    ui->CB_verify_account_email->addItem("Unverified Email:");
    query.prepare("SELECT Type, Identifier "
                  "FROM   ELECTRONIC_ADDRESS "
                  "WHERE  SSN = :ssn and Verified = 'n'");
    query.bindValue(":ssn", userssn);
    if(!query.exec()){
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    if(query.next()){
        do{
            if(query.value(0).toString() == "e"){
                ui->CB_verify_account_email->addItem(query.value(1).toString());
            }
        } while(query.next());
    }
    ui->LE_verify_account_email_code->setText("");
}

void MainWindow::on_PB_verify_account_phone_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;

    if(ui->CB_verify_account_phone->currentText() != "Unverified Phone:"){
        if(ui->LE_verify_account_phone_code->text() == "1234"){
            query.prepare("UPDATE ELECTRONIC_ADDRESS "
                          "SET Verified = 'y' "
                          "WHERE SSN = :ssn and Identifier = :iden");
            query.bindValue(":ssn", userssn);
            query.bindValue(":iden", ui->CB_verify_account_phone->currentText());
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
            mb.setText("Phone verified.");
            mb.exec();
        }
        else{
            mb.setText("Invalid code.");
            mb.exec();
        }
    }
    ui->CB_verify_account_phone->clear();
    ui->CB_verify_account_phone->addItem("Unverified Phone:");
    query.prepare("SELECT Type, Identifier "
                  "FROM   ELECTRONIC_ADDRESS "
                  "WHERE  SSN = :ssn and Verified = 'n'");
    query.bindValue(":ssn", userssn);
    if(!query.exec()){
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    if(query.next()){
        do{
            if(query.value(0).toString() == "p"){
                ui->CB_verify_account_phone->addItem(query.value(1).toString());
            }
        } while(query.next());
    }
    ui->LE_verify_account_phone_code->setText("");
}

void MainWindow::on_CB_cancel_payment_stid_activated(const QString &arg1)
{
    QSqlQuery query(db);
    QMessageBox mb;
    if(arg1.length() == 9){
        query.prepare("SELECT Amount, Memo, Identifier "
                      "FROM SEND_TRANSACTION "
                      "WHERE STid = :stid and SSN = :ssn");
        query.bindValue(":stid",  arg1);
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        else if(query.next()){
            do{
                ui->LE_cancel_payment_amount->setText(query.value(0).toString());
                ui->LE_cancel_payment_memo->setText(query.value(1).toString());
                ui->LE_cancel_payment_type->setText(query.value(2).toString());
            }while(query.next());
        }
    }
}

void MainWindow::on_PB_cancel_payment_cancel_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;
    QString ssnsend, date10min = "";
    double amount = 0;
    if(ui->LE_cancel_payment_amount->text().toDouble() > 0 &&
            ui->LE_cancel_payment_type->text() != ""){
        //+MONEY
        query.prepare("UPDATE USER_ACCOUNT "
                      "SET Balance = Balance + :amount "
                      "WHERE SSN = :ssn");
        query.bindValue(":amount", ui->LE_cancel_payment_amount->text());
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        //MONEY AMOUNT
        query.clear();
        query.prepare("SELECT Balance "
                      "FROM USER_ACCOUNT AS UA, SEND_TRANSACTION AS ST "
                      "WHERE STid = :stid and UA.SSN = ST.SSN");
        query.bindValue(":stid", ui->CB_cancel_payment_stid->currentText());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        else if(query.next()){
            amount = query.value(0).toDouble();
        }
        else{
            mb.setText("Something wrong!");
            mb.exec();
        }
        //-MONEY
        if(amount < ui->LE_cancel_payment_amount->text().toDouble()){
            mb.setText("Will fund the entire payment from his/her primary bank!");
            mb.exec();
        }
        else{
            query.clear();
            query.prepare("UPDATE USER_ACCOUNT "
                          "SET Balance = Balance - :amount "
                          "WHERE SSN = (SELECT EA.SSN "
                          "             FROM SEND_TRANSACTION AS ST, ELECTRONIC_ADDRESS AS EA "
                          "             WHERE ST.STid = :stid and EA.Identifier = ST.Identifier)");
            query.bindValue(":amount", ui->LE_cancel_payment_amount->text());
            query.bindValue(":stid", ui->CB_cancel_payment_stid->currentText());
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
        }
        //CANCELLED = 'y'
        query.clear();
        query.prepare("UPDATE SEND_TRANSACTION "
                      "SET Cancelled = 'y' "
                      "WHERE STid = :stid");
        query.bindValue(":stid", ui->CB_cancel_payment_stid->currentText());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        mb.setText("Cancel Payment Success!");
        mb.exec();
        ui->LE_cancel_payment_amount->setText("");
        ui->LE_cancel_payment_memo->setText("");
        ui->LE_cancel_payment_type->setText("");
        ui->CB_cancel_payment_stid->clear();
        ui->CB_cancel_payment_stid->addItem("Select STid ID :");
        date10min += QDate::currentDate().toString(Qt::ISODate);
        date10min += " ";
        date10min += QTime::currentTime().addSecs(-600).toString();
        query.prepare("SELECT STid "
                      "FROM SEND_TRANSACTION "
                      "WHERE DateTime > :dt and SSN = :ssn and Cancelled = 'n'");
        query.bindValue(":dt",  date10min);
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        else if(query.next()){
            do{
                ui->CB_cancel_payment_stid->addItem(query.value(0).toString());
            }while(query.next());
        }
    }
}

void MainWindow::on_CB_take_money_bank_id_activated(const QString &arg1)
{
    QSqlQuery query(db);
    QMessageBox mb;

    ui->CB_take_money_bank_num->clear();
    if(arg1.length() == 9){
        query.clear();
        query.prepare("SELECT DISTINCT BANumber "
                      "FROM   HAS_ADDITIONAL "
                      "WHERE  SSN = :ssn and BankID = :bid and Verified = 'y'");
        query.bindValue(":ssn", userssn);
        query.bindValue(":bid", arg1);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        else if(query.next()){
            do{
                ui->CB_take_money_bank_num->addItem(query.value(0).toString());
            }while(query.next());
        }
    }
}

void MainWindow::on_PB_take_money_take_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;
    bool invalid = false;
    QString datetime;
    double amount = 0;

    //CHECK EMAIL AND PHONE ARE VERIFIED
    query.prepare("SELECT Verified "
                  "FROM   ELECTRONIC_ADDRESS "
                  "WHERE  SSN = :ssn");
    query.bindValue(":ssn", userssn);
    if(!query.exec()){
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    else if(query.next()){
        do{
            if(query.value(0) == "n"){
                invalid = true;
            }
        }while(query.next());
    }

    datetime = "";
    datetime += QDate::currentDate().addDays(-7).toString(Qt::ISODate);
    datetime += " ";
    datetime += QTime::currentTime().toString();

    if(invalid){
        //499.99
        if(ui->LE_take_money_amount->text().toDouble() > 499.99){
            mb.setText("Exceed one time transfer amount 499.99.");
            mb.exec();
        }
        else{
            //999.99
            query.clear();
            query.prepare("SELECT Amount "
                          "FROM   SEND_TO_BANK "
                          "WHERE  SSN = :ssn and DateTime > :dt");
            query.bindValue(":ssn", userssn);
            query.bindValue(":dt", datetime);
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
            else if(query.next()){
                do{
                    amount += query.value(0).toDouble();
                }while(query.next());
            }

            if(amount + ui->LE_take_money_amount->text().toDouble() > 999.99){
                mb.setText("Exceed weekly transfer amount 999.99.");
                mb.exec();
            }
            else{
                datetime = "";
                datetime += QDate::currentDate().toString(Qt::ISODate);
                datetime += " ";
                datetime += QTime::currentTime().toString();
                query.clear();
                query.prepare("INSERT INTO SEND_TO_BANK "
                              "VALUES (:ssn, :amou, :dt)");
                query.bindValue(":ssn", userssn);
                query.bindValue(":amou", ui->LE_take_money_amount->text().toDouble());
                query.bindValue(":dt", datetime);
                if(!query.exec()){
                    qDebug() << query.lastError();
                    mb.setText(query.lastError().text());
                    mb.exec();
                }
                //-MONEY
                query.clear();
                query.prepare("UPDATE USER_ACCOUNT "
                              "SET Balance = Balance - :amou "
                              "WHERE SSN = :ssn");
                query.bindValue(":ssn", userssn);
                query.bindValue(":amou", ui->LE_take_money_amount->text().toDouble());
                if(!query.exec()){
                    qDebug() << query.lastError();
                    mb.setText(query.lastError().text());
                    mb.exec();
                }
                mb.setText("Transfer success!");
                mb.exec();
            }
        }
    }
    else{
        //9999.99 19999.99
        if(ui->LE_take_money_amount->text().toDouble() > 9999.99){
            mb.setText("Exceed one time transfer amount 9999.99.");
            mb.exec();
        }
        else{
            //19999.99
            query.clear();
            query.prepare("SELECT Amount "
                          "FROM   SEND_TO_BANK "
                          "WHERE  SSN = :ssn and DateTime > :dt");
            query.bindValue(":ssn", userssn);
            query.bindValue(":dt", datetime);
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
            else if(query.next()){
                do{
                    amount += query.value(0).toDouble();
                }while(query.next());
            }

            if(amount + ui->LE_take_money_amount->text().toDouble() > 19999.99){
                mb.setText("Exceed weekly transfer amount 19999.99.");
                mb.exec();
            }
            else{
                datetime = "";
                datetime += QDate::currentDate().toString(Qt::ISODate);
                datetime += " ";
                datetime += QTime::currentTime().toString();
                query.clear();
                query.prepare("INSERT INTO SEND_TO_BANK "
                              "VALUES (:ssn, :amou, :dt)");
                query.bindValue(":ssn", userssn);
                query.bindValue(":amou", ui->LE_take_money_amount->text().toDouble());
                query.bindValue(":dt", datetime);
                if(!query.exec()){
                    qDebug() << query.lastError();
                    mb.setText(query.lastError().text());
                    mb.exec();
                }
                //-MONEY
                query.clear();
                query.prepare("UPDATE USER_ACCOUNT "
                              "SET Balance = Balance - :amou "
                              "WHERE SSN = :ssn");
                query.bindValue(":ssn", userssn);
                query.bindValue(":amou", ui->LE_take_money_amount->text().toDouble());
                if(!query.exec()){
                    qDebug() << query.lastError();
                    mb.setText(query.lastError().text());
                    mb.exec();
                }
                mb.setText("Transfer success!");
                mb.exec();
            }
        }
    }
    ui->LE_take_money_amount->setText("");
}

void MainWindow::on_PB_link_identifier_link_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;
    bool invalid = false;
    double amount = 0;
    QString datetime = "";

    //BASIC CHECK
    if(ui->CB_link_identifier_identifier->currentIndex() == 0){
        mb.setText("Choose Identifier Please!");
        mb.exec();
        invalid = true;
    }
    else if(ui->CB_link_identifier_identifier->currentIndex() == 1 &&
            ui->LE_link_identifier_identifier->text() == ""){
        mb.setText("Invalid email address!");
        mb.exec();
        invalid = true;
    }
    else if(ui->CB_link_identifier_identifier->currentIndex() == 2 &&
            ui->LE_link_identifier_identifier->text().length() != 10){
        mb.setText("Invalid phone number!");
        mb.exec();
        invalid = true;
    }

    if(!invalid){
        //CHECK IF IDENTIFIER REPEAT
        query.prepare("SELECT Identifier "
                      "FROM   ELECTRONIC_ADDRESS ");
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        else if(query.next()){
            do{
                if(query.value(0).toString() == ui->LE_link_identifier_identifier->text()){
                    invalid = true;
                    break;
                }
            }while(query.next());
        }
        if(invalid){
            mb.setText("Identifier is used!");
            mb.exec();
        }
        else{
            //LINK
            query.clear();
            query.prepare("INSERT INTO ELECTRONIC_ADDRESS "
                          "VALUES (:type, :ssn, :iden, 'n')");
            query.bindValue(":iden", ui->LE_link_identifier_identifier->text());
            query.bindValue(":ssn", userssn);
            if(ui->CB_link_identifier_identifier->currentIndex() == 1)
                query.bindValue(":type", "e");
            else
                query.bindValue(":type", "p");
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
            else{
                //CHECK IF SOMEONE ALREADY SEND YOU MONEY IN 15 DAYS.
                datetime = "";
                datetime += QDate::currentDate().addDays(-15).toString(Qt::ISODate);
                datetime += " ";
                datetime += QTime::currentTime().toString();
                //IDENTIFIER
                query.clear();
                query.prepare("SELECT Amount "
                              "FROM SEND_TRANSACTION "
                              "WHERE Identifier = :iden and Cancelled = 'n' and DateTime > :dt");
                query.bindValue(":iden", ui->LE_link_identifier_identifier->text());
                query.bindValue(":dt", datetime);
                if(!query.exec()){
                    qDebug() << query.lastError();
                    mb.setText(query.lastError().text());
                    mb.exec();
                }
                else if(query.next()){
                    do{
                        amount += query.value(0).toDouble();
                    }while(query.next());
                }
                query.clear();
                //+MONEY
                query.prepare("UPDATE USER_ACCOUNT "
                              "SET Balance = Balance + :bal "
                              "WHERE SSN = (SELECT SSN "
                              "             FROM ELECTRONIC_ADDRESS "
                              "             WHERE Identifier = :iden)");
                query.bindValue(":bal", amount);
                query.bindValue(":iden", ui->LE_link_identifier_identifier->text());
                if(!query.exec()){
                    qDebug() << query.lastError();
                    mb.setText(query.lastError().text());
                    mb.exec();
                }
                query.clear();

                mb.setText("Link Identifier!");
                mb.exec();
                ui->LE_link_identifier_identifier->setText("");
            }
        }
    }
}

void MainWindow::on_TW_request_money_cellClicked(int row, int column)
{
    if(column == 4){
        ui->TW_request_money->removeRow(row);
    }
}

void MainWindow::on_PB_request_money_request_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;
    QString RTid, datetime;
    QStringList header;
    double percentsum = 0;

    for(int i = 0; i != ui->TW_request_money->rowCount(); ++i){
        percentsum += ui->TW_request_money->item(i,2)->text().toDouble();
    }

    if(percentsum < 99.99999 || percentsum > 100.00001){
        mb.setText("The sum of percent must be 100!");
        mb.exec();
    }
    else if(ui->LE_request_money_amount->text().toDouble() < 0.000001){
        mb.setText("The request amount must bigger than 0!");
        mb.exec();
    }
    else{
        //UNIQUE RANDOM RTid
        do{
            RTid = "";
            for(int i = 0; i != 9; ++i){
                RTid += QString::number(std::rand()%10);
            }
            query.prepare("SELECT RTid "
                          "FROM   REQUEST_TRANSACTION "
                          "WHERE  RTid = :rtid");
            query.bindValue(":rtid", RTid);
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
        }while(query.next());

        datetime = "";
        datetime += QDate::currentDate().toString(Qt::ISODate);
        datetime += " ";
        datetime += QTime::currentTime().toString();

        query.clear();
        query.prepare("INSERT INTO REQUEST_TRANSACTION "
                      "VALUES (:rtid, :amount, :dt, :memo, :ssn)");
        query.bindValue(":rtid",   RTid);
        query.bindValue(":amount", ui->LE_request_money_amount->text().toDouble());
        query.bindValue(":dt",     datetime);
        query.bindValue(":memo",   ui->LE_request_money_memo->text());
        query.bindValue(":ssn",    userssn);
        if(!query.exec()){
            qDebug() << query.lastError();
            mb.setText(query.lastError().text());
            mb.exec();
        }

        for(int i = 0; i != ui->TW_request_money->rowCount(); ++i){
            query.clear();
            query.prepare("INSERT INTO FROM_ "
                          "VALUES (:rtid, :iden, :perc, 'p')");
            query.bindValue(":rtid", RTid);
            query.bindValue(":iden", ui->TW_request_money->item(i,0)->text());
            query.bindValue(":perc", ui->TW_request_money->item(i,2)->text().toDouble());
            if(!query.exec()){
                qDebug() << query.lastError();
                mb.setText(query.lastError().text());
                mb.exec();
            }
        }
        mb.setText("Request success!");
        mb.exec();
        ui->TW_request_money->clear();
        ui->LE_request_money_amount->clear();
        ui->LE_request_money_memo->clear();
        ui->LE_request_money_identifier->clear();
        ui->TW_request_money->setColumnCount(4);
        header << tr("Identifier") << tr("Name") << tr("Percentage") << tr("Amount") << tr("Delete");
        ui->TW_request_money->setHorizontalHeaderLabels(header);
        ui->TW_request_money->setRowCount(0);
    }
}

void MainWindow::on_TW_requested_money_cellClicked(int row, int column)
{
    QMessageBox mb;
    QSqlQuery query(db);
    double transferamount, amount = 0;
    QString iden;

    query.clear();
    query.prepare("SELECT EA.Identifier "
                  "FROM FROM_ AS F, ELECTRONIC_ADDRESS AS EA "
                  "WHERE EA.SSN = :ssn and F.Identifier = EA.Identifier and RTid = :rtid");
    query.bindValue(":ssn", userssn);
    query.bindValue(":rtid", ui->TW_requested_money->item(row, 0)->text());
    if(!query.exec()){
        mb.setText(query.lastError().text());
        mb.exec();
        qDebug() << query.lastError();
    }
    else if(query.next()){
        iden = query.value(0).toString();
    }

    query.clear();
    if(column == 6){
        //ACCEPT
        query.prepare("UPDATE FROM_ "
                      "SET Status = 'a' "
                      "WHERE RTid = :rtid and Identifier = :iden");
        query.bindValue(":rtid", ui->TW_requested_money->item(row, 0)->text());
        query.bindValue(":iden", iden);
        if(!query.exec()){
            qDebug() << query.lastError();
            mb.setText(query.lastError().text());
            mb.exec();
        }
        mb.setText("Request accept!");
        mb.exec();
        //+MONEY
        transferamount = ui->TW_requested_money->item(row, 1)->text().toDouble() * ui->TW_requested_money->item(row, 5)->text().toDouble() / 100;
        query.clear();
        query.prepare("UPDATE USER_ACCOUNT "
                      "SET Balance = Balance + :amount "
                      "WHERE SSN = (SELECT SSN "
                      "             FROM   REQUEST_TRANSACTION "
                      "             WHERE  RTid = :rtid)");
        query.bindValue(":amount", transferamount);
        query.bindValue(":rtid", ui->TW_requested_money->item(row, 0)->text());
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        //MONEY AMOUNT
        query.clear();
        query.prepare("SELECT Balance "
                      "FROM USER_ACCOUNT "
                      "WHERE SSN = :ssn");
        query.bindValue(":ssn", userssn);
        if(!query.exec()){
            mb.setText(query.lastError().text());
            mb.exec();
            qDebug() << query.lastError();
        }
        else if(query.next()){
            amount = query.value(0).toDouble();
        }
        else{
            mb.setText("Something wrong!");
            mb.exec();
        }
        //-MONEY
        if(amount < transferamount){
            mb.setText("Will fund the entire payment from your primary bank!");
            mb.exec();
        }
        else{
            query.clear();
            query.prepare("UPDATE USER_ACCOUNT "
                          "SET Balance = Balance - :amount "
                          "WHERE SSN = :ssn");
            query.bindValue(":amount", transferamount);
            query.bindValue(":ssn", userssn);
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
        }
        ui->TW_requested_money->removeRow(row);
    }
    if(column == 7){
        //DENY
        query.prepare("UPDATE FROM_ "
                      "SET Status = 'd' "
                      "WHERE RTid = :rtid and Identifier = :iden");
        query.bindValue(":rtid", ui->TW_requested_money->item(row, 0)->text());
        query.bindValue(":iden", iden);
        if(!query.exec()){
            qDebug() << query.lastError();
            mb.setText(query.lastError().text());
            mb.exec();
        }
        mb.setText("Request deny!");
        mb.exec();
        ui->TW_requested_money->removeRow(row);
    }
}

void MainWindow::on_PB_request_money_split_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;
    QString RTid, datetime;
    QStringList header;

    if(ui->LE_request_money_amount->text().toDouble() < 0.000001){
        mb.setText("The request amount must bigger than 0!");
        mb.exec();
    }
    else{
        //UNIQUE RANDOM RTid
        do{
            RTid = "";
            for(int i = 0; i != 9; ++i){
                RTid += QString::number(std::rand()%10);
            }
            query.prepare("SELECT RTid "
                          "FROM   REQUEST_TRANSACTION "
                          "WHERE  RTid = :rtid");
            query.bindValue(":rtid", RTid);
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
        }while(query.next());

        datetime = "";
        datetime += QDate::currentDate().toString(Qt::ISODate);
        datetime += " ";
        datetime += QTime::currentTime().toString();

        query.clear();
        query.prepare("INSERT INTO REQUEST_TRANSACTION "
                      "VALUES (:rtid, :amount, :dt, :memo, :ssn)");
        query.bindValue(":rtid",   RTid);
        query.bindValue(":amount", ui->LE_request_money_amount->text().toDouble());
        query.bindValue(":dt",     datetime);
        query.bindValue(":memo",   ui->LE_request_money_memo->text());
        query.bindValue(":ssn",    userssn);
        if(!query.exec()){
            qDebug() << query.lastError();
            mb.setText(query.lastError().text());
            mb.exec();
        }

        for(int i = 0; i != ui->TW_request_money->rowCount(); ++i){
            query.clear();
            query.prepare("INSERT INTO FROM_ "
                          "VALUES (:rtid, :iden, :perc, 'p')");
            query.bindValue(":rtid", RTid);
            query.bindValue(":iden", ui->TW_request_money->item(i,0)->text());
            query.bindValue(":perc", 100/ui->TW_request_money->rowCount());
            if(!query.exec()){
                qDebug() << query.lastError();
                mb.setText(query.lastError().text());
                mb.exec();
            }
        }
        mb.setText("Request success!");
        mb.exec();
        ui->TW_request_money->clear();
        ui->LE_request_money_amount->clear();
        ui->LE_request_money_memo->clear();
        ui->LE_request_money_identifier->clear();
        ui->TW_request_money->setColumnCount(5);
        header << tr("Identifier") << tr("Name") << tr("Percentage") << tr("Amount") << tr("Delete");
        ui->TW_request_money->setHorizontalHeaderLabels(header);
        ui->TW_request_money->setRowCount(0);
    }
}

void MainWindow::on_PB_request_money_amount_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;
    QString RTid, datetime;
    QStringList header;
    bool invalid = false;
    double totalamount = 0;

    //CHECK EVERY AMOUNT
    for(int i = 0; i != ui->TW_request_money->rowCount(); ++i){
        if(ui->TW_request_money->item(i, 3)->text().toDouble() <0.0000000001){
            invalid = true;
            break;
        }
        else{
            totalamount += ui->TW_request_money->item(i, 3)->text().toDouble();
        }
    }

    //UNIQUE RANDOM RTid
    if(invalid){
        mb.setText("Invalid amount in list!");
        mb.exec();
    }
    else{
        do{
            RTid = "";
            for(int i = 0; i != 9; ++i){
                RTid += QString::number(std::rand()%10);
            }
            query.prepare("SELECT RTid "
                          "FROM   REQUEST_TRANSACTION "
                          "WHERE  RTid = :rtid");
            query.bindValue(":rtid", RTid);
            if(!query.exec()){
                mb.setText(query.lastError().text());
                mb.exec();
                qDebug() << query.lastError();
            }
        }while(query.next());

        datetime = "";
        datetime += QDate::currentDate().toString(Qt::ISODate);
        datetime += " ";
        datetime += QTime::currentTime().toString();

        query.clear();
        query.prepare("INSERT INTO REQUEST_TRANSACTION "
                      "VALUES (:rtid, :amount, :dt, :memo, :ssn)");
        query.bindValue(":rtid",   RTid);
        query.bindValue(":amount", totalamount);
        query.bindValue(":dt",     datetime);
        query.bindValue(":memo",   ui->LE_request_money_memo->text());
        query.bindValue(":ssn",    userssn);
        if(!query.exec()){
            qDebug() << query.lastError();
            mb.setText(query.lastError().text());
            mb.exec();
        }

        for(int i = 0; i != ui->TW_request_money->rowCount(); ++i){
            query.clear();
            query.prepare("INSERT INTO FROM_ "
                          "VALUES (:rtid, :iden, :perc, 'p')");
            query.bindValue(":rtid", RTid);
            query.bindValue(":iden", ui->TW_request_money->item(i,0)->text());
            query.bindValue(":perc", 100*ui->TW_request_money->item(i,3)->text().toDouble()/totalamount);
            if(!query.exec()){
                qDebug() << query.lastError();
                mb.setText(query.lastError().text());
                mb.exec();
            }
        }
        mb.setText("Request success!");
        mb.exec();
        ui->TW_request_money->clear();
        ui->LE_request_money_amount->clear();
        ui->LE_request_money_memo->clear();
        ui->LE_request_money_identifier->clear();
        ui->TW_request_money->setColumnCount(5);
        header << tr("Identifier") << tr("Name") << tr("Percentage") << tr("Amount") << tr("Delete");
        ui->TW_request_money->setHorizontalHeaderLabels(header);
        ui->TW_request_money->setRowCount(0);
    }
}

void MainWindow::on_PB_history_search_clicked()
{
    QSqlQuery query(db);
    QMessageBox mb;
    double totalamount = 0;
    int currentrow = 0;
    QStringList header;
    QString dts = "", dte = "";
    bool invalid = false;

    // 1 account, 2 status, 3 date, 4 month, 5 recipient, 6 all
    int index = ui->CB_history_search_by->currentIndex(), type = ui->CB_history_type->currentIndex();
    int selectdt = ui->CB_history_month->currentIndex();

    if(ui->CB_history_type->currentIndex() == 0){
        mb.setText("Select Type Please!");
        mb.exec();
    }
    else{
        if(selectdt == 0){
            dts = "1991-01-01 00:00:00";
            dte = "2222-01-01 00:00:00";
        }
        else{
            dts = ui->CB_history_month->currentText();
            dts += "-01";
            dte = QDate::fromString(dts, Qt::ISODate).addMonths(+1).toString(Qt::ISODate);
            dts += " 00:00:00";
            dte += " 00:00:00";
        }

        if(index == 1){//account
            if(type == 1){
                ui->TW_history->clear();
                ui->TW_history->setRowCount(0);
                ui->TW_history->setColumnCount(7);
                header << tr("STID") << tr("Amount") << tr("DateTime") << tr("Memo")
                       << tr("Cancelled") << tr("U_SSN") << tr("Identifier");
                ui->TW_history->setHorizontalHeaderLabels(header);

                query.prepare("SELECT * "
                              "FROM SEND_TRANSACTION "
                              "WHERE SSN = :ssn and DateTime > :dts and DateTime < :dte and Identifier = :iden "
                              "ORDER BY DateTime DESC");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":iden", ui->LE_history_search->text());
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        currentrow = ui->TW_history->rowCount();
                        ui->TW_history->setRowCount(currentrow+1);
                        ui->TW_history->setItem(currentrow, 0, new QTableWidgetItem(query.value(0).toString()));
                        ui->TW_history->setItem(currentrow, 1, new QTableWidgetItem(query.value(1).toString()));
                        ui->TW_history->setItem(currentrow, 2, new QTableWidgetItem(query.value(2).toString()));
                        ui->TW_history->setItem(currentrow, 3, new QTableWidgetItem(query.value(3).toString()));
                        ui->TW_history->setItem(currentrow, 4, new QTableWidgetItem(query.value(4).toString()));
                        ui->TW_history->setItem(currentrow, 5, new QTableWidgetItem(query.value(5).toString()));
                        ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem(query.value(6).toString()));
                    }while(query.next());
                }
            }
            else if(type == 2){
                ui->TW_history->clear();
                ui->TW_history->setRowCount(0);
                ui->TW_history->setColumnCount(8);
                header << tr("RTID") << tr("Amount") << tr("DateTime") << tr("Identifier") << tr("Name")
                       << tr("Percenage") << tr("Status") << tr("Memo");
                ui->TW_history->setHorizontalHeaderLabels(header);

                query.prepare("SELECT RT.RTid, RT.Amount, RT.DateTime, F.Identifier, UA.Name, F.Percentage, F.Status, RT.Memo "
                              "FROM ELECTRONIC_ADDRESS AS EA, FROM_ AS F, REQUEST_TRANSACTION AS RT, USER_ACCOUNT AS UA "
                              "WHERE RT.SSN = :ssn and RT.RTid = F.RTid and F.Identifier = EA.Identifier and EA.SSN = UA.SSN "
                              "      and RT.DateTime > :dts and RT.Datetime < :dte and F.Identifier = :iden "
                              "ORDER BY DateTime DESC");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":iden", ui->LE_history_search->text());
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        currentrow = ui->TW_history->rowCount();
                        ui->TW_history->setRowCount(currentrow+1);
                        ui->TW_history->setItem(currentrow, 0, new QTableWidgetItem(query.value(0).toString()));
                        ui->TW_history->setItem(currentrow, 1, new QTableWidgetItem(query.value(1).toString()));
                        ui->TW_history->setItem(currentrow, 2, new QTableWidgetItem(query.value(2).toString()));
                        ui->TW_history->setItem(currentrow, 3, new QTableWidgetItem(query.value(3).toString()));
                        ui->TW_history->setItem(currentrow, 4, new QTableWidgetItem(query.value(4).toString()));
                        ui->TW_history->setItem(currentrow, 5, new QTableWidgetItem(query.value(5).toString()));
                        if(query.value(6).toString() == "p")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Pending"));
                        if(query.value(6).toString() == "a")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Accept"));
                        if(query.value(6).toString() == "d")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Deny"));
                        ui->TW_history->setItem(currentrow, 7, new QTableWidgetItem(query.value(7).toString()));
                    }while(query.next());
                }
            }
        }
        else if(index == 2){//status
            if(type == 1){
                ui->TW_history->clear();
                ui->TW_history->setRowCount(0);
                ui->TW_history->setColumnCount(7);
                header << tr("STID") << tr("Amount") << tr("DateTime") << tr("Memo")
                       << tr("Cancelled") << tr("U_SSN") << tr("Identifier");
                ui->TW_history->setHorizontalHeaderLabels(header);

                query.prepare("SELECT * "
                              "FROM SEND_TRANSACTION "
                              "WHERE SSN = :ssn and DateTime > :dts and DateTime < :dte and Cancelled = :canc "
                              "ORDER BY DateTime DESC");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":canc", ui->LE_history_search->text());
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        currentrow = ui->TW_history->rowCount();
                        ui->TW_history->setRowCount(currentrow+1);
                        ui->TW_history->setItem(currentrow, 0, new QTableWidgetItem(query.value(0).toString()));
                        ui->TW_history->setItem(currentrow, 1, new QTableWidgetItem(query.value(1).toString()));
                        ui->TW_history->setItem(currentrow, 2, new QTableWidgetItem(query.value(2).toString()));
                        ui->TW_history->setItem(currentrow, 3, new QTableWidgetItem(query.value(3).toString()));
                        ui->TW_history->setItem(currentrow, 4, new QTableWidgetItem(query.value(4).toString()));
                        ui->TW_history->setItem(currentrow, 5, new QTableWidgetItem(query.value(5).toString()));
                        ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem(query.value(6).toString()));
                    }while(query.next());
                }
            }
            else if(type == 2){
                ui->TW_history->clear();
                ui->TW_history->setRowCount(0);
                ui->TW_history->setColumnCount(8);

                header << tr("RTID") << tr("Amount") << tr("DateTime") << tr("Identifier") << tr("Name")
                       << tr("Percenage") << tr("Status") << tr("Memo");
                ui->TW_history->setHorizontalHeaderLabels(header);

                query.prepare("SELECT RT.RTid, RT.Amount, RT.DateTime, F.Identifier, UA.Name, F.Percentage, F.Status, RT.Memo "
                              "FROM ELECTRONIC_ADDRESS AS EA, FROM_ AS F, REQUEST_TRANSACTION AS RT, USER_ACCOUNT AS UA "
                              "WHERE RT.SSN = :ssn and RT.RTid = F.RTid and F.Identifier = EA.Identifier and EA.SSN = UA.SSN "
                              "      and RT.DateTime > :dts and RT.Datetime < :dte and F.Status = :stat "
                              "ORDER BY DateTime DESC");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":stat", ui->LE_history_search->text());
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        currentrow = ui->TW_history->rowCount();
                        ui->TW_history->setRowCount(currentrow+1);
                        ui->TW_history->setItem(currentrow, 0, new QTableWidgetItem(query.value(0).toString()));
                        ui->TW_history->setItem(currentrow, 1, new QTableWidgetItem(query.value(1).toString()));
                        ui->TW_history->setItem(currentrow, 2, new QTableWidgetItem(query.value(2).toString()));
                        ui->TW_history->setItem(currentrow, 3, new QTableWidgetItem(query.value(3).toString()));
                        ui->TW_history->setItem(currentrow, 4, new QTableWidgetItem(query.value(4).toString()));
                        ui->TW_history->setItem(currentrow, 5, new QTableWidgetItem(query.value(5).toString()));
                        if(query.value(6).toString() == "p")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Pending"));
                        if(query.value(6).toString() == "a")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Accept"));
                        if(query.value(6).toString() == "d")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Deny"));
                        ui->TW_history->setItem(currentrow, 7, new QTableWidgetItem(query.value(7).toString()));
                    }while(query.next());
                }
            }
        }
        else if(index == 3){//date
            if(ui->CB_history_type->currentIndex() == 0){
                mb.setText("Please select month!");
                mb.exec();
            }
            else{
                dts = ui->CB_history_month->currentText();
                dts += "-";
                if(ui->LE_history_search->text().toInt()  < 1){
                    mb.setText("Invalid date!");
                    mb.exec();
                    invalid = true;
                }
                else{
                    if(ui->LE_history_search->text().toInt()  < 10){
                        dts += "0";
                    }
                    dts += ui->LE_history_search->text();
                    if(QDate::fromString(dts, Qt::ISODate).isValid()){
                        dte = QDate::fromString(dts, Qt::ISODate).addDays(+1).toString(Qt::ISODate);
                        dts += " 00:00:00";
                        dte += " 00:00:00";
                    }
                    else{
                        mb.setText("Invalid date!");
                        mb.exec();
                        invalid = true;
                    }
                }
            }

            if(type == 1 && !invalid){
                ui->TW_history->clear();
                ui->TW_history->setRowCount(0);
                ui->TW_history->setColumnCount(2);
                header << tr("Total Amount") << tr("Cancelled");
                ui->TW_history->setHorizontalHeaderLabels(header);

                query.prepare("SELECT Amount "
                              "FROM SEND_TRANSACTION "
                              "WHERE SSN = :ssn and DateTime > :dts and DateTime < :dte and Cancelled = 'n'");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":canc", 'n');
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        totalamount += query.value(0).toDouble();
                    }while(query.next());
                }
                query.clear();

                ui->TW_history->setRowCount(2);
                ui->TW_history->setItem(0, 0, new QTableWidgetItem(QString::number(totalamount)));
                ui->TW_history->setItem(0, 1, new QTableWidgetItem("n"));
                totalamount = 0;

                query.prepare("SELECT Amount "
                              "FROM SEND_TRANSACTION "
                              "WHERE SSN = :ssn and DateTime > :dts and DateTime < :dte and Cancelled = 'y'");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":canc", 'y');
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        totalamount += query.value(0).toDouble();
                    }while(query.next());
                }
                query.clear();
                ui->TW_history->setItem(1, 0, new QTableWidgetItem(QString::number(totalamount)));
                ui->TW_history->setItem(1, 1, new QTableWidgetItem("y"));
            }
            else if(type == 2 && !invalid){
                ui->TW_history->clear();
                ui->TW_history->setRowCount(0);
                ui->TW_history->setColumnCount(2);
                header << tr("TotalAmount") << tr("Status");
                ui->TW_history->setHorizontalHeaderLabels(header);

                query.prepare("SELECT RT.Amount, F.Percentage "
                              "FROM ELECTRONIC_ADDRESS AS EA, FROM_ AS F, REQUEST_TRANSACTION AS RT, USER_ACCOUNT AS UA "
                              "WHERE RT.SSN = :ssn and RT.RTid = F.RTid and F.Identifier = EA.Identifier and EA.SSN = UA.SSN "
                              "      and RT.DateTime > :dts and RT.Datetime < :dte and F.Status = :stat ");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":stat", "a");
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        totalamount += query.value(0).toDouble()*query.value(1).toDouble()/100;
                    }while(query.next());
                }
                query.clear();

                ui->TW_history->setRowCount(3);
                ui->TW_history->setItem(0, 0, new QTableWidgetItem(QString::number(totalamount)));
                ui->TW_history->setItem(0, 1, new QTableWidgetItem("Accept"));
                totalamount = 0;

                query.prepare("SELECT RT.Amount, F.Percentage "
                              "FROM ELECTRONIC_ADDRESS AS EA, FROM_ AS F, REQUEST_TRANSACTION AS RT, USER_ACCOUNT AS UA "
                              "WHERE RT.SSN = :ssn and RT.RTid = F.RTid and F.Identifier = EA.Identifier and EA.SSN = UA.SSN "
                              "      and RT.DateTime > :dts and RT.Datetime < :dte and F.Status = :stat ");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":stat", "p");
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        totalamount += query.value(0).toDouble()*query.value(1).toDouble()/100;
                    }while(query.next());
                }
                query.clear();

                ui->TW_history->setItem(1, 0, new QTableWidgetItem(QString::number(totalamount)));
                ui->TW_history->setItem(1, 1, new QTableWidgetItem("Pending"));
                totalamount = 0;

                query.prepare("SELECT RT.Amount, F.Percentage "
                              "FROM ELECTRONIC_ADDRESS AS EA, FROM_ AS F, REQUEST_TRANSACTION AS RT, USER_ACCOUNT AS UA "
                              "WHERE RT.SSN = :ssn and RT.RTid = F.RTid and F.Identifier = EA.Identifier and EA.SSN = UA.SSN "
                              "      and RT.DateTime > :dts and RT.Datetime < :dte and F.Status = :stat ");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":stat", "d");
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        totalamount += query.value(0).toDouble()*query.value(1).toDouble()/100;
                    }while(query.next());
                }
                query.clear();
                ui->TW_history->setItem(2, 0, new QTableWidgetItem(QString::number(totalamount)));
                ui->TW_history->setItem(2, 1, new QTableWidgetItem("Deny"));
            }
        }
        else if(index == 4){//month
            if(type == 1){
                ui->TW_history->clear();
                ui->TW_history->setRowCount(0);
                ui->TW_history->setColumnCount(2);
                header << tr("Total Amount") << tr("Cancelled");
                ui->TW_history->setHorizontalHeaderLabels(header);

                query.prepare("SELECT Amount "
                              "FROM SEND_TRANSACTION "
                              "WHERE SSN = :ssn and DateTime > :dts and DateTime < :dte and Cancelled = 'n'");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":canc", 'n');
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        totalamount += query.value(0).toDouble();
                    }while(query.next());
                }
                query.clear();

                ui->TW_history->setRowCount(2);
                ui->TW_history->setItem(0, 0, new QTableWidgetItem(QString::number(totalamount)));
                ui->TW_history->setItem(0, 1, new QTableWidgetItem("n"));
                totalamount = 0;

                query.prepare("SELECT Amount "
                              "FROM SEND_TRANSACTION "
                              "WHERE SSN = :ssn and DateTime > :dts and DateTime < :dte and Cancelled = 'y'");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":canc", 'y');
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        totalamount += query.value(0).toDouble();
                    }while(query.next());
                }
                query.clear();
                ui->TW_history->setItem(1, 0, new QTableWidgetItem(QString::number(totalamount)));
                ui->TW_history->setItem(1, 1, new QTableWidgetItem("y"));
            }
            else if(type == 2){
                ui->TW_history->clear();
                ui->TW_history->setRowCount(0);
                ui->TW_history->setColumnCount(2);
                header << tr("TotalAmount") << tr("Status");
                ui->TW_history->setHorizontalHeaderLabels(header);

                query.prepare("SELECT RT.Amount, F.Percentage "
                              "FROM ELECTRONIC_ADDRESS AS EA, FROM_ AS F, REQUEST_TRANSACTION AS RT, USER_ACCOUNT AS UA "
                              "WHERE RT.SSN = :ssn and RT.RTid = F.RTid and F.Identifier = EA.Identifier and EA.SSN = UA.SSN "
                              "      and RT.DateTime > :dts and RT.Datetime < :dte and F.Status = :stat ");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":stat", "a");
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        totalamount += query.value(0).toDouble()*query.value(1).toDouble()/100;
                    }while(query.next());
                }
                query.clear();

                ui->TW_history->setRowCount(3);
                ui->TW_history->setItem(0, 0, new QTableWidgetItem(QString::number(totalamount)));
                ui->TW_history->setItem(0, 1, new QTableWidgetItem("Accept"));
                totalamount = 0;

                query.prepare("SELECT RT.Amount, F.Percentage "
                              "FROM ELECTRONIC_ADDRESS AS EA, FROM_ AS F, REQUEST_TRANSACTION AS RT, USER_ACCOUNT AS UA "
                              "WHERE RT.SSN = :ssn and RT.RTid = F.RTid and F.Identifier = EA.Identifier and EA.SSN = UA.SSN "
                              "      and RT.DateTime > :dts and RT.Datetime < :dte and F.Status = :stat ");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":stat", "p");
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        totalamount += query.value(0).toDouble()*query.value(1).toDouble()/100;
                    }while(query.next());
                }
                query.clear();

                ui->TW_history->setItem(1, 0, new QTableWidgetItem(QString::number(totalamount)));
                ui->TW_history->setItem(1, 1, new QTableWidgetItem("Pending"));
                totalamount = 0;

                query.prepare("SELECT RT.Amount, F.Percentage "
                              "FROM ELECTRONIC_ADDRESS AS EA, FROM_ AS F, REQUEST_TRANSACTION AS RT, USER_ACCOUNT AS UA "
                              "WHERE RT.SSN = :ssn and RT.RTid = F.RTid and F.Identifier = EA.Identifier and EA.SSN = UA.SSN "
                              "      and RT.DateTime > :dts and RT.Datetime < :dte and F.Status = :stat ");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":stat", "d");
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        totalamount += query.value(0).toDouble()*query.value(1).toDouble()/100;
                    }while(query.next());
                }
                query.clear();
                ui->TW_history->setItem(2, 0, new QTableWidgetItem(QString::number(totalamount)));
                ui->TW_history->setItem(2, 1, new QTableWidgetItem("Deny"));
            }
        }
        else if(index == 5){//recipient
            if(type == 1){
                ui->TW_history->clear();
                ui->TW_history->setRowCount(0);
                ui->TW_history->setColumnCount(7);
                header << tr("STID") << tr("Amount") << tr("DateTime") << tr("Memo")
                       << tr("Cancelled") << tr("U_SSN") << tr("Identifier");
                ui->TW_history->setHorizontalHeaderLabels(header);

                query.prepare("SELECT * "
                              "FROM SEND_TRANSACTION "
                              "WHERE SSN = :ssn and DateTime > :dts and DateTime < :dte and STid = :stid "
                              "ORDER BY DateTime DESC");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":stid", ui->LE_history_search->text());
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        currentrow = ui->TW_history->rowCount();
                        ui->TW_history->setRowCount(currentrow+1);
                        ui->TW_history->setItem(currentrow, 0, new QTableWidgetItem(query.value(0).toString()));
                        ui->TW_history->setItem(currentrow, 1, new QTableWidgetItem(query.value(1).toString()));
                        ui->TW_history->setItem(currentrow, 2, new QTableWidgetItem(query.value(2).toString()));
                        ui->TW_history->setItem(currentrow, 3, new QTableWidgetItem(query.value(3).toString()));
                        ui->TW_history->setItem(currentrow, 4, new QTableWidgetItem(query.value(4).toString()));
                        ui->TW_history->setItem(currentrow, 5, new QTableWidgetItem(query.value(5).toString()));
                        ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem(query.value(6).toString()));
                    }while(query.next());
                }
            }
            else if(type == 2){
                ui->TW_history->clear();
                ui->TW_history->setRowCount(0);
                ui->TW_history->setColumnCount(8);
                header << tr("RTID") << tr("Amount") << tr("DateTime") << tr("Identifier") << tr("Name")
                       << tr("Percenage") << tr("Status") << tr("Memo");
                ui->TW_history->setHorizontalHeaderLabels(header);

                query.prepare("SELECT RT.RTid, RT.Amount, RT.DateTime, F.Identifier, UA.Name, F.Percentage, F.Status, RT.Memo "
                              "FROM ELECTRONIC_ADDRESS AS EA, FROM_ AS F, REQUEST_TRANSACTION AS RT, USER_ACCOUNT AS UA "
                              "WHERE RT.SSN = :ssn and RT.RTid = F.RTid and F.Identifier = EA.Identifier and EA.SSN = UA.SSN "
                              "      and RT.DateTime > :dts and RT.Datetime < :dte and RT.RTid = :rtid "
                              "ORDER BY DateTime DESC");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                query.bindValue(":rtid", ui->LE_history_search->text());
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        currentrow = ui->TW_history->rowCount();
                        ui->TW_history->setRowCount(currentrow+1);
                        ui->TW_history->setItem(currentrow, 0, new QTableWidgetItem(query.value(0).toString()));
                        ui->TW_history->setItem(currentrow, 1, new QTableWidgetItem(query.value(1).toString()));
                        ui->TW_history->setItem(currentrow, 2, new QTableWidgetItem(query.value(2).toString()));
                        ui->TW_history->setItem(currentrow, 3, new QTableWidgetItem(query.value(3).toString()));
                        ui->TW_history->setItem(currentrow, 4, new QTableWidgetItem(query.value(4).toString()));
                        ui->TW_history->setItem(currentrow, 5, new QTableWidgetItem(query.value(5).toString()));
                        if(query.value(6).toString() == "p")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Pending"));
                        if(query.value(6).toString() == "a")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Accept"));
                        if(query.value(6).toString() == "d")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Deny"));
                        ui->TW_history->setItem(currentrow, 7, new QTableWidgetItem(query.value(7).toString()));
                    }while(query.next());
                }
            }
        }
        else if(index == 6){//ALL
            if(type == 1){//send
                ui->TW_history->setColumnCount(7);
                ui->TW_history->setRowCount(0);
                header << tr("STID") << tr("Amount") << tr("DateTime") << tr("Memo")
                       << tr("Cancelled") << tr("U_SSN") << tr("Identifier");
                ui->TW_history->setHorizontalHeaderLabels(header);

                query.prepare("SELECT * "
                              "FROM SEND_TRANSACTION "
                              "WHERE SSN = :ssn and DateTime > :dts and Datetime < :dte "
                              "ORDER BY DateTime DESC");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        currentrow = ui->TW_history->rowCount();
                        ui->TW_history->setRowCount(currentrow+1);
                        ui->TW_history->setItem(currentrow, 0, new QTableWidgetItem(query.value(0).toString()));
                        ui->TW_history->setItem(currentrow, 1, new QTableWidgetItem(query.value(1).toString()));
                        ui->TW_history->setItem(currentrow, 2, new QTableWidgetItem(query.value(2).toString()));
                        ui->TW_history->setItem(currentrow, 3, new QTableWidgetItem(query.value(3).toString()));
                        ui->TW_history->setItem(currentrow, 4, new QTableWidgetItem(query.value(4).toString()));
                        ui->TW_history->setItem(currentrow, 5, new QTableWidgetItem(query.value(5).toString()));
                        ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem(query.value(6).toString()));
                    }while(query.next());
                }
            }
            else if (type == 2){//request
                ui->TW_history->setColumnCount(8);
                ui->TW_history->setRowCount(0);
                header << tr("RTID") << tr("Amount") << tr("DateTime") << tr("Identifier") << tr("Name")
                       << tr("Percenage") << tr("Status") << tr("Memo");
                ui->TW_history->setHorizontalHeaderLabels(header);
                query.prepare("SELECT RT.RTid, RT.Amount, RT.DateTime, F.Identifier, UA.Name, F.Percentage, F.Status, RT.Memo "
                              "FROM ELECTRONIC_ADDRESS AS EA, FROM_ AS F, REQUEST_TRANSACTION AS RT, USER_ACCOUNT AS UA "
                              "WHERE RT.SSN = :ssn and RT.RTid = F.RTid and F.Identifier = EA.Identifier and EA.SSN = UA.SSN "
                              "      and RT.DateTime > :dts and RT.Datetime < :dte "
                              "ORDER BY DateTime DESC");
                query.bindValue(":ssn", userssn);
                query.bindValue(":dts", dts);
                query.bindValue(":dte", dte);
                if(!query.exec()){
                    mb.setText(query.lastError().text());
                    mb.exec();
                    qDebug() << query.lastError();
                }
                else if(query.next()){
                    do{
                        currentrow = ui->TW_history->rowCount();
                        ui->TW_history->setRowCount(currentrow+1);
                        ui->TW_history->setItem(currentrow, 0, new QTableWidgetItem(query.value(0).toString()));
                        ui->TW_history->setItem(currentrow, 1, new QTableWidgetItem(query.value(1).toString()));
                        ui->TW_history->setItem(currentrow, 2, new QTableWidgetItem(query.value(2).toString()));
                        ui->TW_history->setItem(currentrow, 3, new QTableWidgetItem(query.value(3).toString()));
                        ui->TW_history->setItem(currentrow, 4, new QTableWidgetItem(query.value(4).toString()));
                        ui->TW_history->setItem(currentrow, 5, new QTableWidgetItem(query.value(5).toString()));
                        if(query.value(6).toString() == "p")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Pending"));
                        if(query.value(6).toString() == "a")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Accept"));
                        if(query.value(6).toString() == "d")
                            ui->TW_history->setItem(currentrow, 6, new QTableWidgetItem("Deny"));
                        ui->TW_history->setItem(currentrow, 7, new QTableWidgetItem(query.value(7).toString()));
                    }while(query.next());
                }
            }
        }
    }
    ui->LE_history_search->setText("");
}

void MainWindow::on_PB_signout_clicked()
{
    ui->Main_TB->setCurrentIndex(0);
}
