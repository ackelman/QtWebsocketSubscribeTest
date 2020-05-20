# QtWebsocketSubscribeTest
Test Websocket server pushing out "news" to subscribed clients
This is essentially the "ChatServer" example from QT's website, but with multiple "channels" that the clients can subscribe to.

Also, instead of the clients sending messages to the "channel", they only subscribe to the "newsletter" which is created by some third party in a typical producer-consumer fashion.
