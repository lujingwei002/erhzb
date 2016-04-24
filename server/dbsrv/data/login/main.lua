module('Login', package.seeall)


function GET(uid)
    print('Login.GET', uid)
    local result = Mysql.select(Mysql.mysql_connection, string.format('select uid, data from actor where uid = %d', uid))
    if not result or #result == 0 then
        Gamesrv.post('Login.GET', uid, nil)
    else
        Gamesrv.post('Login.GET', uid, result[1].data)
    end
end


function SET(uid, data)
    print('Login.SET', uid, data)
    local conn = Mysql.mysql_connection
    Mysql.command(conn, string.format('replace into actor(uid, data) value (%d, "%s")', uid, Mysql.escape(conn, data)))
end
