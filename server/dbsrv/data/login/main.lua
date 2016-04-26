module('Login', package.seeall)


function GET(sid, uid)
    print('Login.GET', uid)
    local result = Mysql.select(Mysql.mysql_connection, string.format('select uid, data from actor where uid = %d', uid))
    if not result or #result == 0 then
        Gamesrv.post('Login.GET', sid, uid, nil)
        return
    end
    Gamesrv.post('Login.GET', sid, uid, result[1].data)
end


function SET(uid, data)
    print('Login.SET', uid, data)
    local conn = Mysql.mysql_connection
    Mysql.command(conn, string.format('replace into actor(uid, data) value (%d, "%s")', uid, Mysql.escape(conn, data)))
end

function LOGIN(sid, openid, time, sig)
    print(sid, openid, time, sig)
    local result = Mysql.select(Mysql.mysql_connection, string.format('select * from account where openid = %s', openid))
    if not result or #result == 0 then
        print('微信没登录成功吧???')
        Gamesrv.post('Login.LOGIN', sid, 1, 0)
        return
    end
    result = result[1]
    local uid = result.uid
    if sig ~= result.sig then
        print('签名不同')
        Gamesrv.post('Login.LOGIN', sid, 2, 0)
        return
    end
    Gamesrv.post('Login.LOGIN', sid, 0, uid)
end
