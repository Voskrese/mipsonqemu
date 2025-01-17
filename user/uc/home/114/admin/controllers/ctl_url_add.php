<?php
/**
 * 网站收录
 *
 * @since 2009-7-13
 * @copyright http://www.114la.com
 */
!defined('PATH_ADMIN') && exit('Forbidden');

class ctl_url_add
{
    /**
     * 列表
     *
     * @return void
     */
    public static function index()
    {
        try
        {
            app_tpl::assign( 'npa', array('管理首页', '查看收录申请') );
            $type = (isset($_GET['type'])) ? (int)$_GET['type'] : -1;

            $start = (empty($_GET['start'])) ? 0 : (int)$_GET['start'];
            $result = mod_url_add::get_list($type, $start, PAGE_ROWS);

            if (!empty($result))
            {
                app_tpl::assign('page_url', "?c=url_add");
                app_tpl::assign('pages', mod_pager::get_page_number_list($result['total'], $start, PAGE_ROWS));

                app_tpl::assign('list', $result['data']);
                app_tpl::assign('referer', $_SERVER['REQUEST_URI']);
            }
        }
        catch (Exception $e)
        {

        }
        app_tpl::display('url_add_list.tpl');
    }


	/**
     * 显示单条记录
     *
     * @return void
     */
    public static function show()
    {
        try
        {
            app_tpl::assign( 'npa', array('管理首页', '收录管理') );
            $id = (empty($_GET['id']))  ? 0 : (int)$_GET['id'];
            if ($id < 1)
            {
                throw new Exception('操作失败', 10);
            }

            $result = mod_url_add::get_one($id);
            if (empty($result))
            {
                throw new Exception('操作失败', 10);
            }

            app_tpl::assign('data', $result);
            app_tpl::assign('back', (empty($_SERVER['HTTP_REFERER'])) ? '?c=add_url' : $_SERVER['HTTP_REFERER']);
            unset($result);

            app_tpl::assign( 'class_list', mod_class::get_subclass_list(0));
        }
        catch (Exception $e)
        {
            mod_login::message($e->getMessage(), '?a=add_url');
        }
        app_tpl::display('url_add_show.tpl');
    }


    /**
	 * 没有经过审核
	 *
	 * @return void
     */
    public static function delete()
    {
        try
        {
            if (!empty($_POST['delete']))
            {
                $condition = '';
                foreach ($_POST['delete'] as $key => $val)
                {
                    $condition .= (empty($condition)) ? $key : ", {$key}";
                }
                if (!empty($condition))
                {
                    app_db::delete('ylmf_urladd', "id IN ({$condition})");
                    mod_login::message('删除成功', (empty($_POST['referer'])) ? '?c=url_add' : $_POST['referer']);
                }
            }
            else
            {
                mod_login::message('请选择需要删除的行', (empty($_POST['referer'])) ? '?c=url_add' : $_POST['referer']);
            }
        }
        catch (Exception $e)
        {

        }
    }


