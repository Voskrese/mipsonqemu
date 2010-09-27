<?php
/*
	[UCenter Home] (C) 2007-2008 Comsenz Inc.
	$Id: cp_blog.php 13026 2009-08-06 02:17:33Z liguode $
*/
if(!defined('IN_UCHOME')) {
	exit('Access Denied');
}
//bookmarkΪ"��Ҫ�ղ�"
//getΪ���
//$ops=array('checkerror','manage','add','edit','delete','pass','reject','checkseccode','get','relate','bookmark','updatesiteupnum','updatesitedownnum','updatesiteviewnum','reporterr','toolbar');
$ops=array('checkerror','add','edit','delete','checkseccode','get','relate','bookmark','updatesiteupnum','updatesitedownnum','updatesiteviewnum','reporterr');
//�����Ϣ
$op = (empty($_GET['op']) || !in_array($_GET['op'], $ops))?'add':$_GET['op'];
$siteid= empty($_GET['siteid'])?0:intval(trim($_GET['siteid']));
$item = array();
$relatedlist = array();
if($siteid)
		$item = getsite($siteid);
/*
	permit owner id item 
0--������
1--��Ҫ��������
2--��������һ�����ϼ���
*/
$link_priority=array(
 'checkerror'=>array('permit'=>1,'owner'=>0,'id'=>0,'item'=>0),
// 'manage'=>array('permit'=>1,'owner'=>0,'id'=>0,'item'=>0),
 'add'=>array('permit'=>0,'owner'=>0,'id'=>0,'item'=>0),
 'edit'=>array('permit'=>2,'owner'=>2,'id'=>1,'item'=>1),
 'delete'=>array('permit'=>2,'owner'=>2,'id'=>1,'item'=>1),	
// 'pass'=>array('permit'=>1,'owner'=>0,'id'=>1,'item'=>1),
// 'reject'=>array('permit'=>1,'owner'=>0,'id'=>1,'item'=>1),
 'checkseccode'=>array('permit'=>0,'owner'=>0,'id'=>0,'item'=>0), 
 'get'=>array('permit'=>0,'owner'=>0,'id'=>1,'item'=>1),
 'relate'=>array('permit'=>0,'owner'=>0,'id'=>1,'item'=>1),
 'bookmark'=>array('permit'=>0,'owner'=>0,'id'=>1,'item'=>1),
 'updatesiteupnum'=>array('permit'=>0,'owner'=>0,'id'=>1,'item'=>1),
 'updatesitedownnum'=>array('permit'=>0,'owner'=>0,'id'=>1,'item'=>1),
 'updatesiteviewnum'=>array('permit'=>0,'owner'=>0,'id'=>1,'item'=>1),
 'reporterr'=>array('permit'=>0,'owner'=>0,'id'=>1,'item'=>1),
// 'toolbar'=>array('permit'=>1,'owner'=>0,'id'=>0,'item'=>0)
);
//Ȩ�޼��
$ret=check_valid($op,$siteid,$item,$item['postuid'],'managelink',$link_priority);
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
include_once(S_ROOT.'./source/function_site.php');
if($op == 'get'){
	//�õ�siteclassname
	$item['classname']=getsiteclassname($item['class']);
	//�õ���site�����site
	$item['relate'] = getrelatesite($item['class'],$siteid);
	$big_nums = 0;
	foreach($item['relate'] as $key=>$v){
		$item['bigrelate'][$key] = $item['relate'][$key];
		unset($item['relate'][$key]);
		//ȡ��ǰ3����Ϊbig relate
		if((++$big_nums)>=$_SC['related_big_site_num'])
			break;
	}
	//��ȡhotclass
	$hotclass = unserialize(sreadfile(S_ROOT.'./data/todayhotclass.txt'));
	$hottag = unserialize(sreadfile(S_ROOT.'./data/todayhottag.txt'));
}
elseif($op == 'edit'){
		//����edit�ύ
		if(submitcheck('editsubmit')) {
			$item = site_post($_POST, $item);
			if(is_array($item)) {
				showmessage('do_success',$_SGLOBAL['refer']);
			} elseif($item) {
				showmessage('do_error');
			}
		}		
		//��ȷ��ʾtag��array-->string
		$item['tag'] =implode(" ",$item['tag']); 
}
elseif($op == 'relate'){
		//��ȷ��ʾtag
		$item['link_tag'] = empty($item['link_tag'])?array():unserialize($item['link_tag']);
		$relatedlist[]=$item;
}
elseif($_GET['op'] == 'delete') {
	//ɾ��
	if(submitcheck('deletesubmit')) {
		if(deletelink($siteid)) {
			$url =$_SGLOBAL['refer'];
			showmessage('do_success', $url, 0);
		} else {
			showmessage('failed_to_delete_operation');
		}
	}	
}elseif($_GET['op'] == 'checkseccode'){
	//��֤��
		if(ckseccode(trim($_GET['seccode']))) {
			showmessage('succeed');
		} else {
			showmessage('incorrect_code');
		}
}elseif($_GET['op']=='updatesiteupnum'){
		//���¶���
		if(updatestatistic('site','up',array('updateid'=>$item['siteid'],'feedid'=>$item['siteid'])))
			showmessage($item['up']+1);
		else{
			$errstr=$item['up'];
			include_once template("message");
			return;
		}

}elseif($_GET['op']=='updatesitedownnum'){
		//���¶���
        if(updatestatistic('site','down',array('updateid'=>$item['siteid'],'feedid'=>$item['siteid'])))
			showmessage($item['down']+1);
		else{
			$errstr=$item['down'];
			include_once template("message");
			return;
		}

}elseif($_GET['op']=='updatesiteviewnum'){
		//���¶���
        updatestatistic('site','viewnum',array('updateid'=>$item['siteid'],'feedid'=>$item['siteid']));
		showmessage($item['viewnum']+1);
}elseif($_GET['op']=='reporterr'){
		//�ٱ�����
		  include_once(S_ROOT.'./data/data_linkerrtype.php');
		  if(submitcheck('errsubmit')) {
			//��֤��
			if(checkperm('seccode') && !ckseccode($_POST['seccode'])) {
				showmessage('incorrect_code');
			}
			$ret = linkerr_post($_POST, $item);
			if($ret) {	
				showmessage('do_success',$_SGLOBAL[refer]);
			} else{
				showmessage('that_should_at_least_write_things');
			}
		}
}elseif($_GET['op']=='checkerror'){

		$orderarr=$orderarr." ORDER by main.dateline DESC ";
		$theurl="cp.php?ac=$ac&op=$op";
		//��ҳ��ȡ������
		$page=empty($_GET['page'])?0:intval($_GET['page']);
		$perpage=$_SC['bookmark_show_maxnum'];
		$start=$page?(($page-1)*$perpage):0;
		
		//��ȡ����
		 $count=$_SGLOBAL['db']->result($_SGLOBAL['db']->query("SELECT COUNT(*) FROM ".tname('linkerr')),0);

		//��ȡ����û��ͨ����֤����ǩ�ύ
		$query  = $_SGLOBAL['db']->query("SELECT * FROM ".tname('linkerr')." main left join ".tname("link")." field on main.siteid=field.siteid ".$orderarr." limit ".$start." , ".$_SC['bookmark_show_maxnum']);
		while($value =$_SGLOBAL['db']->fetch_array($query)){
			$value['link_tag'] = implode(' ',empty($value['link_tag'])?array():unserialize($value['link_tag']));
			$err=explode(",",$value['errid']);
			$value['errid']='';
			foreach($err as $key=>$val)
			{
			   $value['errid']=$value['errid'].'   '.$key.':'.$_SGLOBAL['linkerrtype'][$val];
			}
			$value['errid']=$value['errid'].$value['other'];
			$linklist[]=$value;
			$value_err['link_description']=$value['errid'];
			$linklist[]= $value_err;
		}
		//��ҳ
		$link_multi = multi($count, $perpage, $page, $theurl,'bmcontent','bmcontent',1);

		$_TPL['css'] = 'network'; 


}elseif($_GET['op']=='bookmark'){

	if(submitcheck('bookmarksubmit')) {
	}
	$browserid=(empty($_GET['browserid'])||!in_array(intval($_GET['browserid']),$_SGLOBAL['browsertype']))?$_SGLOBAL['browsertype']['ie']:intval($_GET['browserid']);
	//��ȷ��ʾtag
	//$item['link_tag'] = implode(' ',empty($item['link_tag'])?array():unserialize($item['link_tag']));
	//��ȡ���õ�tag
	$shownums=$_SC['favorite_tag_maxnum'];
	$tag_query  = $_SGLOBAL['db']->query("SELECT main.* FROM ".tname('sitetag')." main ORDER BY main.totalnum DESC limit 0,".$shownums);
	while($value =$_SGLOBAL['db']->fetch_array($tag_query))
		$taglist[$value['tagid']]=$value['tagname'];
}else {
	 if(submitcheck('addsubmit')) {
		//��֤��
		if(checkperm('seccode') && !ckseccode($_POST['seccode'])) {
			showmessage('incorrect_code');
		}

	}
	//���ӱ༭
	//�����ϰ��õ�tag�е�,ȥ��
	$_GET['tag']=(empty($_GET['tag']))?'':str_replace(array(','), array(' '), $_GET['tag']);

	//��ȡ���õ�tag
	$shownums=$_SC['favorite_tag_maxnum'];
	$tag_query  = $_SGLOBAL['db']->query("SELECT main.* FROM ".tname('sitetag')." main ORDER BY main.totalnum DESC limit 0,".$shownums);
	while($value =$_SGLOBAL['db']->fetch_array($tag_query))
		$taglist[$value['tagid']]=$value['tagname'];
	/*
	//��ȡclass����
	$class_query  = $_SGLOBAL['db']->query("SELECT main.* FROM ".tname('linkclass')." main WHERE main.parentid=0");
	while($value =$_SGLOBAL['db']->fetch_array($class_query))
	{
		//��ȡ����Ŀ¼
		$classnd_query  = $_SGLOBAL['db']->query("SELECT main.* FROM ".tname('linkclass')." main WHERE main.parentid=".$value['groupid']);
		while($classnd_value =$_SGLOBAL['db']->fetch_array($classnd_query))
		{
			//��ȡ����Ŀ¼
			$classrd_query  = $_SGLOBAL['db']->query("SELECT main.* FROM ".tname('linkclass')." main WHERE main.parentid=".$classnd_value['groupid']);
			while($classrd_value =$_SGLOBAL['db']->fetch_array($classrd_query))
			{
				$classnd_value['grandson'][]= $classrd_value;
			}
			$value['son'][]=$classnd_value;
		}
		$classlist[]=$value;
	}
	*/
}

include_once template("cp_site");

?>