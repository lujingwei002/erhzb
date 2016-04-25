module('Gameclient', package.seeall)

function dispatch(sid, data, datalen)
    print('Gameclient.dispatch', sid, data, datalen)
    Websocket.send_frame(sid, data, datalen)
end

