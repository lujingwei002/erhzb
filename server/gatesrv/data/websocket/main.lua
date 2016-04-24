module('Websocket', package.seeall)

print('Websocket module')

function dispatch(sid, data, datalen)
    print('Websocket dispatch', sid, data, datalen)
    --Websocket.send_string_frame(sid, data)
    --Websocket.send_frame(sid, data, datalen)
    Gameclient.send(sid, data, datalen)
end
