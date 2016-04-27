module('Dounie', package.seeall)

--斗牛场
STATE_WAITING = 0
STATE_FAPAI = 1
STATE_QIANGZHUANG = 2
STATE_XIAZHU = 3
STATE_XUANPAI = 4
STATE_JIESUAN = 5

--房间管理器
room_mgr = {

}
--房间状态处理函数
enter_state_handler = {
}

--游戏开始时初始化房间列表
function init_room_mgr()
    local max_room_count = DounieConf.max_room_count
    for i = 0, max_room_count do
        create_room()
    end
end

--创建房间
function create_room()
    local room = 
    {
        --玩家列表
        player_list = {},
        --玩家列表
        actor_list = {},
    }
    table.insert(room_mgr, room)
    room.id = #room_mgr
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
    local cards = random_card()
    print('cards')
    for i = 1, #cards do
        print(card_val[cards[i]])
    end
    local val, cards = cal_card_score(cards)
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
function cal_card_score(cards)
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
        return 20 * 100 + max_val, cards
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
        return 19 * 100 + max_val, cards
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
        return 18 * 100 + max_val, cards
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
    return last_val * 100 + max_val, last_set
end












--发牌
function random_card()
    local card_list = {
        1,5,9, 13,17,21,25,29,33,37,41,45,49, 
        2,6,10,14,18,22,26,30,34,38,42,46,50, 
        3,7,11,15,19,23,27,31,35,39,43,47,51, 
        4,8,12,16,20,24,28,32,36,40,44,48,52, 
    }
    local cards = {}
    for i = 1, 5 do
        local idx = math.random(1, #card_list)
        local num = card_list[idx]
        card_list[idx] = card_list[#card_list]
        card_list[#card_list] = nil
        table.insert(cards, num)
    end
    return cards
    --[[
    for _, player in pairs(room.player_list) do
        player.card = card
    end
    --]]
end


--随机进入房间
function enter_room(acotr, type)

end


--广播消息给房间里的玩家
function broadcast_msg(room, msg)
    local actor_list = room.actor_list
    for _, actor in pairs(actor_list) do
        Actor.post_msg(actor, msg)
    end
end


--更新函数
function update()

end


enter_state_handler[STATE_WAITING] = function(room)

end

enter_state_handler[STATE_FAPAI] = function(room)
    local countdown = DounieConf.countdown[room.state]
    if countdown > 0 then
        broadcast_cown_down(room, countdown)
    end
end


enter_state_handler[STATE_QIANGZHUANG] = function(room)

end

enter_state_handler[STATE_XUANPAI] = function(room)
    --开始倒数
    local countdown = DounieConf.countdown[room.state]
    if countdown > 0 then
        broadcast_cown_down(room, countdown)
    end
end

enter_state_handler[STATE_JIESUAN] = function(room)

end

--下一个状态
function goto_next_state(room)
    room.state = room.state + 1
    enter_state_handler[room.state](room)
end

--广播倒计时
function broadcast_cown_down(room, countdown)

end


--广播房间的状态
function broadcast_room_info(room)

end

