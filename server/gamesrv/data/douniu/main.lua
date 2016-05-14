module('Douniu', package.seeall)

--斗牛场
STATE_NULL = 1  --不够人时
STATE_WAITING = 2 --等待
--STATE_FAPAI = 3 --发牌
STATE_QIANGZHUANG = 3 --抢庄
STATE_XIAZHU = 4 --下注
STATE_XUANPAI = 5 --选牌
STATE_JIESUAN = 6 --结算

MEMBER_OBSERVER = 1
MEMBER_PLAYER = 2
MEMBER_MASTER = 3

--房间管理器
room_mgr = {

}
--房间状态处理函数
exit_state_handler = {}
enter_state_handler = {}
update_state_handler = {}

function main()
    init_room_mgr()
end

function on_actor_exit(actor)
    print('on_actor_exit', actor.uid)
    exit_room(actor)
end

--游戏开始时初始化房间列表
function init_room_mgr()
    local max_room_count = DouniuConf.max_room_count
    for i = 0, max_room_count do
        for type, type_conf in pairs(DouniuConf.room_type) do
            create_room(type)
        end
    end
end

--创建房间
function create_room(type)
    local type_conf = DouniuConf.room_type[type]
    local room = 
    {
        state = STATE_NULL,  
        max_actor = type_conf.max_actor,
        min_score = type_conf.min_score,
        type = type,
        --玩家列表
        player_list = {},
        actor_count = 0,
    }
    if not room_mgr[type] then
        room_mgr[type] = {}
    end
    table.insert(room_mgr[type], room)
    print('创建房间 类型', type, '底分', room.min_score)
    return room
end

--牌的外观
card_view = {
    101,102,103,104,105,106,107,108,109,110,111,112,113, 
    201,202,203,204,205,206,207,208,209,210,211,212,213, 
    301,302,303,304,305,306,307,308,309,310,311,312,313, 
    401,402,403,404,405,406,407,408,409,410,411,412,413, 
}

--牌的点数
card_val = {
    1,5,9, 13,17,21,25,29,33,37,41,45,49, 
    2,6,10,14,18,22,26,30,34,38,42,46,50, 
    3,7,11,15,19,23,27,31,35,39,43,47,51, 
    4,8,12,16,20,24,28,32,36,40,44,48,52, 
}


--牌的点数
card_num = {
    1,2,3,4,5,6,7,8,9,10,11,12,13, 
    1,2,3,4,5,6,7,8,9,10,11,12,13, 
    1,2,3,4,5,6,7,8,9,10,11,12,13, 
    1,2,3,4,5,6,7,8,9,10,11,12,13, 
}

--牌的分数
card_score = {
    1,2,3,4,5,6,7,8,9,10,10,10,10, 
    1,2,3,4,5,6,7,8,9,10,10,10,10, 
    1,2,3,4,5,6,7,8,9,10,10,10,10, 
    1,2,3,4,5,6,7,8,9,10,10,10,10, 
}

function test()
    local card_list = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,
        14,15,16,17,18,19,20,21,22,23,24,25,26,
        27,28,29,30,31,32,33,34,35,36,37,38,39,
        40,41,42,43,44,45,46,47,48,49,50,51,52
    }
    local cards = random_cards(card_list)
    print('cards')
    for i = 1, #cards do
        print(card_view[cards[i]])
    end
    local val, cards = cal_best_card_paixing(cards)
    print('va', val)
    print('cards')
    for i = 1, #cards do
        print(card_view[cards[i]])
    end
end

function cal_card_max_val(cards)
    local val_list = {card_val[cards[1]], card_val[cards[2]],card_val[cards[3]],card_val[cards[4]],card_val[cards[5]],}
    --计算出最大的一张牌
    local max_val = 0
    for i = 1, #cards do
        if max_val < val_list[i] then
            max_val = val_list[i]
        end
    end
    return max_val
end

--计算牌型
function cal_card_paixing(cards)
    local num_list = {card_num[cards[1]], card_num[cards[2]],card_num[cards[3]],card_num[cards[4]],card_num[cards[5]]}
    local score_list = {card_score[cards[1]], card_score[cards[2]],card_score[cards[3]],card_score[cards[4]],card_score[cards[5]],}

    local max_val = cal_card_max_val(cards)
    local total = score_list[1] + score_list[2] + score_list[3]
    if math.mod(total, 10) ~= 0 then
        return 0
    end
    local total = score_list[4] + score_list[5]
    local paival = math.mod(total, 10)
    if paival == 0 then paival = 10 end

    return paival, paival * 100 + max_val