    /**
     * 通过审核
     *
     * @return void
     */
    public static function pass()
    {
        try
        {
            $data = array();

            $id = (empty($_POST['id'])) ? '' : $_POST['id'];
            $pass = (empty($_POST['pass'])) ? '' : $_POST['pass'];
            $referer = (empty($_POST['referer'])) ? '?c=url_add' : $_POST['referer'];

            if ($pass == 'yes')
            {
                $site_name = (empty($_POST['site_name'])) ? '' : htmlspecialchars($_POST['site_name'], ENT_QUOTES);
                if (empty($site_name))
                {
                    throw new Exception('请输入站点名称', 10);
                }
                $data['name'] = $site_name;

                $site_url = (empty($_POST['site_url'])) ? '' : $_POST['site_url'];
                if (empty($site_url) || !preg_match('#^http[s]?://#', $site_url))
                {
                    throw new Exception('网站地址不能为空或请以http://开头', 10);
                }
                $data['url'] = $site_url;

                $color = (empty($_POST['color'])) ? '' : trim($_POST['color']);
                if (!empty($color) && !preg_match("/^#?([a-f]|[0-9]){3}(([a-f]|[0-9]){3})?$/i", $color))
                {
                    throw new Exception('颜色代码不正确，（正确方式：#FF0000）', 10);
                }
                $data['namecolor'] = $color;
				$start_time = (empty($_POST['start_time'])) ? 0 : strtotime($_POST['start_time']);
                $data['starttime'] = $start_time;

                $end_time = (empty($_POST['end_time'])) ? 0 : strtotime($_POST['end_time']);
                if ($end_time < $start_time)
                {
                    throw new Exception('结束时间不能早于开始时间', 10);
                }
                $data['endtime'] = $end_time;
				$remark = (empty($_POST['remark'])) ? '' : trim($_POST['remark']);
				$data['remark'] = $remark;

                $class_id = (empty($_POST['classid'])) ? 0 : (
                            (false === strpos($_POST['classid'], ',')) ? $_POST['classid'] :
                                                                          substr($_POST['classid'], strrpos($_POST['classid'], ',') + 1)
                            );
                $cache_main_class = mod_class::get_class_list();
                if (!array_key_exists($class_id, $cache_main_class))
                {
                    throw new Exception('请选择站点分类', 10);
                }
                if ($cache_main_class[$class_id]['parentid'] == 0 || $cache_main_class[$cache_main_class[$class_id]['parentid']]['parentid'] == 0)
                {
                    throw new Exception('分类请选择子分类', 10);
                }
                if (preg_match("#^http[s]?://#", $cache_main_class[$class_id]['path']))
                {
                    throw new Exception('父分类是外部链接,无法添加!', 10);
        	    }
                $data['class'] = $class_id;

                $recommend = (empty($_POST['recommend'])) ? 0 : 1;
                $data['good'] = $recommend;

                $order = (empty($_POST['displayorder'])) ? 100 : $_POST['displayorder'];
                $data['displayorder'] = $order;

          //      $data['adduser'] = addslashes(mod_login::get_username());

                $shenhe = (empty($_POST['shenhe'])) ? '' : htmlspecialchars($_POST['shenhe'], ENT_QUOTES);

				$site_tag = (empty($_POST['site_tag'])) ? '' : trim($_POST['site_tag']);   

				//处理一些link的东西
				//检查在link中是否存在
				$data['url'] = handleUrlString($data['url']);
				$data['hashurl']=qhash($data['url']);
				$data['md5url']=md5($data['url']);

				$sql = "SELECT * FROM ylmf_link WHERE hashurl='".$data['hashurl']."' and md5url='".$data['md5url']."' and url='".$data['url']."'";
				$query = app_db::query($sql);
				$linkid=0;
				$rt = app_db::fetch_one();				
				$linkid = $rt['linkid'] ;					
				
	
				global $_SC;

				$data['initaward'] = empty($linkid)?$_SC['link_award_initial_value']:$rt['initaward'];			
				$data['storenum'] = empty($linkid)?0:$rt['storenum'];
				$data['viewnum']= empty($linkid)?0:$rt['viewnum'];
				$data['up']= empty($linkid)?0:$rt['up'];
				$data['down']= empty($linkid)?0:$rt['down'];
				$data['tmppic']= empty($linkid)?'':$rt['tmppic'];
				$data['pic']= empty($linkid)?'':$rt['pic'];
				$data['picflag']= empty($linkid)?0:$rt['picflag'];

				if(empty($linkid))
					$data=setlinkimagepath($data);
				$data['award']=calc_link_award($data['initaward'],$data['storenum'],$data['viewnum'],$data['up'],$data['down']);
                // 新增
                if (app_db::insert('ylmf_site', array_keys($data), array_values($data)))
                {
					$siteid = @mysql_result(app_db::query("SELECT last_insert_id()"), 0);
					
					$tagarr = mod_url_add::batch_tag($siteid,$site_tag);
					$data['tag'] = empty($tagarr)?'':addslashes(serialize($tagarr));
					if (false === app_db::update('ylmf_site', $data, "id = {$siteid}"))
					{
						 throw new Exception('数据库操作失败', 10);
					}
					//ramen  20100925 更新link表中的信息，设置siteid，将tag url descrition等清空
					
					if($linkid)
						self::updatelinkinfo($siteid,$linkid);

					$type = 1;
            	    app_db::update('ylmf_urladd', array('type' => $type, 'shenhe' => $shenhe), "id = {$id}");
                    //生成静态页面开始
                    mod_make_html::make_html_one_catalog($cache_main_class[$class_id]['parentid'],
                                                        $cache_main_class[$cache_main_class[$class_id]['parentid']]['classname']);
                }
                else
                {
                    throw new Exception('数据库操作失败', 10);
                }

                $old = mod_url_add::get_one($id);
                $siteinfo = $old;
                // 发送邮件
        	    if (mod_config::get_one_config('yl_sendemail'))
        	    {
            	    $sysname = mod_config::get_one_config('yl_sysname');
            	    $subject = $sysname . ' 站点收录审核通知邮件';
            		$toemail = $siteinfo['email'];

                    $slclassname = $cache_main_class[$cache_main_class[$class_id]['parentid']]['classname'] .' - ' . $cache_main_class[$class_id]['classname'];
    			    $body = '您在' . $sysname . '提交的站点收录请求被通过,收录到了分类:' . $slclassname . '.(因为CDN缓存关系,可能需要1-3天才能更新)';

        		    mod_mail::send($toemail, '', $subject, $body, 'txt');
        	    }

                mod_login::message('操作成功', $referer);
            }
            elseif ($pass == 'no')
            {
                $shenhe = (empty($_POST['shenhe'])) ? '' : htmlspecialchars($_POST['shenhe'], ENT_QUOTES);
            	$old = mod_url_add::get_one($id);
            	if (!empty($old))
            	{
            	    $siteinfo = $old;
            	    $type = 2;
            	    app_db::update('ylmf_urladd', array('type' => $type, 'shenhe' => $shenhe), "id = {$id}");

            	    // 发送邮件
            	    if (mod_config::get_one_config('yl_sendemail') && $shenhe != '')
            	    {
                	    $sysname = mod_config::get_one_config('yl_sysname');
                	    $subject = $sysname . ' 站点收录审核通知邮件';
                		$toemail = $siteinfo['email'];

                		$body = '您在' . $sysname . '提交的站点收录请求被拒绝,理由如下:' . $shenhe;
            		    mod_mail::send($toemail, '', $subject, $body, 'txt');
            	    }

            	    mod_login::message('操作成功', $referer);
            	}
            	else
            	{
            	    throw new Exception('操作失败', 10);
            	}
            }
        }
        catch (Exception $e)
        {
            app_tpl::assign('error', $e->getMessage());
        }

        !empty($_POST['site_name']) && $_POST['name'] = $_POST['site_name'];
        !empty($_POST['site_url']) && $_POST['siteurl'] = $_POST['site_url'];
        app_tpl::assign('data', $_POST);
        app_tpl::assign( 'class_list', mod_class::get_subclass_list(0));
        !empty($_POST['classid']) && app_tpl::assign('class_id', $_POST['classid']);
        app_tpl::display('url_add_show.tpl');
    }

