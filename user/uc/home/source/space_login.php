<?php
/*
	[UCenter Home] (C) 2007-2008 Comsenz Inc.
	$Id: network_album.php 12078 2009-05-04 08:28:37Z zhengqingpeng $
*/

if(!defined('IN_UCHOME')) {
	exit('Access Denied');
}

//是否公开
if(empty($_SCONFIG['networkpublic'])) {
	checklogin();//需要登录
}

include_once(S_ROOT.'./source/space_bookmark_show.php');

//今日热荐
include_once(S_ROOT.'./source/todayhotcollect.php');
include_once(S_ROOT.'./source/sitefeed.php');
//最后登录名
$membername = empty($_SCOOKIE['loginuser'])?'':sstripslashes($_SCOOKIE['loginuser']);
$wheretime = $_SGLOBAL['timestamp']-3600*24*30;
if(!empty($_SCOOKIE['cookiecheck']))
	$cookiecheck = ' checked';
$_TPL['css'] = 'network';
include_once template("space_login");


?>