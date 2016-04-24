
print('==============================================')
print('====================dbsrv====================')
print('==============================================')

package.path = package.path..';'..'./data/?.lua'
print(package.path)

require('gamesrv.main')
require('user.main')
require('login.main')
require('mysql.main')

Mysql.main()
Gamesrv.listen('127.0.0.1', 8082)