	public static function updatelinkinfo($siteid,$linkid)
	{
				echo $siteid.'  '.$linkid;
				$result = mod_url_add::get_one_link($linkid);
                if (empty($result)||$siteid<1)
                {
                    throw new Exception('没有找到数据', 10);
                }
				$result['siteid'] = $siteid;
				$result['url'] = '';
				//更新tag信息
				if(!empty($result['link_tag'])){
					$need_delete_tags = unserialize($result['link_tag']);
					//更新计数
					app_db::query("UPDATE ylmf_sitetag SET totalnum=totalnum-1 WHERE tagid IN (".simplode(array_keys($need_delete_tags)).")");
					//清除在sitetagsite中的$linkid
					app_db::query("DELETE FROM ylmf_sitetagsite WHERE linkid=".$linkid);					
				}
				$result['link_tag'] = '';
				$result['link_subject'] = '';
				$result['link_description'] = '';
				$result['up'] = 0;
				$result['down'] = 0;
				$result['storenum'] = 0;
				$result['viewnum'] = 0;
				$result['tmppic'] = '';
				$result['pic'] = '';
				$result['md5url'] = '';
				$result['hashurl'] = 0;
				$result['award'] = 0;
				$result['initaward'] = 0;
				app_db::update('ylmf_link', $result, "linkid = {$linkid}");
	}

}
?>
