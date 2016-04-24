module('Gatesrv', package.seeall)


function dispatch_json(sid, msgname, data)
    print('Gatesrv.dispatch', sid, msgname, data)
    local msg = Pblua.msgnew(msgname)
    local json = Json.decode(data)
    for k, v in pairs(json) do
        msg[k] = v
    end
    print(msg:debug_string())
    --Gatesrv.send(sid, data, datalen)
end
