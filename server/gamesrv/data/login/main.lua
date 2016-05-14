module('Login', package.seeall)

temp_session = {}

function GET(sid, uid, data)
    temp_session[sid] = nil
    print('Login.GET', sid, uid, data)
    local actordata = Pblua.msgnew('dbproto.Actor')
    actordata:parse_from_string(data or '')
    actordata.uid = uid
    actordata.username = tostring(uid)--'叼你怕了吗'..uid
    print(actordata:debug_string())
    --Dbclient.post('Login.SET', uid, actordata:tostring())
    local reply = Pblua.msgnew('login.LOGIN_R')
    reply.uid = uid
    reply.errno = 0
    Actor.create_actor(sid, actordata)
    Gatesrv.send_proto(sid, reply:msgname(), reply:tostring())
end


function SET(sid, uid)
    Actor.destory_actor(sid, uid)
end

function LOGIN(sid, errno, uid)
    if errno ~= 0 then
        print('登录出错了', sid, errno, uid)
        local reply = Pblua.msgnew('login.LOGIN_R')
        reply.uid = uid
        reply.errno = errno
        Gatesrv.send_proto(sid, reply:msgname(), reply:tostring())
        return
    end
    local actor = Actor.actor_mgr[uid]
    if actor then
        print('重复登录了')
        local reply = Pblua.msgnew('login.LOGIN_R')
        reply.uid = uid
        reply.errno = 4 
        Gatesrv.send_proto(sid, reply:msgname(), reply:tostring())
        --local last_sid = actor.sid
        --Actor.actor_session[sid] = actor
        --Actor.actor_session[last_sid] = nil
        --actor.sid = sid
        return
    end
    if temp_session[sid] and os.time() - temp_session[sid] < 10 then
        print('稍等')
        --已经在拉取数据了，稍等
        return
    end
    print('登录成功了阿', sid, errno, uid)
    temp_session[sid] = os.time()
    --去DB加载数据吧
    Dbclient.post('Login.GET', sid, uid)
end

function msg_login(sid, msg)
    if not Dbclient.is_connect() then
        print('dbclient is disconnect')
        local reply = Pblua.msgnew('login.LOGIN_R')
        reply.uid = uid
        reply.errno = 4 
        Gatesrv.send_proto(sid, reply:msgname(), reply:tostring())
        return
    end
    Dbclient.post('Login.LOGIN', sid, msg.openid, msg.time, msg.sig)
end


function MSG_GETDATA(actor, msg)
    local actordata = actor.actordata

    local reply = Pblua.msgnew('login.GETDATA_R')
    reply.diamond = actordata.diamond
    reply.coin = actordata.coin
    reply.headimg = actordata.headimg
    reply.username = actordata.username

    Actor.post_msg(actor, reply)
end

