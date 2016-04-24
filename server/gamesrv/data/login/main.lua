module('Login', package.seeall)


function GET(uid, data)
    print('Login.GET', uid, data)
    local actordata = Pblua.msgnew('dbproto.User')
    actordata:parse_from_string(data or '')
    actordata.uid = uid
    print(actordata:debug_string())
    Dbclient.post('Login.SET', uid, actordata:tostring())
end