end



--计算牌的分数
--5小牛 》4条 》5花牛 > 牛牛　》牛九 。。。。 牛1 》 没牛
--5小牛 手上的牌小于5点，肯总各小于10点
--5花牛 手上的牌全是JQK
function cal_best_card_paixing(cards)
    local num_list = {card_num[cards[1]], card_num[cards[2]],card_num[cards[3]],card_num[cards[4]],card_num[cards[5]]}
    local score_list = {card_score[cards[1]], card_score[cards[2]],card_score[cards[3]],card_score[cards[4]],card_score[cards[5]],}
    local val_list = {card_val[cards[1]], card_val[cards[2]],card_val[cards[3]],card_val[cards[4]],card_val[cards[5]],}

    --计算出最大的一张牌
    local max_val = cal_card_max_val(cards)
    --判断5小牛
    local total = 0
    for i = 1, #cards do
        total = total + num_list[i]
        if num_list[i] >= 5 then
            total = -1
            break
        end
    end
    if total ~= -1 and total < 10 then
        return 13, 13 * 100 + max_val, cards
    end

    --判断4炸
    local find = true
    for i = 1, #cards do
        local last_val = nil
        for j = 1, #cards do
            if i ~= j then
                if not last_val then
                    last_val = num_list[j]
                end
                if last_val ~= num_list[j] then
                    find = false
                    break
                end
            end
        end
    end
    if find then
        return 12, 12 * 100 + max_val, cards
    end
    
    --判断5花牛
    find = true
    for i = 1, #cards do
        if num_list[i] <= 10 then
            find = false
            break
        end
    end
    if find then
        return 11, 11 * 100 + max_val, cards
    end

    --判断牛几
    local last_val = 0
    local last_set = cards
    for v1 = 1, #cards - 1 do
        for v2 = v1 + 1, #cards do
            local total = 0
            for i = 1, #cards do
                if v1 ~= i and v2 ~= i then
                    total = total + score_list[i]
                end
            end
            if math.mod(total, 10) == 0 then
                local v = math.mod(score_list[v1] + score_list[v2], 10)
                if v == 0 then v = 10 end
                if v > last_val then
                    last_set = {}
                    last_val = v
                    for i = 1, #cards do
                        if v1 ~= i and v2 ~= i then
                            table.insert(last_set, cards[i])
                        end
                    end
                    table.insert(last_set, cards[v1])
                    table.insert(last_set, cards[v2])
                end
            end
        end
    end
    return last_val, last_val * 100 + max_val, last_set
end


