
print('==============================================')
print('====================dbsrv====================')
print('==============================================')

package.path = package.path..';'..'./data/?.lua'
print(package.path)

require('gamesrv.main')
require('user.main')
require('login.main')

Pblua.mappath('dbproto', '../doc/dbproto')
Pblua.mappath('proto',   '../doc/proto')
Pblua.import('dbproto/actor.proto')
local user = Pblua.msgnew('dbproto.Actor')
assert(user)

--Sqlconn.connect('127.0.0.1', 'test', 'root', '0987abc123')
Sqlconn.connect('127.0.0.1', 'test', 'root', '')
--测试mysql
local result =Sqlconn.select(string.format('select count(*) as c from actor'))
print(result[1].c)

Gamesrv.listen('127.0.0.1', 8082)

Login.test()
