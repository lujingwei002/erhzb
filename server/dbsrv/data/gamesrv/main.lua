module('Gamesrv', package.seeall)


function on_accept(sockfd)

end

function dispatch(sockfd, data, datalen)
    print('Gamesrv.dispatch', sockfd, data, datalen)
    Gamesrv.send(sockfd, 'i am dbsrv')
end


function GET_DATA(data, datalen)

end
