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
	if(!file_exists(S_ROOT.'./data/todayhot.txt')){
	/*
		$todayhot = array();
		$todayhotlist = array();
		$_SCONFIG['todayhot']='1,2,3,4';
		if($_SCONFIG['todayhot']) {
			$query = $_SGLOBAL['db']->query("SELECT * FROM ".tname('link')." WHERE linkid IN (".simplode(explode(',', $_SCONFIG['todayhot'])).")");
			while ($value = $_SGLOBAL['db']->fetch_array($query)) {
				$todayhotlist[] = $value;
			}
		}
		if($todayhotlist) {
			$todayhot = sarray_rand($todayhotlist, 1);
		}
		foreach($todayhot as $key => $value) {
			$value['link_short_subject'] = getstr(trim($value['link_subject']), $_SC['subject_todayhot_length']);	
			$value['link_short_description'] = getstr(trim($value['link_description']), $_SC['description_todayhot_length']);
			include_once(S_ROOT.'./source/function_link.php');
			$value['link_tag'] = convertlinktag($value['linkid'],$value['link_tag']);
			$value['link_tag'] = empty($value['link_tag'])?array():unserialize($value['link_tag']);
			//$value['link_tag'] = getstr(trim($value['link_tag']), 40);
			$todayhot[$key]=$value;
		}
	 */
		include_once(S_ROOT.'./source/function_cache.php');
		everydayhot_cache();

	}
	$todayhot = unserialize(sreadfile(S_ROOT.'./data/todayhot.txt'));
?>
