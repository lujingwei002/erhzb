<?php

include "httpclient.php";

//通过code换取网页授权access_token
$appid = "wxc5748610fdf0779b";
$appsecret = "c7836b876f458aba14d53688069a1e36";
$secret = "lol_client";

$http_user_agent = isset($_SERVER['HTTP_USER_AGENT']) ? $_SERVER['HTTP_USER_AGENT'] : 'agent';
$code = isset($GET['code']) ? $_GET['code'] : 'code';

HttpClient::init($HttpClient, array('userAgent' => $http_user_agent, 'redirect' => true));

$url = "https://api.weixin.qq.com/sns/oauth2/access_token?appid=$appid&secret=$appsecret&code=$code&grant_type=authorization_code";

$HttpClient->get($url);
echo $HttpClient->buffer;
$json = $HttpClient->buffer;
$json = json_decode($json); 
var_dump($json);
if(!isset($json->openid))
{
    echo 'no login\n';
    return;
}

$access_token = $json->access_token;
$openid = $json->openid;

//返回
/*
{
    "access_token":"ACCESS_TOKEN",
    "expires_in":7200,
    "refresh_token":"REFRESH_TOKEN",
    "openid":"OPENID",
    "scope":"SCOPE"
}
 */

//拉取用户信息
$ur = "https://api.weixin.qq.com/sns/userinfo?access_token=$access_token&openid=$openid&lang=zh_CN";
/*
{
    "openid":" OPENID",
    "nickname": NICKNAME,
    "sex":"1",
    "province":"PROVINCE"
    "city":"CITY",
    "country":"COUNTRY",
    "headimgurl":    "http://wx.qlogo.cn/mmopen/g3MonUZtNHkdmzicIlibx6iaFqAc56vxLSUfpb6n5WKSYVY0ChQKkiaJSgQ1dZuTOgvLLrhJbERQQ4eMsv84eavHiaiceqxibJxCfHe/46", 
    "privilege":[
        "PRIVILEGE1"
        "PRIVILEGE2"
    ],
    "unionid": "o6_bmasdasdsad6_2sgVt7hMZOPfL"
}
 */

//生成签名


//保存到数据库

?>