--发牌
function random_cards(card_list)
    local cards = {}
    for i = 1, 5 do
        local idx = math.random(1, #card_list)
        local num = card_list[idx]
        card_list[idx] = card_list[#card_list]
        card_list[#card_list] = nil
        table.insert(cards, num)
    end
    return cards
end

function exit_room(actor)
    local room = actor.room
    if not room then
        print('not in room')
        return
    end
    local player_list = room.player_list
    local player = actor.douniu_player
    if not player then
        print('not douniu player')
        return
    end
    player_list[player.pos] = nil
    actor.room = nil
    actor.douniu_player = nil
    room.actor_count = room.actor_count - 1
    broadcast_actor_exit(room, actor)
    --没有人玩了
    if room.actor_count == 1 then
        if room.state >= STATE_QIANGZHUANG and room.state < STATE_JIESUAN  then
            room.state = STATE_XUANPAI
            room.countdown = nil
            goto_next_state(room)
        end
    elseif room.actor_count == 0 then
        room.state = 0
        room.countdown = nil
        goto_next_state(room)
    end
end

--随机进入房间
function enter_room(actor, type)
    if actor.room then
        print('actor is already in other room')
        return
    end
    local room_list = room_mgr[type]
    if not room_list then
        print('room type is not exists')
        return
    end
    for _, room in pairs(room_list) do
        if room.type == type and room.actor_count < room.max_actor then
            local player = {
                actor = actor,
                member = MEMBER_OBSERVER,
                cards = {},
            }
            for i = 1, room.max_actor do
                if not room.player_list[i] then
                    player.pos = i
                    room.player_list[i] = player
                    actor.room = room
                    actor.douniu_player = player
                    room.actor_count = room.actor_count + 1
                    return room
                end
            end
        end
    end
    return false
end



--更新函数
function update()
    for _, room_list in pairs(room_mgr) do
        for _, room in pairs(room_list) do
            room_update(room)
        end
    end
end

--房间更新
function room_update(room)
    --倒计时
    if room.countdown then
        print('room_update', room.countdown)
        --减1秒
        room.countdown = room.countdown - 1
        if room.countdown <= 0 then
            room.countdown = nil
            goto_next_state(room)
        end
    end

    --帧函数
    local handler = update_state_handler[room.state]
    if handler then
        handler(room)
    end
end

--如果够人了就开始游戏
update_state_handler[STATE_NULL] = function(room)
    if room.actor_count < 2 then
        return
    end
    --下一个状态
    goto_next_state(room)
end

enter_state_handler[STATE_WAITING] = function(room)
    --开始游戏
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        player.member = MEMBER_PLAYER
        player.recv_xuanpai = nil
        player.recv_xiazhu = nil
        player.recv_qiangzhuang = nil
        player.cards = nil
        player.paixing = nil
        player.paival = nil
        player.max_val= nil
        player.ratio_list = nil
        player.ratio_idx = nil
        player.is_winner = nil
    end
    room.master_player = nil
    room.win_player = nil
end

--[[
enter_state_handler[STATE_FAPAI] = function(room)
    room.card_list = {
        1,5,9, 13,17,21,25,29,33,37,41,45,49, 
        2,6,10,14,18,22,26,30,34,38,42,46,50, 
        3,7,11,15,19,23,27,31,35,39,43,47,51, 
        4,8,12,16,20,24,28,32,36,40,44,48,52, 
    }
    --发牌
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER then
            player.cards = random_cards(room.card_list)
            send_fapai(player)
        end
    end
end
--]]

enter_state_handler[STATE_QIANGZHUANG] = function(room)
    print('enter qiangzhuang', room.state)
    room.card_list = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,
        14,15,16,17,18,19,20,21,22,23,24,25,26,
        27,28,29,30,31,32,33,34,35,36,37,38,39,
        40,41,42,43,44,45,46,47,48,49,50,51,52
    }
    --发牌
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER then
            player.cards = random_cards(room.card_list)
            player.max_val = cal_card_max_val(player.cards)
            send_fapai(player)
        end
    end
end

enter_state_handler[STATE_XUANPAI] = function(room)
    --告诉每个人有哪些牌
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER then
            local msg = Pblua.msgnew('douniu.PAI_LIST')
            local msg_cards = msg.cards
            for _, c in pairs(player.cards) do
                msg_cards:add(card_view[c])
            end
            msg.best_paixing, _, _ = cal_best_card_paixing(player.cards)
            Actor.post_msg(player.actor, msg)
        end
    end
end

exit_state_handler[STATE_XUANPAI] = function(room)
    --时间到了，没有选牌的都是没牛
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER then
            if not player.recv_xuanpai then
                --牌型
                player.paixing = 0
                --牌型分数
                player.paival = player.max_val
                --广播出去
                local reply = Pblua.msgnew('douniu.FANPAI_R')
                reply.uid = player.actor.uid
                reply.paixing = 0
                local msg_cards = reply.cards
                for i = 1, #player.cards do
                    msg_cards:add(card_view[player.cards[i]])
                end
                broadcast_msg(room, reply)
            end
        end
    end
    local sort_player_list = {}
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER then
            table.insert(sort_player_list, player) 
        end
    end
    --根据分数排序，算出赢的玩家
    table.sort(sort_player_list, function(a, b)
        return a.paival> b.paival
    end)
    local win_player = sort_player_list[1]
    win_player.is_winner = true
    room.win_player = win_player
end

enter_state_handler[STATE_JIESUAN] = function(room)
    --结算
    local player_list = room.player_list
    local total_score = 0
    for _, player in pairs(player_list) do
        if not player.is_winner and player.member ~= MEMBER_OBSERVER then
            player.score = -room.min_score * player.recv_xiazhu
            total_score = total_score + player.score
        end
    end
    room.win_player.score = total_score
    --下发结算信息
    local reply = Pblua.msgnew('douniu.JIESUAN_R')
    local msg_infos = reply.infos
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER then
            local info = msg_infos:add()
            info.uid = player.actor.uid
            info.ratio = player.recv_xiazhu
            info.paixing = player.paixing
            info.score = player.score
            info.is_winner = player.is_winner and 1 or 0
            if player.member == MEMBER_MASTER then
                info.is_master = 1
            end
        end
    end
    broadcast_msg(room, reply)
end

exit_state_handler[STATE_QIANGZHUANG] = function(room)
    --是否全部人都已决定了
    local player_list = room.player_list
    local is_all_recv = true
    for _, player in pairs(player_list) do
        if not player.recv_qiangzhuang then
            is_all_recv = false
            break
        end
    end
    if is_all_recv then
        return
    end
    --时间到了，随机一个庄吧
    random_quang(room)
end


enter_state_handler[STATE_XIAZHU] = function(room)
    --要钱当前的钱数量，决定可以下的倍数
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER then
            local actordata = player.actor.actordata
            local coin = actordata.coin
            for idx, ratio_list in pairs(DouniuConf.room_type[room.type].ratio) do
                if coin >= ratio_list[1] then
                    player.ratio_list = ratio_list
                    player.ratio_idx = idx
                    print('下发下注数', player.actor.uid, player.ratio_idx)
                end
            end
        end
    end
    --下发玩家可以下的倍数(庄是不会下的)
    local master_player = room.master_player
    for _, player in pairs(player_list) do
        --todo
        if player.member ~= MEMBER_OBSERVER and player.member ~= MEMBER_MASTER then
        --if player.member ~= MEMBER_OBSERVER then
            local msg = Pblua.msgnew('douniu.XIAZHU_LIST')
            local ratio_list = player.ratio_list
            --如果庄倍数比你小
            if master_player.ratio_idx > player.ratio_idx then
                ratio_list = master_player.ratio_list
            end
            for i = 2, #ratio_list do
                msg.ratio_list:add(ratio_list[i])
            end
            Actor.post_msg(player.actor, msg)
        end
    end
end

exit_state_handler[STATE_XIAZHU] = function(room)
    --时间到了，选一个最小的注数
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        --todo
        --if player.member ~= MEMBER_OBSERVER and player.member ~= MEMBER_MASTER then
        if player.member ~= MEMBER_OBSERVER then
            if not player.recv_xiazhu then
                player.recv_xiazhu = player.ratio_list[2]
                --广播出去
                local reply = Pblua.msgnew('douniu.XIAZHU_R')
                reply.uid = player.actor.uid
                reply.ratio = player.recv_xiazhu
                broadcast_msg(room, reply)
            end
        end
    end
end

--计算这个玩家可以下的注数
function calc_player_ratio(room, player)

end

--下一个状态
function goto_next_state(room)
    --离开上一个状态
    local handler = exit_state_handler[room.state]
    if handler then
        handler(room)
    end

    room.state = room.state + 1
    if room.state > STATE_JIESUAN then
        room.state = STATE_NULL
    end

    local msg = Pblua.msgnew('douniu.ENTER_STATE_R')
    msg.state = room.state
    --广播倒计时
    local countdown = DouniuConf.countdown[room.state]
    if countdown > 0 then
        --倒计时结束自动进入下一个状态
        room.countdown = countdown
        msg.countdown = countdown
    end
    broadcast_msg(room, msg)

    --进入下一个状态
    local handler = enter_state_handler[room.state]
    if handler then
        handler(room)
    end
end


--随机一个庄
function random_quang(room)
    print('随机一个庄')
    --候选人
    local candidate = {}
    local player_list = room.player_list
    for idx, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER and player.recv_qiangzhuang == 1 then
            table.insert(candidate, idx)
        end
    end
    --没有人抢阿
    if #candidate == 0 then
        for idx, player in pairs(player_list) do
            if player.member ~= MEMBER_OBSERVER then
                table.insert(candidate, idx)
            end
        end
    end
    local idx = math.random(1, #candidate)
    local player = player_list[candidate[idx]]
    --设置为庄
    player.member = MEMBER_MASTER
    room.master_player = player
    --通知其他人
    broadcast_zhuang_result(room, player.actor.uid)
end

