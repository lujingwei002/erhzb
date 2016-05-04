module('Douniu', package.seeall)

--进入房间
function MSG_ENTER(actor, msg)
    local type = msg.type
    local room = enter_room(actor, type)
    if not room then
        local reply = Pblua.msgnew('douniu.ENTER_R')  
        reply.errno = 1
        reply.errstr = '进入房间失败'
        Actor.post_msg(actor, reply)
        return
    end
    local reply = Pblua.msgnew('douniu.ENTER_R')  
    reply.errno = 0
    Actor.post_msg(actor, reply)
    
    broadcast_actor_enter(room, actor)
    send_room_info(actor, room)
end

function MSG_EXIT(actor, msg)
    local room = actor.room
    if not room then
        print('actor is not in room now')
        return
    end
    exit_room(actor)
end

--广播玩家退出房间
function broadcast_actor_exit(room, actor)
    print('broadcast_actor_exit')
    local actordata = actor.actordata
    local msg = Pblua.msgnew('douniu.ACTOR_EXIT_R')  
    msg.uid = actordata.uid
    broadcast_msg(room, msg)
end


--广播玩家进入房间
function broadcast_actor_enter(room, actor)
    local actordata = actor.actordata
    local msg = Pblua.msgnew('douniu.ACTOR_ENTER_R')  
    msg.uid = actordata.uid
    msg.username = actordata.username
    msg.headimg = actordata.headimg
    msg.viplevel = actordata.viplevel
    msg.pos = actor.douniu_player.pos
    broadcast_msg(room, msg)
end

--广播消息给房间里的玩家
function broadcast_msg(room, msg)
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        Actor.post_msg(player.actor, msg)
    end
end


function send_fapai(player)
    local msg = Pblua.msgnew('douniu.FAPAI_R')
    local msg_cards = msg.cards
    for i = 1, 3 do
        msg_cards:add(card_val[player.cards[i]])
    end
    Actor.post_msg(player.actor, msg)
end

--广播房间的状态
function send_room_info(actor, room)
    local msg = Pblua.msgnew('douniu.ROOM_INFO_R')
    msg.state = room.state
    msg.countdown = room.countdown
    local player_list = room.player_list
    local msg_player_list = msg.player_list
    for _, player in pairs(player_list) do
        local actordata = player.actor.actordata
        local player_info = msg_player_list:add()
        player_info.uid = actordata.uid
        player_info.username = actordata.username
        player_info.headimg = actordata.headimg
        player_info.viplevel = actordata.viplevel
        player_info.member = player.member
        player_info.pos = player.pos
        player_info.coin = actordata.coin
        local msg_cards = player_info.cards
        for _, c in pairs(player.cards) do
            msg_cards:add(c)
        end
    end
    Actor.post_msg(actor, msg)
end

--抢庄
function MSG_QIANGZHUANG(actor, msg)
    local room = actor.room
    if not room then
        --不在房间
        return
    end
    if room.state ~= STATE_QIANGZHUANG then
        --状态不对
        return
    end
    local player = actor.douniu_player
    if player.recv_qiangzhuang then
        --已经抢过了
        return
    end
    player.recv_qiangzhuang = msg.is_master
    --广播出去
    local reply = Pblua.msgnew('douniu.QIANGZHUANG_R')
    reply.uid = actor.uid
    reply.is_master = msg.is_master
    broadcast_msg(room, reply)
    --是否全部人都已决定了
    local player_list = room.player_list
    local is_all_recv = true
    for _, player in pairs(player_list) do
        if not player.recv_qiangzhuang then
            is_all_recv = false
            break
        end
    end
    if not is_all_recv then
        return
    end
    --随机一个庄
    random_quang(room)
    --马上到下一个状态了
    goto_next_state(room)
end

function broadcast_zhuang_result(room, uid)
    local msg = Pblua.msgnew('douniu.ZHUANG_R')
    msg.uid = uid
    broadcast_msg(room, msg)
end


