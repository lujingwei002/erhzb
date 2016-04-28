

create  table  if not exists  account
(
       uid int comment '角色ID' auto_increment,
       openid varchar(64) comment '平台id',
       sig varchar(128) comment '签名',
       time int comment '登录时间戳',
       headimg varchar(128) comment '头像地址',
       username varchar(128) comment '用户名',
       primary key(uid)
)
ENGINE = MYISAM;

alter table account comment '账号表';


create  table  if not exists  actor
(
    uid int comment '角色ID',
    data mediumblob default NULL COMMENT '角色数据',
    diamond int comment '钻石，元宝',
    coin int comment '铜钱，银两',
    primary key(uid)
)
ENGINE = MYISAM;

alter table actor comment '玩家表';

