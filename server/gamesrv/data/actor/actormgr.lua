module('Actor', package.seeall)

actor_mgr = {}
actor_session = {}

function destory_actor(sid, uid)
    print('destory_actor', sid, uid)
    actor_session[sid] = nil
    local actor = actor_mgr[uid]
    if actor.sid == sid then
        actor_mgr[uid] = nil
    end
end

function create_actor(sid, actordata)
    local actor = {
        sid = sid,
        uid = actordata.uid,
        actordata = actordata,
    }
    actor_mgr[actor.uid] = actor
    actor_session[sid] = actor
end


function post_msg(actor, msg)
    Gatesrv.send_proto(actor.sid, msg:msgname(), msg:tostring())
end
