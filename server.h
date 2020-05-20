#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMap>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)
QT_FORWARD_DECLARE_CLASS(QString)
QT_FORWARD_DECLARE_CLASS(QTimer)

typedef struct {
  qint64 last_value;
  QList<QWebSocket *> subscribed_clients;
} Variable;

class ChatServer : public QObject {
  Q_OBJECT
public:
  explicit ChatServer(quint16 port, QObject *parent = nullptr);
  ~ChatServer() override;

private slots:
  void onNewConnection();
  void processMessage(const QString &message);
  void socketDisconnected();
  void onTimerTick();

private:
  QWebSocketServer *webSocketServer;
  QList<QWebSocket *> clients;
  QMap<QString, Variable> variables;
  QTimer *timer;

  void unsubClientFromVar(const QString &var, QWebSocket* client);
};

#endif //CHATSERVER_H