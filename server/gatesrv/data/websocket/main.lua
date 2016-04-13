module('Websocket', package.seeall)


function dispatch(sid, data, datalen)
    print('Websocket dispatch', sid, datalen)
    if Gameclient.is_connect() then
        Gameclient.send(sid, data, datalen)
    else
        --Websocket.send_string_frame(sid, "逻辑服务器末连接")
        Websocket.send_binary_frame(sid, data, datalen)
    end
end
