module('Login', package.seeall)


function GET(sid, uid)
    print('Login.GET', uid)
    local actorrow = Sqlconn.select(string.format('select * from actor where uid = %d', uid))
    if not actorrow or #actorrow == 0 then
        Gamesrv.post('Login.GET', sid, uid, nil)
        return
    end
    actorrow = actorrow[1]

    local accountrow = Sqlconn.select(string.format('select * from account where uid = %d', uid))
    if not accountrow or #accountrow == 0 then
        Gamesrv.post('Login.GET', sid, uid, nil)
        return
    end
    accountrow = accountrow[1]

    local actordata = Pblua.msgnew('dbproto.Actor')
    actordata.headimg = accountrow.headimg
    actordata.coin = actorrow.coin
    actordata.diamond = actorrow.diamond
    actordata.username = actorrow.username

    Gamesrv.post('Login.GET', sid, uid, actordata:tostring()) 
end


function SET(sid, uid, data)
    print('Login.SET', uid, data)
    Sqlconn.command(string.format('replace into actor(uid, data) value (%d, "%s")', uid, Sqlconn.escape(data)))
    Gamesrv.post('Login.SET', sid, uid) 
end

function LOGIN(sid, openid, time, sig)
    print('Login.LOGIN', sid, openid, time, sig)
    local result = Sqlconn.select(string.format('select * from account where openid = %s', openid))
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

--测试
function test()
    local uid = 1
    local result = Sqlconn.select(string.format('select * from actor where uid = %d', uid))
    if not result or #result == 0 then
        print('no data')
        return
    end
    result = result[1]
    print('diamond', result.diamond, result.coin)
    local actordata = Pblua.msgnew('dbproto.Actor')
end
