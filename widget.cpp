#include "widget.h"
#include "ui_widget.h"


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->SerialClose->setEnabled(false); //Disable the close button
    ui->LEDControl->setEnabled(false); //Disable the LED control button
    SerialPortInfoInit(); //Initialize the serial port information
    // If there is data received, call the readData() function
    connect(&serialPort, &QSerialPort::readyRead, this, &Widget::ReadData);
    connect(timer, &QTimer::timeout, this, &Widget::DisplayTime); //Display the current date and time
    timer->start(1000); //Start the timer
    SQLInit(); //Initialize the database
}

Widget::~Widget()
{
    serialPort.close(); //Close the serial port
    db.close(); //Close the database
    delete ui; //Delete the UI
}
// Output the available serial port information
void Widget::SerialPortInfoInit()
{
    // QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &portInfo, QSerialPortInfo::availablePorts())
    {
        ui->Display->append("Available Ports: " + portInfo.portName() + "\n"); //Append the available serial port information to the text box
    }

}
// Initialize the database
void Widget::SQLInit()
{
    db.setDatabaseName("mydatabase.db"); //Set the database name
    if(!db.open()) //Open the database
    {
        //ui->Display->append("Failed to open the database!\n"); //Append the error message to the text box
    }
    else
    {
        //ui->Display->append("Open the database successfully!\n");// qDebug()<<"Open the database successfully!";
    }
    query.exec("create table if not exists agriculture(id int primary key, temperature int, humidity int, light int, datetime datetime)"); //Create a table
    query.exec("select id from agriculture order by id desc limit 1"); //Get the last ID
    if(query.next()) //If there is data, get the ID
    {
        num = query.value(0).toInt();
    }
    else //If there is no data, set the ID to 0
    {
        num = 0;
    }
}
// Display the current date and time
void Widget::DisplayTime()
{
    current_date_time = QDateTime::currentDateTime(); //Get the current date and time
    QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss"); //Convert the date and time to string
    ui->LCD->display(current_date); //Display the date and time
}
// Connect the serial port
void Widget::on_SerialConnect_clicked()
{
    serialPort.setPortName(ui->Port->text()); //Set the serial port name
    serialPort.setBaudRate(QSerialPort::Baud115200); //Set the baud rate
    if(!serialPort.open(QIODevice::ReadWrite)) //Open the serial port
    {
        // Failed to open the serial port, prompt an error message
        QMessageBox::critical(this,tr("Connect Failure!"),tr("Please check your device carefully!"));
    }
    else
    {
        ui->SerialStatus->setText("Connected!"); //Set the status to connected
        ui->SerialConnect->setEnabled(false); //Disable the connect button
        ui->SerialClose->setEnabled(true); //Enable the close button
        ui->LEDControl->setEnabled(true); //Enable the LED control button
        // QMessageBox::information(this,tr("Connect Success!"),tr("You can start operating!"));
    }
}
// Close the serial port
void Widget::on_SerialClose_clicked()
{
    serialPort.close(); //Close the serial port
    ui->SerialStatus->setText("Disconnected!"); //Set the status to disconnected
    ui->SerialConnect->setEnabled(true); //Enable the connect button
    ui->SerialClose->setEnabled(false); //Disable the close button
    ui->LEDControl->setEnabled(false); //Disable the LED control button
}
// Receive data and read data
void Widget::ReadData()
{
    QByteArray buf; // Create a byte array to store the received data
    buf = serialPort.readAll();
    if(!buf.isEmpty()) // If the data is not empty, append the data to the text box
    {
        // QString str = ui->Display->toPlainText();
        // str += tr(buf + "\n");
        QString str = tr(buf);
        // ui->Display->clear();
        //ui->Display->append(str+"\n");
        //QDebug<<str;

        QString temperatureStr = QString(str[0]) + QString(str[1]); // Convert the temperature value to QString
        QString humidityStr = QString(str[2]) + QString(str[3]); // Convert the humidity value to QString
        QString lightStr = QString(str[4]) + QString(str[5]) + QString(str[6]); // Convert the light value to QString

        ui->TempValue->setText(temperatureStr); // Display the temperature value
        ui->HumiValue->setText(humidityStr); // Display the humidity value
        ui->LightValue->setText(lightStr); // Display the light value
        
        int temperature = temperatureStr.toInt(); // Convert the temperature value to int
        int humidity = humidityStr.toInt(); // Convert the humidity value to int
        int light = lightStr.toInt(); // Convert the light value to int
        
       // Insert data into the database every 10 times
        if(i++ % 10 == 0)
        {
            query.exec("INSERT INTO agriculture (id, temperature, humidity, light, datetime) "
                "VALUES ('" + QString::number(++num) +"', '" + QString::number(temperature) + "', '" + QString::number(humidity) + "', '" + QString::number(light) + "', '" + current_date_time.toString("yyyy-MM-dd hh:mm:ss") + "')"); // Insert data into the database
        }
        // Check for errors
        if (query.lastError().isValid()) {
            //ui->Display->append("Failed to insert data:" + query.lastError().text() + "\n");
        }
        if(str[7]=="1") // Display the LED status
        {
            ui->LEDStatus->setText("ON");
            ui->LEDControl->setText("Close"); // Change the LED control button text
        }
        else if(str[7]=="0")
        {
            ui->LEDStatus->setText("OFF");
            ui->LEDControl->setText("Open");
        }
    }
    buf.clear(); // Clear the buffer
}
// LED control
void Widget::on_LEDControl_clicked()
{
    if(ui->LEDControl->text() == "Open")
    {
        serialPort.write("1"); //send 0x01 to open the LED
        QThread::msleep(10); //Delay 100ms
    }
    else
    {
        serialPort.write("0"); //send 0x00 to close the LED
        QThread::msleep(10);
    }
}

