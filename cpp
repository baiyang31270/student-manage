#include <QApplication>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QMessageBox>
#include <mysql.h>

// 定义学生信息结构体
struct Student {
    QString name;
    int age;
    QString gender;
    int studentId;
};
//主窗口类
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {   
        setupUi();
        connectDatabase();
    }

    ~MainWindow() {
        mysql_close(database);
    }

private:
    QLabel* nameLabel;
    QLineEdit* nameLineEdit;
    QLabel* ageLabel;
    QLineEdit* ageLineEdit;
    QLabel* genderLabel;
    QLineEdit* genderLineEdit;
    QLabel* studentIdLabel;
    QLineEdit* studentIdLineEdit;
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* updateButton;
    QPushButton* searchButton;
    QTableWidget* tableWidget;

    MYSQL* database;
    //ui
    void setupUi() {
        QWidget* centralWidget = new QWidget;
        setCentralWidget(centralWidget);

        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

        tableWidget = new QTableWidget;
        tableWidget->setColumnCount(4);
        tableWidget->setHorizontalHeaderLabels({ "姓名", "年龄", "性别", "学号" });
        mainLayout->addWidget(tableWidget);

        QHBoxLayout* inputLayout = new QHBoxLayout;
        nameLabel = new QLabel("姓名:");
        nameLineEdit = new QLineEdit;
        ageLabel = new QLabel("年龄:");
        ageLineEdit = new QLineEdit;
        genderLabel = new QLabel("性别:");
        genderLineEdit = new QLineEdit;
        studentIdLabel = new QLabel("学号:");
        studentIdLineEdit = new QLineEdit;
        inputLayout->addWidget(nameLabel);
        inputLayout->addWidget(nameLineEdit);
        inputLayout->addWidget(ageLabel);
        inputLayout->addWidget(ageLineEdit);
        inputLayout->addWidget(genderLabel);
        inputLayout->addWidget(genderLineEdit);
        inputLayout->addWidget(studentIdLabel);
        inputLayout->addWidget(studentIdLineEdit);
        mainLayout->addLayout(inputLayout);

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        addButton = new QPushButton("增加");
        deleteButton = new QPushButton("删除");
        updateButton = new QPushButton("修改");
        searchButton = new QPushButton("搜索");
        buttonLayout->addWidget(addButton);
        buttonLayout->addWidget(deleteButton);
        buttonLayout->addWidget(updateButton);
        buttonLayout->addWidget(searchButton);
        mainLayout->addLayout(buttonLayout);

        setWindowTitle("学生信息管理系统");
        resize(600, 400);
        connect(addButton, SIGNAL(clicked()), this, SLOT(addStudent()));
        connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteStudent()));
        connect(updateButton, SIGNAL(clicked()), this, SLOT(updateStudent()));
        connect(searchButton, SIGNAL(clicked()), this, SLOT(searchStudent()));
    }
    //连接数据库
    void connectDatabase() {
        database = mysql_init(nullptr);
        if (database == nullptr) {
            QMessageBox::critical(this, "Error", "Failed to initialize database connection.");
            return;
        }

        // 设置中文字符集
        mysql_options(database, MYSQL_SET_CHARSET_NAME, "utf8mb4");

        if (mysql_real_connect(database, "ip", "root", "password", "students", 0, nullptr, 0) == nullptr) {
            QMessageBox::critical(this, "出错啦！", "请检查你的网络.");
            return;
        }
        displayStudents();
        // 设置客户端字符集
        mysql_set_character_set(database, "utf8mb4");
    }
    //学生信息操作
private slots:
    void addStudent() {
        QString name = nameLineEdit->text();
        QString ageStr = ageLineEdit->text();
        QString gender = genderLineEdit->text();
        QString studentIdStr = studentIdLineEdit->text();

        if (name.isEmpty() || ageStr.isEmpty() || gender.isEmpty() || studentIdStr.isEmpty()) {
            QMessageBox::information(this, "出错啦！", "请输入完整的学生信息.");
            return;
        }

        int age = ageStr.toInt();
        int studentId = studentIdStr.toInt();

        QString query = QString("INSERT INTO students (name, age, gender, studentId) VALUES ('%1', %2, '%3', %4)").arg(name).arg(age).arg(gender).arg(studentId);
        if (mysql_query(database, query.toUtf8().constData()) != 0) {
            QMessageBox::critical(this, "出错啦！", "添加错误.");
            return;
        }

        displayStudents();
        clearInputs();
    }

    void deleteStudent() {
        QString studentIdStr = studentIdLineEdit->text();
        if (studentIdStr.isEmpty()) {
            QMessageBox::information(this, "出错啦！", "请输入要删除的学生ID.");
            return;
        }

        int studentId = studentIdStr.toInt();

        QString query = QString("DELETE FROM students WHERE studentId = %1").arg(studentId);
        if (mysql_query(database, query.toUtf8().constData()) != 0) {
            QMessageBox::critical(this, "出错啦！", "删除学生错误.");
            return;
        }

        displayStudents();
        clearInputs();
    }

    void updateStudent() {
        QString name = nameLineEdit->text();
        QString ageStr = ageLineEdit->text();
        QString gender = genderLineEdit->text();
        QString studentIdStr = studentIdLineEdit->text();

        if (name.isEmpty() || ageStr.isEmpty() || gender.isEmpty() || studentIdStr.isEmpty()) {
            QMessageBox::information(this, "出错啦！", "请输入完整的学生信息.");
            return;
        }

        int age = ageStr.toInt();
        int studentId = studentIdStr.toInt();

        QString query = QString("UPDATE students SET name = '%1', age = %2, gender = '%3' WHERE studentId = %4").arg(name).arg(age).arg(gender).arg(studentId);
        if (mysql_query(database, query.toUtf8().constData()) != 0) {
            return;
        }

        displayStudents();
        clearInputs();
    }

    void searchStudent() {
        QString studentIdStr = studentIdLineEdit->text();
        if (studentIdStr.isEmpty()) {
            QMessageBox::information(this, "出错啦！", "请输入要搜索的学生ID.");
            return;
        }
        int studentId = studentIdStr.toInt();
        QString query = QString("SELECT * FROM students WHERE studentId = %1").arg(studentId);
        if (mysql_query(database, query.toUtf8().constData()) != 0) {
            return;
        }
        MYSQL_RES* result = mysql_store_result(database);
        if (result == nullptr) {
            QMessageBox::information(this, "出错啦！", "未找到该学生.");
            return;
        }

        int numFields = mysql_num_fields(result);
        MYSQL_ROW row = mysql_fetch_row(result);

        if (row == nullptr) {
            QMessageBox::information(this, "出错啦！", "未找到该学生.");
            mysql_free_result(result);
            return;
        }

        QString name = QString::fromUtf8(row[0]);
        int age = QString(row[1]).toInt();
        QString gender = QString::fromUtf8(row[2]);
        int studentIdResult = QString(row[3]).toInt();

        nameLineEdit->setText(name);
        ageLineEdit->setText(QString::number(age));
        genderLineEdit->setText(gender);
        studentIdLineEdit->setText(QString::number(studentIdResult));

        mysql_free_result(result);
    }


