module('Gatesrv', package.seeall)


function dispatch_json(sid, msgname, modname, actionname, data)
    print('Gatesrv.dispatch', sid, msgname, modname, actionname)
    local msg = Pblua.msgnew(msgname)
    local json = Json.decode(data)
    for k, v in pairs(json) do
        msg[k] = v
    end
    dispatch_msg(sid, modname, actionname, msg)
end

function dispatch_proto(sid, msgname, modname, actionname, data)
    print('Gatesrv.dispatch', sid, msgname, modname, actionname)
    local msg = Pblua.msgnew(msgname)
    msg:parse_from_string(data)
    dispatch_msg(sid, modname, actionname, msg)
end

function dispatch_msg(sid, modname, actionname, msg)
    print(msg:debug_string())
    local mod = _G[modname]
    local action = mod['MSG_'..actionname]
    local actor = Actor.actor_session[sid]
    if not actor then
        Login.msg_login(sid, msg)
        return
    end
    action(actor, msg)
end

function on_session_open(sid)
    print('Gatesrv.on_session_open', sid)
end

function on_session_close(sid)
    print('Gatesrv.on_session_close', sid)
    local actor = Actor.actor_session[sid]
    if not actor then
        return
    end
    local actordata = actor.actordata
    Dbclient.post('Login.SET', sid, actor.uid, actordata:tostring())
end

function MSG_ECHO(sid, msg)
    print('echo')
    Gatesrv.send_proto(sid, msg:msgname(), msg:tostring())
end
