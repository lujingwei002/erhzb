module('Websocket', package.seeall)


function dispatch(sid, data, datalen)
    print('Websocket dispatch', sid, datalen)
    if Gameclient.is_connect() then
        Gameclient.send_data(sid, data, datalen)
    else
        Websocket.send_string_frame(sid, "逻辑服务器末连接")
        --Websocket.send_binary_frame(sid, data, datalen)
    end
end

function on_session_open(sid)
    print('Websocket.on_session_open', sid)
    if Gameclient.is_connect() then
        Gameclient.send_session_open(sid)
    else
        Websocket.send_string_frame(sid, "逻辑服务器末连接")
    end
end

function on_session_close(sid)
    print('Websocket.on_session_close', sid)
    if Gameclient.is_connect() then
        Gameclient.send_session_close(sid)
    else
        Websocket.send_string_frame(sid, "逻辑服务器末连接")
    end
end
