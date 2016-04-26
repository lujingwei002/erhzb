module('Dbclient', package.seeall)


function on_connect(sockfd)
    print('Dbclient.on_connect', sockfd)
    --Dbclient.send(sockfd, "hello", 6)
    --加载数据
    --local uid = 1
    --local sid = 0
    --Dbclient.post('Login.GET', sid, uid)
end


function dispatch(sockfd, data, datalen)
    print('Dbclient.dispatch', sockfd, data, datalen)
end

