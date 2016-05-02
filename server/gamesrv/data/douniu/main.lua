module('Douniu', package.seeall)

--斗牛场
STATE_NULL = 1
STATE_WAITING = 2
STATE_FAPAI = 3
STATE_QIANGZHUANG = 4
STATE_XIAZHU = 5
STATE_XUANPAI = 6
STATE_JIESUAN = 7

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
        min_score = type_conf.min_score,
        type = type,
        --玩家列表
        player_list = {},
    }
    if not room_mgr[type] then
        room_mgr[type] = {}
    end
    table.insert(room_mgr[type], room)
    print('创建房间 类型', type, '底分', room.min_score)
    return room
end

--牌的点数
card_val = {
    101,102,103,104,105,106,107,108,109,110,111,112,113, 
    201,202,203,204,205,206,207,208,209,210,211,212,213, 
    301,302,303,304,305,306,307,308,309,310,311,312,313, 
    401,402,403,404,405,406,407,408,409,410,411,412,413, 
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
        1,5,9, 13,17,21,25,29,33,37,41,45,49, 
        2,6,10,14,18,22,26,30,34,38,42,46,50, 
        3,7,11,15,19,23,27,31,35,39,43,47,51, 
        4,8,12,16,20,24,28,32,36,40,44,48,52, 
    }
    local cards = random_cards(card_list)
    print('cards')
    for i = 1, #cards do
        print(card_val[cards[i]])
    end
    local val, cards = cal_card_paixing(cards)
    print('va', val)
    print('cards')
    for i = 1, #cards do
        print(card_val[cards[i]])
    end
end

--计算牌的分数
--5小牛 》4条 》5花牛 > 牛牛　》牛九 。。。。 牛1 》 没牛
--5小牛 手上的牌小于5点，肯总各小于10点
--5花牛 手上的牌全是JQK
function cal_card_paixing(cards)
    local num_list = {card_num[cards[1]], card_num[cards[2]],card_num[cards[3]],card_num[cards[4]],card_num[cards[5]]}
    local score_list = {card_score[cards[1]], card_score[cards[2]],card_score[cards[3]],card_score[cards[4]],card_score[cards[5]],}

    --计算出最大的一张牌
    local max_val = 0
    for i = 1, #cards do
        if max_val < cards[i] then
            max_val = cards[i]
        end
    end
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


--随机进入房间
function enter_room(acotr, type)
    if actor.room then
        print('actor is already in other room')
        return
    end
    local room_list = room_mgr[type]
    for _, room in pairs(room_list) do
        if room.type == type and #room.player_list < DouniuConf.max_room_actor then
            local player = {
                actor = actor,
                member = MEMBER_OBSERVER,
                cards = {},
            }
            actor.room = room
            actor.douniu_player = player
            table.insert(room.player_list, player)
            return true
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
        --减1秒
        room.countdown = room.countdown - 1
        if room.countdown <= 0 then
            goto_next_state(room)
            room.countdown = nil
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
    if #room.player_list < 2 then
        return
    end
    --下一个状态
    goto_next_state(room)
end

enter_state_handler[STATE_WAITING] = function(room)
    --广播倒计时
    local countdown = DouniuConf.countdown[room.state]
    if countdown > 0 then
        broadcast_cown_down(room, countdown)
        --倒计时结束自动进入下一个状态
        room.countdown = countdown
    end
end

enter_state_handler[STATE_FAPAI] = function(room)
    --广播倒计时
    local countdown = DouniuConf.countdown[room.state]
    if countdown > 0 then
        broadcast_cown_down(room, countdown)
        --倒计时结束自动进入下一个状态
        room.countdown = countdown
    end
    room.card_list = {
        1,5,9, 13,17,21,25,29,33,37,41,45,49, 
        2,6,10,14,18,22,26,30,34,38,42,46,50, 
        3,7,11,15,19,23,27,31,35,39,43,47,51, 
        4,8,12,16,20,24,28,32,36,40,44,48,52, 
    }
    --发牌
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        if player.memger ~= MEMBER_OBSERVER then
            player.cards = random_cards(room.card_list)
            send_fapai(player)
        end
    end
end

enter_state_handler[STATE_QIANGZHUANG] = function(room)

end

enter_state_handler[STATE_XUANPAI] = function(room)
    --开始倒数
    local countdown = DouniuConf.countdown[room.state]
    if countdown > 0 then
        broadcast_cown_down(room, countdown)
        --倒计时结束自动进入下一个状态
        room.countdown = countdown
    end
    --告诉每个人有哪些牌
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        if player.memger ~= MEMBER_OBSERVER then
            local msg = Pblua.msgnew('douniu.XUANPAI_R')
            local msg_cards = msg.cards
            for _, c in pairs(player.cards) do
                msg_cards:add(card_val[c])
            end
            msg.best_paixing, _, _ = cal_card_paixing(player.cards)
            Actor.post_msg(player.actor, msg)
        end
    end
end

enter_state_handler[STATE_JIESUAN] = function(room)

end

exit_state_handler[STATE_QIANGZHUANG] = function(room)
    --时间到了，随机一个庄吧
    random_quang(room)
end


enter_state_handler[STATE_XIAZHU] = function(room)
    --要钱当前的钱数量，决定可以下的倍数
    local player_list = room.player_list
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER then
            local coin = player.actor.coin
            for idx, ratio_list in pairs(DouniuConf.room_type[room.type].ratio) do
                if coin >= ratio_list[1] then
                    player.ratio_list = radio_list
                    player.ratio_idx = idx
                end
            end
        end
    end
    --下发玩家可以下的倍数
    local master_player = room.master_player
    for _, player in pairs(player_list) do
        if player.member ~= MEMBER_OBSERVER and player.member ~= MEMBER_MASTER then
            local msg = Pblua.msgnew('douniu.XIAZHU_LIST')
            local ratio_list = player.radio_list
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
        if player.member ~= MEMBER_OBSERVER and player.member ~= MEMBER_MASTER then
            if not player.recv_xiazhu then
                player.recv_xiazhu = player.ratio_list[2]
                --广播出去
                local reply = Pblua.msgnew('douniu.XIAZHU_R')
                reply.uid = player.actor.uid
                reply.ratio = player.recv_xiazhu
                broadcast_msg(msg)
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
    local msg = Pblua.msgnew('douniu.ENTER_STATE_R')
    msg.state = room.state
    broadcast_msg(room, msg)

    --进入下一个状态
    local handler = enter_state_handler[room.state]
    if handler then
        handler(room)
    end
end


--随机一个庄
function random_quang(room)
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

