

print('==============================================')
print('====================gamesrv====================')
print('==============================================')

package.path = package.path..';'..'./data/?.lua'
print(package.path)
math.randomseed(os.time())

require('dbclient.main')
require('gatesrv.main')
require('login.main')
require('actor.actormgr')
require('douniu.main')
require('config.dounieconf')

--测试protobuf是确初始化
Pblua.mappath('dbproto', '../doc/dbproto')
Pblua.mappath('proto',   '../doc/proto')
Pblua.import('dbproto/actor.proto')
Pblua.import('proto/login.proto')
Pblua.import('proto/gatesrv.proto')
local user = Pblua.msgnew('dbproto.Actor')
assert(user)

--测试json
print(Json.encode({uid = 1}))
local json = Json.decode('{"uid" : 1}')

Gatesrv.listen('127.0.0.1', 8081)
Dbclient.connect('127.0.0.1', 8082)


Dounie.main()
