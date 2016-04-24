

print('==============================================')
print('====================gatesrv====================')
print('==============================================')

package.path = package.path..';'..'./data/?.lua'
print(package.path)

require('login.main')
require('websocket.main')
require('gameclient.main')

Websocket.listen('127.0.0.1', 8080)
Gameclient.connect('127.0.0.1', 8081)
