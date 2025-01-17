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
$ops=array('manage','add','edit','delete','checkseccode');
//检查信息
$op = (empty($_GET['op']) || !in_array($_GET['op'], $ops))?'add':$_GET['op'];
$linkclassid= empty($_GET['linkclassid'])?0:intval(trim($_GET['linkclassid']));
$linkclassitem = array();
if($linkclassid)
{
	$query=$_SGLOBAL['db']->query("SELECT main.* FROM ".tname('linkclass')." main where main.classid=".$linkclassid);
	$linkclassitem = $_SGLOBAL['db']->fetch_array($query);
}
/*
	permit owner id item 
0--不关心
1--需要符合条件
2--几个中有一个符合即可
*/
$linkclass_priority=array(
 'manage'=>array('permit'=>1,'owner'=>0,'id'=>0,'item'=>0),
 'add'=>array('permit'=>1,'owner'=>0,'id'=>0,'item'=>0),
 'edit'=>array('permit'=>1,'owner'=>0,'id'=>1,'item'=>1),
 'delete'=>array('permit'=>1,'owner'=>0,'id'=>1,'item'=>1),	 
 'checkseccode'=>array('permit'=>0,'owner'=>0,'id'=>0,'item'=>0), 	
);
$ret=check_valid($op,$linkclassid,$linkclassitem,$linkclassitem['uid'],'managelinkclass',$linkclass_priority);
switch($ret)
{
	case -1:
		showmessage('no_authority_to_do_this');
	break;
	case -2:
		showmessage('error_parameter');
	break;
	default:
	break;
}

$wherearr='';
$orderarr='';
$theurl='';
$linkclasslist=array();
$manageclass=array();
if($op=='manage'){
	
	$classid= empty($_GET['classid'])?0:intval(trim($_GET['classid']));

	$groupid=0;
	//获取class分类
	$class_query  = $_SGLOBAL['db']->query("SELECT main.* FROM ".tname('linkclass')." main WHERE main.parentid=0");
	while($value =$_SGLOBAL['db']->fetch_array($class_query))
	{
		//获取二级目录
		$classnd_query  = $_SGLOBAL['db']->query("SELECT main.* FROM ".tname('linkclass')." main WHERE main.parentid=".$value['groupid']);
		while($classnd_value =$_SGLOBAL['db']->fetch_array($classnd_query))
		{

				
			/*
			//获取三级目录
			$classrd_query  = $_SGLOBAL['db']->query("SELECT main.* FROM ".tname('linkclass')." main WHERE main.parentid=".$classnd_value['groupid']);
			while($classrd_value =$_SGLOBAL['db']->fetch_array($classrd_query))
			{
				
			
				$classnd_value['grandson'][]= $classrd_value;
				
			}
			*/
			//获取本层class的tag
			$classtag_query  = $_SGLOBAL['db']->query("SELECT main.*, field.* FROM ".tname('linkclass')." main	LEFT JOIN ".tname('linkclasstag')." field ON main.classid=field.classid  WHERE main.classid=".$classnd_value['classid']);
			while($classtag_value =$_SGLOBAL['db']->fetch_array($classtag_query))
			{
						$classnd_value['tag'][]= $classtag_value;
			}
			$value['son'][]=$classnd_value;
		}
		$linkclasslist[]=$value;
	}
	if($classid)
	{
			$query=$_SGLOBAL['db']->query("SELECT main.* FROM ".tname('linkclass')." main where main.classid=".$classid);
			$classitem = $_SGLOBAL['db']->fetch_array($query);
			if(empty($classitem))   $classid=0;
			else
				$groupid=$classitem['groupid'];
	}

	if($classid){

			//获取三级目录
			$classrd_query  = $_SGLOBAL['db']->query("SELECT main.* FROM ".tname('linkclass')." main WHERE main.parentid=".$groupid);
			while($classrd_value =$_SGLOBAL['db']->fetch_array($classrd_query))
			{
				$manageclass['son'][]= $classrd_value;				
			}
			//获取本层class的tag
			$classtag_query  = $_SGLOBAL['db']->query("SELECT main.*, field.* FROM ".tname('linkclass')." main	LEFT JOIN ".tname('linkclasstag')." field ON main.classid=field.classid  WHERE main.classid=".$classid);
			while($classtag_value =$_SGLOBAL['db']->fetch_array($classtag_query))
			{
						$manageclass['tag'][]= $classtag_value;
			}

	}
	else
		$manageclass= $linkclasslist[0]['son'][0];
}
elseif($op=='add'){
	$classid= empty($_POST['classid'])?0:intval(trim($_POST['classid']));
	$groupid=0;
	//获取class分类
	$class_query  = $_SGLOBAL['db']->query("SELECT main.* FROM ".tname('linkclass')." main WHERE main.classid=".$classid);
	$classitem = $_SGLOBAL['db']->fetch_array($class_query);
	$groupid=$classitem['groupid'];
	if(submitcheck('addsubmit')){
		//验证码
		if(checkperm('seccode') && !ckseccode($_POST['seccode'])) {
			showmessage('incorrect_code');
		}
		include_once(S_ROOT.'./source/function_linkclass.php');
		$ret=linkclass_post($_POST,$classitem);
		if(is_array($ret)) {
			$url = $_SGLOBAL['refer'];		
			showmessage('do_success',$url,0);
		} elseif($ret==false) {
			showmessage('that_should_at_least_write_things');
		}elseif($ret==-1) {
			showmessage('link_has_existed');
		}
	}

}else{
}


//最后登录名
$membername = empty($_SCOOKIE['loginuser'])?'':sstripslashes($_SCOOKIE['loginuser']);
$wheretime = $_SGLOBAL['timestamp']-3600*24*30;

$_TPL['css'] = 'network';
include_once template("cp_linkclass");


?>