--下注
function MSG_XIAZHU(actor, msg)
    local room = actor.room
    if not room then
        --不在房间
        return
    end
    if room.state ~= STATE_XIAZHU then
        --状态不对
        return
    end
    local player = actor.douniu_player
    if player.recv_xiazhu then
        --已经下过注了
        return
    end
    local ratio_list = player.ratio_list
    local find = false
    for i = 2, #ratio_list do
        if ratio_list[i] == msg.ratio then
            find = true
            break
        end
    end
    if not find then
        print('倍数有误')
        return
    end

    player.recv_xiazhu = msg.ratio
    --广播出去
    local reply = Pblua.msgnew('douniu.XIAZHU_R')
    reply.uid = actor.uid
    reply.ratio = msg.ratio
    broadcast_msg(room, reply)

    --是否全部人都已决定了
    local player_list = room.player_list
    local is_all_recv = true
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER and player.member ~= MEMBER_MASTER then
            if not player.recv_xiazhu then
                is_all_recv = false
                break
            end
        end
    end
    if not is_all_recv then
        return
    end
    goto_next_state(room)
end

--提示
function MSG_XUANPAI_NONE(actor, msg)
    local room = actor.room
    if not room then
        --不在房间
        return
    end
    if room.state ~= STATE_XUANPAI then
        --状态不对
        return
    end
    local player = actor.douniu_player
    if player.recv_xuanpai then
        --已经选过牌了
        return
    end

    player.recv_xuanpai = true
    player.paixing = 0 
    player.paival = 0 

    --广播出去
    local reply = Pblua.msgnew('douniu.FANPAI_R')
    reply.uid = actor.uid
    reply.paixing = paixing
    for _, p in pairs(player.cards) do
        reply.cards:add(card_val[p])
    end
    broadcast_msg(room, reply)

    --是否全部人都已决定了
    local player_list = room.player_list
    local is_all_recv = true
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER then
            if not player.recv_xuanpai then
                is_all_recv = false
                break
            end
        end
    end
    if not is_all_recv then
        return
    end
    goto_next_state(room)
end



--提示
function MSG_XUANPAI_TIP(actor, msg)
    local room = actor.room
    if not room then
        --不在房间
        return
    end
    if room.state ~= STATE_XUANPAI then
        --状态不对
        return
    end
    local player = actor.douniu_player
    if player.recv_xuanpai then
        --已经选过牌了
        return
    end

    player.recv_xuanpai = true
    local best_paixing, paival, cards_set = cal_best_card_paixing(player.cards)
    player.paixing = best_paixing
    player.paival = paival 

    --广播出去
    local reply = Pblua.msgnew('douniu.FANPAI_R')
    reply.uid = actor.uid
    reply.paixing = paixing
    for _, p in pairs(cards_set) do
        reply.cards:add(card_val[p])
    end
    broadcast_msg(room, reply)

    --是否全部人都已决定了
    local player_list = room.player_list
    local is_all_recv = true
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER then
            if not player.recv_xuanpai then
                is_all_recv = false
                break
            end
        end
    end
    if not is_all_recv then
        return
    end
    goto_next_state(room)
end

--选牌
function MSG_XUANPAI(actor, msg)
    local room = actor.room
    if not room then
        --不在房间
        return
    end
    if room.state ~= STATE_XUANPAI then
        --状态不对
        return
    end
    local player = actor.douniu_player
    if player.recv_xuanpai then
        --已经选过牌了
        return
    end

    player.recv_xuanpai = true
    local cards = {}
    local msg_cards = msg.cards
    for _, c in pbpairs(msg_cards) do
        table.insert(cards, player.cards[c])
    end
    local paixing, paival= cal_card_paixing(cards)
    player.paixing = paixing
    player.paival = paival 

    --广播出去
    local reply = Pblua.msgnew('douniu.FANPAI_R')
    reply.uid = actor.uid
    reply.paixing = paixing
    reply.cards:copy_from(msg.cards)
    broadcast_msg(room, reply)

    --是否全部人都已决定了
    local player_list = room.player_list
    local is_all_recv = true
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER then
            if not player.recv_xuanpai then
                is_all_recv = false
                break
            end
        end
    end
    if not is_all_recv then
        return
    end
    goto_next_state(room)
end


