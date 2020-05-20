#include "server.h"

#include <QtWebSockets>
#include <QtCore>

#include <cstdio>

QT_USE_NAMESPACE

static QString getIdentifier(QWebSocket *peer) {
  return QStringLiteral("%1:%2").arg(peer->peerAddress().toString(),
    QString::number(peer->peerPort()));
}

ChatServer::ChatServer(quint16 port, QObject *parent) : QObject(parent),
    webSocketServer(new QWebSocketServer(QStringLiteral("Chat Server"),
    QWebSocketServer::NonSecureMode, this)) {
  if (webSocketServer->listen(QHostAddress::Any, port)) {
    QTextStream(stdout) << "Chat Server listening on port " << port << '\n';
    connect(webSocketServer, &QWebSocketServer::newConnection, this, &ChatServer::onNewConnection);

    // Start the background thread that will check if any variable has changed
    this->timer = new QTimer(this);
    connect(this->timer, &QTimer::timeout, this, &ChatServer::onTimerTick);
    this->timer->start(1000);
  }
}

ChatServer::~ChatServer() {
  this->timer->stop();
  webSocketServer->close();
}

void ChatServer::onNewConnection() {
  QWebSocket* socket = webSocketServer->nextPendingConnection();
  QTextStream(stdout) << getIdentifier(socket) << " connected!\n";
  socket->setParent(this);

  connect(socket, &QWebSocket::textMessageReceived, this,
    &ChatServer::processMessage);
  connect(socket, &QWebSocket::disconnected, this,
    &ChatServer::socketDisconnected);

  clients.append(socket);
}

void ChatServer::onTimerTick() {
  qint64 new_value = QDateTime::currentSecsSinceEpoch();
  for (QString key : this->variables.keys()) {
    Variable* var = &this->variables[key];
    // Check if the value of the variable has changed
    // This is just a dummy if-statement right now
    if (new_value - var->last_value >= QString::compare(key, "A", Qt::CaseInsensitive) + 1) {
      for (QWebSocket *client : qAsConst(var->subscribed_clients)) {
        client->sendTextMessage(QString("push ") + key + ' ' + QDateTime::fromSecsSinceEpoch(new_value).toString("yyyy-MM-dd hh:mm:ss")); // Push new value
      }
      var->last_value = new_value;
    }
  }
}

void ChatServer::unsubClientFromVar(const QString &variableName, QWebSocket* client) {
  this->variables[variableName].subscribed_clients.removeAll(client);

  // If there are no subscribed clients
  if (this->variables[variableName].subscribed_clients.size() == 0) {
    this->variables.remove(variableName);
  }
}

void ChatServer::processMessage(const QString &message) {
  QWebSocket *pSender = qobject_cast<QWebSocket *>(sender());

  QTextStream(stdout) << "Received message '" << message << "' from " << getIdentifier(pSender) << "\n";

  try {
    QList<QString> tokens = message.split(' ');
    QString cmd = tokens[0];
    QString variableName = tokens[1];

    if (cmd == "sub") {
      if (!variables.contains(variableName)) {
        variables.insert(variableName, Variable());
      }
      // Only subscribe the client if it isn't already subscribed to the variable
      if (!variables[variableName].subscribed_clients.contains(pSender)) {
        variables[variableName].subscribed_clients.append(pSender);
      } else {
        pSender->sendTextMessage("Error: You are already subscribed to that variable");
      }
    } else if (cmd == "unsub") {
      if (!variables.contains(variableName)) {
        pSender->sendTextMessage("Error: You are not subscribed to that variable");
        return;
      }

      unsubClientFromVar(variableName, pSender);
    }
  } catch (...) {
    pSender->sendTextMessage("Error: Invalid command");
  }
}

void ChatServer::socketDisconnected() {
  QWebSocket *client = qobject_cast<QWebSocket *>(sender());
  QTextStream(stdout) << getIdentifier(client) << " disconnected!\n";
  if (client) {
    // Remove all the subscriptions that the client had.
    for (QString key : this->variables.keys()) {
      unsubClientFromVar(key, client);
    }

    clients.removeAll(client);
    client->deleteLater();
  }
}