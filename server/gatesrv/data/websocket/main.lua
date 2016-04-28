module('Websocket', package.seeall)


function dispatch(sid, data, datalen)
    Log.log(string.format('Websocket dispatch sid(%d) datalen(%d)', sid, datalen))
    if Gameclient.is_connect() then
        Gameclient.send_data(sid, data, datalen)
    else
        Websocket.send_string_frame(sid, "逻辑服务器末连接")
        --Websocket.send_binary_frame(sid, data, datalen)
    end
end

function on_session_open(sid)
    Log.log(string.format('Websocket.on_session_open sid(%d)', sid))
    if Gameclient.is_connect() then
        Gameclient.send_session_open(sid)
    else
        Websocket.send_string_frame(sid, "逻辑服务器末连接")
    end
end

function on_session_close(sid)
    Log.log(string.format('Websocket.on_session_close sid(%d)', sid))
    if Gameclient.is_connect() then
        Gameclient.send_session_close(sid)
    else
        Websocket.send_string_frame(sid, "逻辑服务器末连接")
    end
end
