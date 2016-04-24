create  table  if not exists  actor
(
       uid int comment '角色ID' primary key,
       data mediumblob default NULL COMMENT '角色数据'
)
ENGINE = MYISAM;

alter table actor comment '玩家表';

