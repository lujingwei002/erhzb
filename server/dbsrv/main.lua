
print('==============================================')
print('====================dbsrv====================')
print('==============================================')

package.path = package.path..';'..'./data/?.lua'
print(package.path)

require('gamesrv.main')
require('user.main')
require('login.main')

Sqlconn.connect('127.0.0.1', 'test', 'root', '0987abc123')
--测试mysql
local result =Sqlconn.select(string.format('select count(*) as c from actor'))
print(result[1].c)

Gamesrv.listen('127.0.0.1', 8082)
