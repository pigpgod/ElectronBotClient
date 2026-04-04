/**
 * @file main.cpp
 * @brief 应用程序入口
 * 
 * ElectronBot 客户端应用程序的入口点：
 * - 初始化 Qt 应用程序
 * - 创建并显示主窗口
 * - 启动事件循环
 */

#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
