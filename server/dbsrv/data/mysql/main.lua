module('Mysql', package.seeall)

mysql_connection = nil

function main()
    mysql_connection = Mysql.connect('127.0.0.1', 'test', 'root', '0987abc123')
    --local result = Mysql.select(mysql_connection, string.format('select uid, data from actor where uid = %d', 1))
    --print(result[1].uid)
    --print(result[1].data)
end
