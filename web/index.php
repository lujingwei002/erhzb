<?php


$appid = "wxc5748610fdf0779b";
$scope = "snsapi_userinfo";
$redirect_uri = "https://www.baidu.com/haha";
$state = "haha";

$url = "https://open.weixin.qq.com/connect/oauth2/authorize?appid=$appid&redirect_uri=$redirect_uri&response_type=code&scope=$scope&state=$state#wechat_redirect";
//重定向到登录页面
header("Location: $url");
echo $url;
?>

