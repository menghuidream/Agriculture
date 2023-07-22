#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QThread>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLayout>
#include <QValueAxis>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE
QT_CHARTS_USE_NAMESPACE // Use the QtCharts namespace

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void SerialPortInfoInit(); //Output the available serial port information
    void ReadData(); //Read the data from the serial port
    void SQLInit(); //Initialize the database
    void DisplayTime(); //Display the current date and time

private slots:
    void on_SerialConnect_clicked(); //Connect the serial port

    void on_SerialClose_clicked(); //Close the serial port

    void on_LEDControl_clicked(); //LED control

    void on_BtTempView_clicked(); //Temperature view

    void on_BtLightView_clicked();

    void on_BtHumidView_clicked();

private:
    Ui::Widget *ui;
    QSerialPort serialPort; //Define the serial port
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE"); //Define the database
    QSqlQuery query; //Define the SQL query
    QDateTime current_date_time; //Get the current date and time
    QTimer *timer = new QTimer(); //Define the timer
    int num; //Define the ID
    int i = 0; //Define the counter
};
#endif // WIDGET_H
