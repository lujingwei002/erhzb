module('Dbclient', package.seeall)


function on_connect(sockfd)
    print('Dbclient.on_connect', sockfd)
    --Dbclient.send(sockfd, "hello", 6)
    --加载数据
    local uid = 1
    Dbclient.post('Login.GET', uid)
end


function dispatch(sockfd, data, datalen)
    print('Dbclient.dispatch', sockfd, data, datalen)
end

