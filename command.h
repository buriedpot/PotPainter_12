#ifndef COMMAND_H
#define COMMAND_H
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <QString>
#include <QStringList>
#include "canvas.h"
using namespace std;


class Command
{
private:
    vector<string> split(string cmd, char tag);
    string picsavedir;
public:
    Command();
    Command(QString picsavedir);
    int cmdAnalyze(string cmd, Canvas& canvas, QFile &fin);
    static string lower(const string& str);
};

#endif // COMMAND_H
