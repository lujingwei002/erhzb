module('Login', package.seeall)


function GET(sid, uid, data)
    print('Login.GET', sid, uid, data)
    local actordata = Pblua.msgnew('dbproto.Actor')
    actordata:parse_from_string(data or '')
    actordata.uid = uid
    print(actordata:debug_string())
    --Dbclient.post('Login.SET', uid, actordata:tostring())
    local reply = Pblua.msgnew('login.LOGIN_R')
    reply.uid = uid
    reply.errno = 0
    Gatesrv.send_proto(sid, reply:msgname(), reply:tostring())
end


function MSG_LOGIN(sid, msg)
    Dbclient.post('Login.LOGIN', sid, msg.openid, msg.time, msg.sig)
end


function LOGIN(sid, errno, uid)
    print('登录成功了阿', sid, errno, uid)
    if errno ~= 0 then
        local reply = Pblua.msgnew('login.LOGIN_R')
        reply.uid = uid
        reply.errno = errno
        Gatesrv.send_proto(sid, reply:msgname(), reply:tostring())
        return
    end
    --去DB加载数据吧
    Dbclient.post('Login.GET', sid, uid)
end
