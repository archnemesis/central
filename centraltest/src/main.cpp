#include <QCoreApplication>
#include <QDebug>

#include "Client.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    Central::Client testClient(QHostAddress::LocalHost, 1234);

    QObject::connect(
        &testClient,
        &Central::Client::helloRequestReceived,
        [&testClient]() {
            qDebug() << "Hello request has been received";

            testClient.authenticate(
                "robin",
                "password",
                [&testClient](){
                    // this gets run if login is good
                    testClient.getWorkspaces(
                        [&testClient](const QList<Central::Workspace>& workspaces) {
                            for (const auto& workspace : workspaces) {
                                qDebug() << "Got workspace" << workspace.name;

                                testClient.getLayersForWorkspace(
                                    workspace.id,
                                    [&testClient](const QList<Central::Layer>& layers) {
                                        for (const auto& layer : layers) {
                                            qDebug() << "Got layer" << layer.name;

                                            testClient.getObjectsForLayer(
                                                layer.id,
                                                [&testClient](const QList<Central::Object>& objects) {
                                                    for (const auto& object : objects) {
                                                        qDebug() << "Got object" << object.label;
                                                    }
                                                },
                                                [](const Central::ProtocolError& error) {
                                                    qDebug() << "Object list error!";
                                                },
                                                [](int current, int total) {
                                                    qDebug() << "Getting" << current << "of" << total;
                                                });
                                        }
                                    },
                                    [](const Central::ProtocolError& error) {
                                        qDebug() << "Layer list error!";
                                    });
                            }
                        },
                        [](const Central::ProtocolError& error) {
                            qDebug() << "Workspace list error!";
                        });
                },
                [](const Central::ProtocolError& error) {
                    // this gets run if login fails
                    qDebug() << "Login failed!";
                });
        });

    testClient.connectToServer();

    return QCoreApplication::exec();
}
