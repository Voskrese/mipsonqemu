<?php
/*
	[UCenter Home] (C) 2007-2008 Comsenz Inc.
	$Id: network_album.php 12078 2009-05-04 08:28:37Z zhengqingpeng $
*/

if(!defined('IN_UCHOME')) {
	exit('Access Denied');
}
//今日热荐
$todayhot=array();
$cachefile=S_ROOT.'./data/todayhot.txt';
if(!check_todayhot_cache($cachefile)) {
	include_once(S_ROOT.'./source/function_cache.php');
	everydayhot_cache();
}

$todayhot = unserialize(sreadfile($cachefile));

function check_todayhot_cache() {
		global $_SGLOBAL;
		$cachefile = S_ROOT.'./data/todayhot.txt';
		if(!file_exists($cachefile))
			return false;
		$ftime = filemtime($cachefile);
		//24 hours
		if($_SGLOBAL['timestamp'] - $ftime < (24*60*60)) {
			return true;
		}
		return false;
}
?>
