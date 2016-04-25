module('Gatesrv', package.seeall)


function dispatch_json(sid, msgname, modname, actionname, data)
    print('Gatesrv.dispatch', sid, msgname, modname, actionname, data)
    local msg = Pblua.msgnew(msgname)
    local json = Json.decode(data)
    for k, v in pairs(json) do
        msg[k] = v
    end
    print(msg:debug_string())
    local mod = _G[modname]
    local action = mod[actionname]
    action(sid, msg)
    --Gatesrv.send(sid, data, datalen)
end