private:
    void displayStudents() {
        QString query = "SELECT * FROM students";
        if (mysql_query(database, query.toUtf8().constData()) != 0) {
            return;
        }

        MYSQL_RES* result = mysql_store_result(database);
        if (result == nullptr) {
            return;
        }

        int numColumns = mysql_num_fields(result);
        if (numColumns != 4) {
            QMessageBox::critical(this, "出错啦!", "未找到.");
            mysql_free_result(result);
            return;
        }

        tableWidget->clearContents();
        tableWidget->setRowCount(0);

        while (MYSQL_ROW row = mysql_fetch_row(result)) {
            displayStudentResult(row);
        }

        mysql_free_result(result);
    }

    void displayStudentResult(MYSQL_ROW row) {
        QString name = row[0];
        QString age = row[1];
        QString gender = row[2];
        QString studentId = row[3];

        int rowIdx = tableWidget->rowCount();
        tableWidget->insertRow(rowIdx);

        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        QTableWidgetItem* ageItem = new QTableWidgetItem(age);
        QTableWidgetItem* genderItem = new QTableWidgetItem(gender);
        QTableWidgetItem* studentIdItem = new QTableWidgetItem(studentId);

        tableWidget->setItem(rowIdx, 0, nameItem);
        tableWidget->setItem(rowIdx, 1, ageItem);
        tableWidget->setItem(rowIdx, 2, genderItem);
        tableWidget->setItem(rowIdx, 3, studentIdItem);
    }

    void clearInputs() {
        nameLineEdit->clear();
        ageLineEdit->clear();
        genderLineEdit->clear();
        studentIdLineEdit->clear();
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // 创建登录窗口
    QWidget loginWindow;
    QVBoxLayout* loginLayout = new QVBoxLayout(&loginWindow);
    QLabel* usernameLabel = new QLabel("账号:");
    QLineEdit* usernameLineEdit = new QLineEdit;
    QLabel* passwordLabel = new QLabel("密码:");
    QLineEdit* passwordLineEdit = new QLineEdit;
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    QPushButton* loginButton = new QPushButton("登录");
    loginLayout->addWidget(usernameLabel);
    loginLayout->addWidget(usernameLineEdit);
    loginLayout->addWidget(passwordLabel);
    loginLayout->addWidget(passwordLineEdit);
    loginLayout->addWidget(loginButton);
    loginWindow.show();

    // 创建主窗口
    MainWindow mainWindow;

    // 连接登录窗口的登录按钮点击事件到主窗口的槽函数
    QObject::connect(loginButton, &QPushButton::clicked, [&]() {
        QString username = usernameLineEdit->text();
        QString password = passwordLineEdit->text();

        // 连接数据库
        MYSQL* database = mysql_init(nullptr);
        if (database == nullptr) {
            QMessageBox::critical(&loginWindow, "出错啦！", "无法建立连接.");
            return;
        }

        if (mysql_real_connect(database, "ip", "root", "password", "user", 0, nullptr, 0) == nullptr) {
            QMessageBox::critical(&loginWindow, "出错啦！", "无法建立连接.");
            return;
        }

        // 设置客户端字符集
        mysql_set_character_set(database, "utf8mb4");

        // 执行查询语句
        QString query = QString("SELECT * FROM users WHERE username = '%1' AND password = '%2'").arg(username).arg(password);
        if (mysql_query(database, query.toUtf8().constData()) != 0) {
            QMessageBox::critical(&loginWindow, "出错啦！", "无法获取查询结果.");
            mysql_close(database);
            return;
        }

        MYSQL_RES* result = mysql_store_result(database);
        if (result == nullptr) {
            QMessageBox::critical(&loginWindow, "出错啦！", "无法获取查询结果.");
            mysql_close(database);
            return;
        }

        int numRows = mysql_num_rows(result);
        if (numRows == 1) {
            // 验证成功，关闭登录窗口，显示主窗口
            loginWindow.close();
            mainWindow.show();
        }
        else {
            QMessageBox::critical(&loginWindow, "出错啦！", "不存在账号或密码,请联系管理员");
        }

        mysql_free_result(result);
        mysql_close(database);
        });

    return app.exec();

}

#include "main.moc"