// Make a chart to display the temperature changes
void Widget::on_BtTempView_clicked()
{
    QLineSeries *series = new QLineSeries(); // Create a line series
    // Read the latest 8 rows from the table
    query.exec("select * from agriculture order by id desc limit 8");
    int j = 8;
    while (query.next()) {
        QString temperature = query.value("temperature").toString();
        series->append(j--, temperature.toInt());
        //ui->Display->append("Temperature: " + temperature + "\n");
    }

    QChart *chart = new QChart(); // Create a chart
    chart->legend()->hide(); // Hide the legend
    chart->addSeries(series); // Add the series to the chart
    chart->createDefaultAxes(); // Create default axes
    chart->setTitle("Temperature Changes"); // Set the title of the chart

    QValueAxis *axisX = new QValueAxis(); // Create a value axis
    axisX->setLabelFormat("%.0f h"); // Set the label format
    axisX->setTitleText("Hours"); // Set the title of the axis

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%.0f C");
    axisY->setTitleText("Celsius (°C)");

    chart->setAxisX(axisX, series); // Set the axis
    chart->setAxisY(axisY, series);

    QGraphicsScene *scene = new QGraphicsScene(); // Create a graphics scene
    QGraphicsView *view = new QGraphicsView(scene); // Create a graphics view
    view->setRenderHint(QPainter::Antialiasing); // Set the render hint
    view->setSceneRect(0, 0, 670, 380); // Set the size of the view

    QChartView *chartView = new QChartView(chart); // Create a chart view
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(0, 0, 670, 380);

    scene->addWidget(chartView); // Add the chart view to the scene
    QVBoxLayout *layout = new QVBoxLayout(); // Create a vertical layout
    layout->addWidget(view); // Add the view to the layout

    QLayout *existingLayout = ui->TempView->layout(); // Get the existing layout
    if (existingLayout != nullptr) // If the layout is not empty, delete the layout
    {
        delete existingLayout;
    }

    ui->TempView->setLayout(layout); // Set the layout
}


