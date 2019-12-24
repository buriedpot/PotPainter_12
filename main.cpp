#include "mainwindow.h"
#include "resetcanvasdlg.h"
#include "command.h"
#include <iostream>
#include <string>
#include <QApplication>
#include <QTextStream>
#include <QByteArray>
using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    cout << argc << endl;
    if (argc == 2) {
        cout << "Please enter three parameters" << endl;
        return 0;
        //TODO command string control part
        /*
         * import script
         * or command directly
         */
        string command;
        Command c;
        Canvas canvas;
        while (1) {
            QTextStream qtin(stdin);

            char tmp[101];
            cout << ">>> ";
            cin.getline(tmp, 100);
            command = tmp;
            if (command == "quit" || command == "q") break;
            //int operate = c.cmdAnalyze(command, canvas, fin);
            //canvas->update();
            //if (operate == -1) cout << "Invalid Command!" << endl;
        }
        //delete canvas;
        return 0;
    }
    else if (argc == 3) {
        Canvas canvas;
        cout << argv[1] << argv[2] << endl;
        QString scriptpath(argv[1]), picsavedir(argv[2]);
        Command c(argv[2]);
        QFile script(scriptpath);
        script.open(QIODevice::ReadOnly | QIODevice::Text);
        while (!script.atEnd()) {
            QByteArray line = script.readLine();
            QString tmp(line);
            if (tmp[tmp.size() - 1] == '\n')
                tmp = tmp.left(tmp.size() - 1);
            string command = tmp.toStdString();
            cout << command << endl;
            int operate = c.cmdAnalyze(command, canvas, script);
            //canvas->update();
            if (operate == -1) cout << "Invalid Command!" << endl;
        }
        return 0;
    }
    else {
        w.show();
    }
    return a.exec();
}

