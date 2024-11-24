#include <QCoreApplication>
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

#include "Server.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    Central::Server server;

    return QCoreApplication::exec();
}