void Widget::on_BtLightView_clicked()
{
    QLineSeries *series = new QLineSeries(); // Create a line series
    // Read the latest 8 rows from the table
    query.exec("select * from agriculture order by id desc limit 8");
    int j = 8;
    while (query.next()) {
        QString light = query.value("light").toString();
        series->append(j--, light.toInt());
        //ui->Display->append("light: " + light + "\n");
    }

    QChart *chart = new QChart(); // Create a chart
    chart->legend()->hide(); // Hide the legend
    chart->addSeries(series); // Add the series to the chart
    chart->createDefaultAxes(); // Create default axes
    chart->setTitle("Light Changes"); // Set the title of the chart

    QValueAxis *axisX = new QValueAxis(); // Create a value axis
    axisX->setLabelFormat("%.0f h"); // Set the label format
    axisX->setTitleText("Hours"); // Set the title of the axis

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%.0f lux");
    axisY->setTitleText("Light Intensity (lux)");

    chart->setAxisX(axisX, series); // Set the axis
    chart->setAxisY(axisY, series);

    QGraphicsScene *scene = new QGraphicsScene(); // Create a graphics scene
    QGraphicsView *view = new QGraphicsView(scene); // Create a graphics view
    view->setRenderHint(QPainter::Antialiasing); // Set the render hint
    view->setSceneRect(0, 0, 670, 380); // Set the size of the view

    QChartView *chartView = new QChartView(chart); // Create a chart view
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(0, 0, 670, 380);

    scene->addWidget(chartView); // Add the chart view to the scene
    QVBoxLayout *layout = new QVBoxLayout(); // Create a vertical layout
    layout->addWidget(view); // Add the view to the layout

    QLayout *existingLayout = ui->LightView->layout(); // Get the existing layout
    if (existingLayout != nullptr) // If the layout is not empty, delete the layout
    {
        delete existingLayout;
    }

    ui->LightView->setLayout(layout); // Set the layout
}


void Widget::on_BtHumidView_clicked()
{
    QLineSeries *series = new QLineSeries(); // Create a line series
    // Read the latest 8 rows from the table
    query.exec("select * from agriculture order by id desc limit 8");
    int j = 8;
    while (query.next()) {
        QString humidity = query.value("humidity").toString();
        series->append(j--, humidity.toInt());
        //ui->Display->append("Humidity: " + humidity + "\n");
    }

    QChart *chart = new QChart(); // Create a chart
    chart->legend()->hide(); // Hide the legend
    chart->addSeries(series); // Add the series to the chart
    chart->createDefaultAxes(); // Create default axes
    chart->setTitle("Humidity Changes"); // Set the title of the chart

    QValueAxis *axisX = new QValueAxis(); // Create a value axis
    axisX->setLabelFormat("%.0f h"); // Set the label format
    axisX->setTitleText("Hours"); // Set the title of the axis

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%.0f g/m3");
    axisY->setTitleText("Humidity (g/m³)");

    chart->setAxisX(axisX, series); // Set the axis
    chart->setAxisY(axisY, series);

    QGraphicsScene *scene = new QGraphicsScene(); // Create a graphics scene
    QGraphicsView *view = new QGraphicsView(scene); // Create a graphics view
    view->setRenderHint(QPainter::Antialiasing); // Set the render hint
    view->setSceneRect(0, 0, 670, 380); // Set the size of the view

    QChartView *chartView = new QChartView(chart); // Create a chart view
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(0, 0, 670, 380);

    scene->addWidget(chartView); // Add the chart view to the scene
    QVBoxLayout *layout = new QVBoxLayout(); // Create a vertical layout
    layout->addWidget(view); // Add the view to the layout

    QLayout *existingLayout = ui->HumidView->layout(); // Get the existing layout
    if (existingLayout != nullptr) // If the layout is not empty, delete the layout
    {
        delete existingLayout;
    }

    ui->HumidView->setLayout(layout); // Set the layout
}

