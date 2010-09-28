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

include_once(S_ROOT.'./data/data_network.php');

//日志
     $viewstr=array(
            'lastvisit'=>'lastvisit',
            'lastadd'=>'dateline',
            'oftenvisit'=>'viewnum',
            'lastrecommend'=>'lastvisit'
            );

	//显示数量
	  $shownum = 6;
    //$userbrowertype=getuserbrowserarray();
    //显示类别如最近访问，最新添加etc...
    $see=empty($_GET['see'])?'':$_GET['see'];
    //浏览器类型
    $browserid=(empty($_GET['browserid']))?0:intval($_GET['browserid']);
    if(!in_array($browserid,$_SGLOBAL['browsertype']))
        $browserid=0;	
    $tagid=empty($_GET['tagid'])?0:intval($_GET['tagid']);
    //获取总条数
    $page=empty($_GET['page'])?0:intval($_GET['page']);
    $perpage=$_SC['bookmark_show_maxnum'];
    $start=$page?(($page-1)*$perpage):0;
    $theurl="space.php?uid=$space[uid]&do=$do&tagid=$tagid";
    $count = $_SGLOBAL['db']->result($_SGLOBAL['db']->query("SELECT COUNT(*) FROM ".tname('sitetagsite')." main where main.uid=".$_SGLOBAL['supe_uid']." AND main.tagid=".$tagid),0);
    //获取tag名字
    $tagname=$_SGLOBAL['db']->result($_SGLOBAL['db']->query("SELECT tagname FROM ".tname('sitetag')." main where main.tagid=".$tagid),0);
    $tagname="标签:".$tagname;
    //获取bookmarklist

	$query = $_SGLOBAL['db']->query("SELECT main.*, sub.* FROM ".tname('sitetagsite')." main
		LEFT JOIN ".tname('bookmark')." sub ON main.bmid=sub.bmid where main.uid=".$_SGLOBAL['supe_uid']." AND main.tagid=".$tagid." ORDER BY sub.dateline DESC limit ".$start." , ".$_SC['bookmark_show_maxnum']);
	$bookmarklist = array();
	while ($value = $_SGLOBAL['db']->fetch_array($query)) {

		//获取其bookmark对应的link的信息如pic等
		$link_query = $_SGLOBAL['db']->query("SELECT main.*, sub.* FROM ".tname('bookmark')." main
		LEFT JOIN ".tname('link')." sub ON main.linkid=sub.linkid where main.bmid=".$value['bmid']);

		$value = $_SGLOBAL['db']->fetch_array($link_query);
		
		//处理link在site中的情况
		include_once(S_ROOT.'./source/function_common.php');
		$value = getlinkfromsite($value);
		$value['taglist'] = empty($value['tag'])?array():unserialize($value['tag']);
		if(empty($value['description']))
				$value['description']=$value['link_description'];
		if(empty($value['taglist'])){
			//如果bookmark没有tag，则取link的tag
			$value['taglist'] = empty($value['link_tag'])?array():unserialize($value['link_tag']);
		}
		$value['description'] = getstr($value['description'], 86, 0, 0, 0, 0, -1);
		$value['subject'] = getstr($value['subject'], 50, 0, 0, 0, 0, -1);
		$bookmarklist[] = $value;
	}
foreach($bookmarklist as $key => $value) {
	realname_set($value['uid'], $value['username']);
	$bookmarklist[$key] = $value;
}
//分页
$multi = multi($count, $perpage, $page, $theurl,'','bmcontent');

$_TPL['css'] = 'network';
include_once template("space_linktag");

?>
